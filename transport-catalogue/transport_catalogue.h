#pragma once

#include <algorithm>
#include <deque>
#include <iostream>
#include <optional>
#include <vector>
#include <utility>
#include <unordered_map>
#include <set>
#include "geo.h"

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
        Bus(const std::string& bus_name, const std::vector<std::string_view>& route) : name(bus_name), route(route) {}
        std::string name;
        std::vector<std::string_view> route;        
    };

    class StopHasher {
    public:
        size_t operator()(std::pair<const Stop*, const Stop*> p) const {
            return hasher_(p.first) * 37 + hasher_(p.second);
        }
    private:
        std::hash<const void*> hasher_;
    };

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;

        void AddStop(const std::string& stop_name, const geo::Coordinates& coord);
        void AddStopsDistance(const std::string& stop1, const std::string& stop2, double distance);
        void AddBus(const std::string& bus_name, const std::vector<std::string_view>& route);

        const Bus* FindBusByName(std::string_view bus_name) const;
        const Stop* FindStopByName(std::string_view stop_name) const;

        double GetStopsDistance(std::string_view stop1, std::string_view stop2) const;
        std::optional<std::set<std::string_view>> GetBusesByStop(std::string_view stop_name) const;
        std::optional<RouteInfo> GetRouteInfoByBusName(const std::string& name) const;

    private:
        const Stop* GetStopPtr(const std::string& name, const geo::Coordinates& coord);
        std::string_view GetCopyStopName(std::string_view name);
        const Bus* GetBusPtr(const std::string& name, const std::vector<std::string_view>& route);
        std::string_view GetCopyBusName(std::string_view name);

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, const Stop*> stop_data_;
        std::unordered_map<std::string_view, const Bus*> bus_data_;
        mutable std::unordered_map<std::string_view, std::set<std::string_view>> stop_buses_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopHasher> stops_distance_;
    };

}

