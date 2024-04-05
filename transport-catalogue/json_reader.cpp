#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
using namespace std::literals;

namespace io {
    std::pair<std::vector<std::string>, std::vector<std::string>> ParseRoute(const json::Node stops_node, bool is_roundtrip) {
        std::vector<std::string> route;
        std::vector<std::string> end_points;
        const auto& stops_list = stops_node.AsArray();
        route.reserve(stops_list.size());
        for (const auto& stop : stops_list) {
            route.push_back(stop.AsString());
        }
        if (!route.empty()) {
            end_points = is_roundtrip ? std::vector<std::string>{route.front()}
            : std::vector<std::string>{ route.front(), route.back() };
        }
        if (!is_roundtrip) {
            std::vector<std::string> results(route.begin(), route.end());
            results.insert(results.end(), std::next(route.rbegin()), route.rend());
            return { results, end_points };
        }
        return { route, end_points };
    }

    void JsonReader::ApplyBaseRequests(model::TransportCatalogue& catalogue) const {
        using namespace json;
        std::vector<size_t> commands_stop;
        std::vector<size_t> commands_bus;
        //--
        auto root = doc_.GetRoot().AsMap();
        auto base_requests = root.at("base_requests");
        size_t command_idx = 0;
        const auto& base_list = base_requests.AsArray();
        for (const auto& base : base_list) {
            const auto& base_obj = base.AsMap();
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
            const auto& base_obj = base.AsMap();
            const auto& road_distances = base_obj.at("road_distances").AsMap();
            for (const auto& [to_stop, dist] : road_distances) {
                catalogue.SetStopsDistance(base_obj.at("name").AsString(), to_stop, dist.AsDouble());
            }
        }

        for (size_t idx : commands_bus) {
            const auto& base = base_list[idx];
            const auto& base_obj = base.AsMap();
            std::string type = base_obj.at("type").AsString();
            if (type == "Bus") {
                json::Node stops_node = base_obj.at("stops");
                bool is_roundtrip = base_obj.at("is_roundtrip").AsBool();
                auto [route, end_points] = ParseRoute(stops_node, is_roundtrip);

                catalogue.AddBus(base_obj.at("name").AsString(), route, end_points, is_roundtrip);
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
        auto root = doc_.GetRoot().AsMap();
        auto settings_obj = root.at("render_settings").AsMap();
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

    //-----------------------
    std::string Print(const json::Node& node) {
        std::ostringstream out;
        Print(json::Document{ node }, out);
        return out.str();
    }

    json::Node PrintBusStat(const model::TransportCatalogue& transport_catalogue, int id, std::string_view name) {
        json::Dict dict;
        dict["request_id"] = id;
        if (auto info = transport_catalogue.GetRouteInfoByBusName(std::string(name)); info.has_value()) {
            dict["curvature"] = json::Node(info->curvature);
            dict["route_length"] = info->length;
            dict["stop_count"] = static_cast<int>(info->stop_count);
            dict["unique_stop_count"] = static_cast<int>(info->unique_stop_count);
        }
        else
        {
            dict["error_message"] = "not found"s;
        }
        return json::Node(dict);
    }

    json::Node PrintStopStat(const model::TransportCatalogue& transport_catalogue, int id, std::string_view name) {
        json::Dict dict;
        dict["request_id"] = id;
        if (auto buses = transport_catalogue.GetBusesByStop(name); buses.has_value()) {            
            json::Array buses_list;
            for (const auto& bus_name : buses.value()) {
                buses_list.emplace_back(std::string(bus_name));
            }
            dict["buses"] = buses_list;
        }
        else
        {
            dict["error_message"] = "not found"s;
        }
        return json::Node(dict);
    }

    json::Node PrintMapStat(const renderer::MapRenderer& map_renderer, int id) {
        json::Dict dict;
        dict["request_id"] = id;
        std::ostringstream oss;
        map_renderer.RenderMap().Render(oss);
        std::string map_str = oss.str();
        dict["map"] = map_str;

        return json::Node(dict);
    }

    void JsonReader::ApplyStatRequests(const model::TransportCatalogue& catalogue) const {
        using namespace json;
        //----
        auto root = doc_.GetRoot().AsMap();
        auto stat_requests = root.at("stat_requests");
        json::Array result;
        for (const auto& stat : stat_requests.AsArray()) {
            auto stat_obj = stat.AsMap();
            int id = stat_obj.at("id").AsInt();
            std::string name;
            if (stat_obj.count("name")) {
				name = stat_obj.at("name").AsString();
            }
            std::string type = stat_obj.at("type").AsString();

            if (type == "Bus") {
                auto node = PrintBusStat(catalogue, id, name);
                result.push_back(std::move(node));
            }
            if (type == "Stop") {
                auto node = PrintStopStat(catalogue, id, name);
                result.push_back(std::move(node));
            }
            if (type == "Map") {
                auto settings = ParseRenderSettings();
                renderer::MapRenderer map_renderer(catalogue, settings);
                auto node = PrintMapStat(map_renderer, id);
                result.push_back(std::move(node));
            }

        }
		Print(json::Document{ Node(std::move(result)) }, std::cout);
    }
}
