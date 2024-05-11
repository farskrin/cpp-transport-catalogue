#include "json_reader.h"
#include <sstream>
#include <variant>
#include <memory>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
using namespace std::literals;

namespace io {
    std::tuple<std::vector<std::string>, std::vector<std::string>, size_t> ParseRoute(const json::Node stops_node, bool is_roundtrip) {
        std::vector<std::string> route;
        std::vector<std::string> end_points;
        size_t end_point_idx = 0;
        const auto& stops_list = stops_node.AsArray();
        route.reserve(stops_list.size());
        for (const auto& stop : stops_list) {
            route.push_back(stop.AsString());
        }
        if (!route.empty()) {
            end_points = is_roundtrip ? std::vector<std::string>{route.front()}
            : std::vector<std::string>{ route.front(), route.back() };
            end_point_idx = route.size() - 1;
        }
        if (!is_roundtrip) {
            std::vector<std::string> results(route.begin(), route.end());
            results.insert(results.end(), std::next(route.rbegin()), route.rend());
            return { results, end_points, end_point_idx };
        }
        return { route, end_points, end_point_idx };
    }

    void JsonReader::ApplyBaseRequests(model::TransportCatalogue& catalogue) const {
        using namespace json;
        std::vector<size_t> commands_stop;
        std::vector<size_t> commands_bus;
        //--
        auto root = doc_.GetRoot().AsDict();
        auto base_requests = root.at("base_requests");
        size_t command_idx = 0;
        const auto& base_list = base_requests.AsArray();
        for (const auto& base : base_list) {
            const auto& base_obj = base.AsDict();
            std::string type = base_obj.at("type").AsString();
            if (type == "Stop") {
                geo::Coordinates coord = { base_obj.at("latitude").AsDouble(), base_obj.at("longitude").AsDouble() };
                catalogue.AddStop(base_obj.at("name").AsString(), coord);
                commands_stop.push_back(command_idx++);
            }
            else
            {
                commands_bus.push_back(command_idx++);
            }
        }

        for (size_t idx : commands_stop) {
            const auto& base = base_list[idx];
            const auto& base_obj = base.AsDict();
            const auto& road_distances = base_obj.at("road_distances").AsDict();
            for (const auto& [to_stop, dist] : road_distances) {
                catalogue.SetStopsDistance(base_obj.at("name").AsString(), to_stop, dist.AsDouble());
            }
        }

        for (size_t idx : commands_bus) {
            const auto& base = base_list[idx];
            const auto& base_obj = base.AsDict();
            std::string type = base_obj.at("type").AsString();
            if (type == "Bus") {
                json::Node stops_node = base_obj.at("stops");
                bool is_roundtrip = base_obj.at("is_roundtrip").AsBool();
                auto [route, end_points, end_point_idx] = ParseRoute(stops_node, is_roundtrip);

                catalogue.AddBus(base_obj.at("name").AsString(), route, end_points, is_roundtrip, end_point_idx);
            }
        }
    }

    svg::Color ParseColor(json::Node color_node) {
        if (color_node.IsString()) {
            return color_node.AsString();
        }
        if (color_node.IsArray()) {
            auto color_list = color_node.AsArray();
            if (color_list.size() == 3) {
                return svg::Rgb{
                    static_cast<uint8_t>(color_list.at(0).AsInt()),
                    static_cast<uint8_t>(color_list.at(1).AsInt()),
                    static_cast<uint8_t>(color_list.at(2).AsInt()) 
                };
            }
            if (color_list.size() == 4) {
                return svg::Rgba{
                    static_cast<uint8_t>(color_list.at(0).AsInt()),
                    static_cast<uint8_t>(color_list.at(1).AsInt()),
                    static_cast<uint8_t>(color_list.at(2).AsInt()),
                    color_list.at(3).AsDouble()
                };
            }
        }
        return svg::Color();
    }

    renderer::RenderSettings JsonReader::ParseRenderSettings() const {
        using namespace json;
        renderer::RenderSettings render_settings;
        auto root = doc_.GetRoot().AsDict();
        auto settings_obj = root.at("render_settings").AsDict();
        render_settings.width = settings_obj.at("width").AsDouble();
        render_settings.height = settings_obj.at("height").AsDouble();
        render_settings.padding = settings_obj.at("padding").AsDouble();
        render_settings.line_width = settings_obj.at("line_width").AsDouble();
        render_settings.stop_radius = settings_obj.at("stop_radius").AsDouble();
        render_settings.bus_label_font_size = settings_obj.at("bus_label_font_size").AsInt();
        render_settings.bus_label_offset = std::make_pair(settings_obj.at("bus_label_offset").AsArray().at(0).AsDouble(),
            settings_obj.at("bus_label_offset").AsArray().at(1).AsDouble());
        render_settings.stop_label_font_size = settings_obj.at("stop_label_font_size").AsInt();
        render_settings.stop_label_offset = std::make_pair(settings_obj.at("stop_label_offset").AsArray().at(0).AsDouble(),
            settings_obj.at("stop_label_offset").AsArray().at(1).AsDouble());
        render_settings.underlayer_color = ParseColor(settings_obj.at("underlayer_color"));
        render_settings.underlayer_width = settings_obj.at("underlayer_width").AsDouble();

        auto color_palette_list = settings_obj.at("color_palette").AsArray();
        for (const auto& color : color_palette_list) {
            render_settings.color_palette.push_back(ParseColor(color));
        }

        return render_settings;
    }

