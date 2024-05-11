#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace model {

    struct Stop {
        Stop(const std::string& stop_name, const geo::Coordinates& coord) : name(stop_name), coord(coord) {}
        std::string name;
        geo::Coordinates coord;
    };

    struct RouteInfo {
        std::string route_name;
        size_t stop_count = 0;
        size_t unique_stop_count = 0;
        double length = 0.0;
        double curvature = 0.0;
    };

    struct Bus {
        Bus(const std::string& bus_name, const std::vector<std::string_view>& route, 
            const std::vector<std::string_view>& end_points, bool is_roundtrip, size_t end_point_idx)
            : name(bus_name), route(route), end_points(end_points), is_roundtrip(is_roundtrip), end_point_idx(end_point_idx) {
        }
        std::string name;
        std::vector<std::string_view> route;
        //--конечные точки маршрута, одна для кольцевого и две для некольцевого
        std::vector<std::string_view> end_points;
        bool is_roundtrip;
        size_t end_point_idx = 0;
    };

    class StopHasher {
    public:
        size_t operator()(std::pair<const Stop*, const Stop*> p) const {
            return hasher_(p.first) * 37 + hasher_(p.second);
        }
    private:
        std::hash<const void*> hasher_;
    };

}   //model
