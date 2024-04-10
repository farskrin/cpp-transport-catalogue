#pragma once

#include "svg.h"
#include "geo.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <set>
#include <iterator>
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 */
namespace renderer {

	struct RenderSettings {
		double width = 0.;
		double height = 0.;

		double padding = 0.;

		double line_width = 0.;
		double stop_radius = 0.;

		int bus_label_font_size = 0;
		std::pair<double, double> bus_label_offset;

		int stop_label_font_size = 0;
        std::pair<double, double> stop_label_offset;

		svg::Color underlayer_color;
		double underlayer_width = 0.;

		std::vector<svg::Color> color_palette;
	};

    //------------
    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        SphereProjector() = default;
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding)
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        SphereProjector(SphereProjector&& rhs) = default;
        SphereProjector& operator=(SphereProjector&& rhs) = default;

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_ = 0.;
        double min_lon_ = 0.;
        double max_lat_ = 0.;
        double zoom_coeff_ = 0.;
    };


    //------------
	class MapRenderer {
    public:
        MapRenderer(const model::TransportCatalogue& db, const RenderSettings& settings)
            : db_(db), settings_(settings), geo_coords_(std::move(GetGeoCoords())),
            proj_(std::move(SphereProjector{ geo_coords_.begin(), geo_coords_.end(), settings_.width, settings_.height, settings_.padding })) {           
        }

        svg::Document RenderMap() const;

    private:
        std::set<std::string_view> GetSortedStops() const;
        std::vector<geo::Coordinates> GetGeoCoords() const;
        void GenerateRoutesLine(svg::Document& doc, const std::set<std::string>& bus_names, const std::vector<svg::Color>& color_palette) const;
        void GenerateRoutesLabel(svg::Document& doc, const std::set<std::string>& bus_names, const std::vector<svg::Color>& color_palette) const;
        void GenerateStopsCircle(svg::Document& doc, const std::set<std::string_view>& sorted_stops) const;
        void GenerateStopsLabel(svg::Document& doc, const std::set<std::string_view>& sorted_stops) const;

        const model::TransportCatalogue& db_;
        const RenderSettings& settings_;
        std::vector<geo::Coordinates> geo_coords_;
        SphereProjector proj_;
	};

}