    routing::RoutingSettings JsonReader::ParseRoutingSettings() const
    {
        using namespace json;
        auto meter_per_min = [](double km_per_hour) { return 1'000. * km_per_hour / 60.; };
        routing::RoutingSettings routing_settings;
        auto root = doc_.GetRoot().AsDict();
        auto settings_obj = root.at("routing_settings").AsDict();
        routing_settings.bus_wait_time = settings_obj.at("bus_wait_time").AsInt();
        routing_settings.bus_velocity = meter_per_min(settings_obj.at("bus_velocity").AsDouble());
        return routing_settings;
    }

    //-----------------------
    std::string Print(const json::Node& node) {
        std::ostringstream out;
        Print(json::Document{ node }, out);
        return out.str();
    }

    void PrintBusStat(const model::TransportCatalogue& transport_catalogue, int id,
        std::string_view name, json::Builder& builder) {
        builder.StartDict();
        builder.Key("request_id").Value(id);
        if (auto info = transport_catalogue.GetRouteInfoByBusName(std::string(name)); info.has_value()) {
            builder.Key("curvature").Value(info->curvature);
            builder.Key("route_length").Value(info->length);
            builder.Key("stop_count").Value(static_cast<int>(info->stop_count));
            builder.Key("unique_stop_count").Value(static_cast<int>(info->unique_stop_count));
        }
        else
        {
            builder.Key("error_message"s).Value("not found"s);
        }
        builder.EndDict();
    }

    void PrintStopStat(const model::TransportCatalogue& transport_catalogue, int id,
        std::string_view name, json::Builder& builder) {
        builder.StartDict();
        builder.Key("request_id"s).Value(id);
        if (auto buses = transport_catalogue.GetBusesByStop(name); buses.has_value()) {            
            builder.Key("buses"s).StartArray();           
            for (const auto& bus_name : buses.value()) {
                builder.Value(std::string(bus_name));
            }
            builder.EndArray();
        }
        else
        {
            builder.Key("error_message"s).Value("not found"s);
        }
        builder.EndDict();
    }

    void PrintMapStat(const renderer::MapRenderer& map_renderer, int id, json::Builder& builder) {
        builder.StartDict();
        builder.Key("request_id"s).Value(id);
        std::ostringstream oss;
        map_renderer.RenderMap().Render(oss);
        std::string map_str = oss.str();
        builder.Key("map"s).Value(map_str);
        builder.EndDict();
    }

    struct RouteItemVisitor {
        json::Builder& json;

        void operator()(const routing::WaitItem& response) const {
            json.Key("type"s).Value(response.type);
            json.Key("stop_name"s).Value(response.stop_name);
            json.Key("time"s).Value(response.time);
        }
        void operator()(const routing::BusItem& response) const {
            json.Key("type"s).Value(response.type);
            json.Key("bus").Value(response.bus_name);
            json.Key("span_count"s).Value(response.span_count);
            json.Key("time"s).Value(response.time);
        }
    };

    void PrintRouteStat(const routing::ResponseData& router_data, int id, json::Builder& builder) {
        builder.StartDict();
        builder.Key("request_id"s).Value(id);
        builder.Key("total_time"s).Value(router_data.total_time);
        builder.Key("items"s).StartArray();
        for (const auto& item : router_data.items) {
            builder.StartDict();
            std::visit(RouteItemVisitor{ builder }, item);
            builder.EndDict();
        }
        builder.EndArray();
        builder.EndDict();
    }

    void PrintErrorMessage(int request_id, json::Builder& builder) {
        builder.StartDict();
        builder.Key("request_id"s).Value(request_id);
        builder.Key("error_message"s).Value("not found"s);
        builder.EndDict();
    }

    void JsonReader::ApplyStatRequests(const model::TransportCatalogue& catalogue) const {
        using namespace json;
        //----
        std::optional<renderer::RenderSettings> render_settings;
        std::optional <routing::RoutingSettings> routing_settings;
        std::unique_ptr<renderer::MapRenderer> map_renderer = nullptr;
        std::unique_ptr<routing::TransportRouter> router = nullptr;
        //----
        auto root = doc_.GetRoot().AsDict();
        auto stat_requests = root.at("stat_requests");
        auto builder = json::Builder();
        builder.StartArray();
        for (const auto& stat : stat_requests.AsArray()) {
            auto stat_obj = stat.AsDict();
            int id = stat_obj.at("id").AsInt();
            std::string name;
            if (stat_obj.count("name")) {
				name = stat_obj.at("name").AsString();
            }
            std::string type = stat_obj.at("type").AsString();

            if (type == "Bus") {
                PrintBusStat(catalogue, id, name, builder);                
            }
            if (type == "Stop") {
                PrintStopStat(catalogue, id, name, builder);
            }
            if (type == "Map") {
                if (!render_settings.has_value()) {
                    render_settings = ParseRenderSettings();
                    map_renderer = std::make_unique<renderer::MapRenderer>(catalogue, *render_settings);
                }
                PrintMapStat(*map_renderer, id, builder);
            }
            if (type == "Route") {
                const auto& from = stat_obj.at("from").AsString();
                const auto& to = stat_obj.at("to").AsString();
                if (!routing_settings.has_value()) {
                    routing_settings = ParseRoutingSettings();
                    router = std::make_unique<routing::TransportRouter>(catalogue, *routing_settings);
                }                
                if (auto route_data = router->BuildRoute(from, to)) {
                    PrintRouteStat(*route_data, id, builder);
                }
                else
                {
                    PrintErrorMessage(id, builder);
                }
            }
        }
        builder.EndArray();
		Print(json::Document{ builder.Build() }, std::cout);
    }
}
