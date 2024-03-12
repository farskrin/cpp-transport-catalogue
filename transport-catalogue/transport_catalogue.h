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
        //std::string stop_name;
        geo::Coordinates coord;    
        std::set<std::string_view> stop_buses;
    };
    
    struct RouteInfo {
        std::string route_name;
        size_t stop_count = 0;
        size_t unique_stop_count = 0;
        double route_length = 0.0;
    };

    struct Bus {
        //std::string bus_name;
        std::vector<std::string_view> stops;
        RouteInfo route_info;
    };

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;
        ~TransportCatalogue();

        void AddStop(const std::string& stop_name, Stop* stop);

        void AddBus(const std::string& bus_name, const Bus* bus);

        const Bus* FindBusByName(std::string_view bus_name) const;

        Stop* FindStopByName(std::string_view stop_name) const;

        std::optional<std::set<std::string_view>> FindStopBusesByName(std::string_view stop_name) const;

        std::string_view GetClonedStop(std::string_view s);

        std::string_view GetClonedBus(std::string_view s);


    private:
        std::deque<std::string> stops_;
        std::deque<std::string> buses_;
        std::unordered_map<std::string_view, Stop*> stop_data_;
        std::unordered_map<std::string_view, const Bus*> bus_data_;        
    };

}

