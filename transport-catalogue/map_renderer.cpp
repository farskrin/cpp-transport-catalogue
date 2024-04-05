#include "map_renderer.h"

namespace renderer {

	bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    void MapRenderer::GenerateRoutesLine(svg::Document& doc, const std::set<std::string>& bus_names, const std::vector<svg::Color>& color_palette) const {
        size_t color_idx = 0;
        size_t color_cnt = color_palette.size();
        for (const auto& bus_name : bus_names) {
            svg::Polyline polyline;
            auto bus = db_.FindBusByName(bus_name);
            if (bus->route.empty()) continue;

            for (const auto& stop_name : bus->route) {
                auto stop = db_.FindStopByName(stop_name);
                polyline.AddPoint(proj_(stop->coord));
            }
            //--
            polyline.SetFillColor("none")
                .SetStrokeColor(color_palette.at(color_idx % color_cnt))
                .SetStrokeWidth(settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            doc.Add(std::move(polyline));
            color_idx++;
        }
        
    }

    void MapRenderer::GenerateRoutesLabel(svg::Document& doc, const std::set<std::string>& bus_names, const std::vector<svg::Color>& color_palette) const
    {
        //--
        size_t color_idx = 0;
        size_t color_cnt = color_palette.size();
        for (const auto& bus_name : bus_names) {
            auto bus = db_.FindBusByName(bus_name);
            if (bus->route.empty()) continue;
            //--
            for (const auto& stop_name : bus->end_points) {
                svg::Text text_front, text_back;
                auto stop = db_.FindStopByName(stop_name);
                text_front.SetFillColor(color_palette.at(color_idx % color_cnt))
                    .SetPosition(proj_(stop->coord))
                    .SetOffset({ settings_.bus_label_offset.first, settings_.bus_label_offset.second })
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(bus->name);

                text_back.SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetPosition(proj_(stop->coord))
                    .SetOffset({ settings_.bus_label_offset.first, settings_.bus_label_offset.second })
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(bus->name);

                doc.Add(std::move(text_back));
                doc.Add(std::move(text_front));

                if (bus->end_points.size() == 2 && bus->end_points[0] == bus->end_points[1]) break;
            }
            color_idx++;
        }
    }

    void MapRenderer::GenerateStopsCircle(svg::Document& doc, const std::set<std::string_view>& sorted_stops) const
    {        
        for (const auto& stop_name : sorted_stops) {
            auto stop = db_.FindStopByName(stop_name);
            svg::Circle circle;
            circle.SetCenter(proj_(stop->coord))
                .SetRadius(settings_.stop_radius)
                .SetFillColor("white");
            doc.Add(std::move(circle));
        }
    }

    void MapRenderer::GenerateStopsLabel(svg::Document& doc, const std::set<std::string_view>& sorted_stops) const
    {
        for (const auto& stop_name : sorted_stops) {
            svg::Text text_front, text_back;
            auto stop = db_.FindStopByName(stop_name);
            text_front.SetFillColor("black")
                .SetPosition(proj_(stop->coord))
                .SetOffset({ settings_.stop_label_offset.first, settings_.stop_label_offset.second })
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(stop->name);

            text_back.SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetPosition(proj_(stop->coord))
                .SetOffset({ settings_.stop_label_offset.first, settings_.stop_label_offset.second })
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(stop->name);

            doc.Add(std::move(text_back));
            doc.Add(std::move(text_front));
        }
    }    

    svg::Document MapRenderer::RenderMap() const {

        auto bus_data = db_.GetBusData();
        std::set<std::string> bus_names;
        std::transform(bus_data.begin(), bus_data.end(), std::inserter(bus_names, bus_names.begin()),
            [](const std::pair<std::string_view, const model::Bus*>& data) {
                return std::string(data.first);
            });

        std::vector<svg::Color> color_palette = settings_.color_palette;

        svg::Document doc;
        //--
        GenerateRoutesLine(doc, bus_names, color_palette);
        GenerateRoutesLabel(doc, bus_names, color_palette);
        //--
        std::set<std::string_view> sorted_stops = GetSortedStops();
        GenerateStopsCircle(doc, sorted_stops);
        GenerateStopsLabel(doc, sorted_stops);

        return doc;
    }

    std::set<std::string_view> MapRenderer::GetSortedStops() const {
        using namespace model;
        return db_.GetSortedStopsInRoutes();
    }

    std::vector<geo::Coordinates> MapRenderer::GetGeoCoords() const {
        using namespace model;
        std::vector<geo::Coordinates> geo_coords;
        const auto& stops_in_buses = db_.GetSortedStopsInRoutes();
        for (const auto& stop_name : stops_in_buses) {
            auto stop = db_.FindStopByName(stop_name);
            geo_coords.push_back(stop->coord);
        }
        return geo_coords;
    }

}