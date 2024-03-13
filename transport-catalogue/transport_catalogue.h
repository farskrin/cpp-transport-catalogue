#pragma once

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
        std::string name;
        geo::Coordinates coord;        
    };
    
    struct RouteInfo {
        std::string route_name;
        size_t stop_count = 0;
        size_t unique_stop_count = 0;
        double route_length = 0.0;
    };

    struct Bus {
        std::string name;
        std::vector<std::string_view> route;        
    };

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;
        ~TransportCatalogue();

        void AddStop(const std::string& stop_name, const geo::Coordinates& coord);

        void AddBus(const std::string& bus_name, const std::vector<std::string_view>& route);

        const Bus* FindBusByName(std::string_view bus_name) const;

        const Stop* FindStopByName(std::string_view stop_name) const;

        std::optional<std::set<std::string_view>> GetBusesByStop(std::string_view stop_name) const;

        std::optional<RouteInfo> GetRouteInfoByBusName(const std::string& name) const;

    private:
        std::string_view GetCopyStopName(std::string_view name);
        std::string_view GetCopyBusName(std::string_view name);

        std::deque<std::string> stops_;
        std::deque<std::string> buses_;
        std::unordered_map<std::string_view, const Stop*> stop_data_;
        std::unordered_map<std::string_view, const Bus*> bus_data_;
        mutable std::unordered_map<std::string_view, std::set<std::string_view>> stop_buses_;
    };

}

