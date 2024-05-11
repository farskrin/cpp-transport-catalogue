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
#include "domain.h"

namespace model {

    struct TimeAndSpanCount {
        TimeAndSpanCount(std::string_view from, std::string_view to, double time, int span_count)
            : from(from), to(to), time(time), span_count(span_count) {
        }
        std::string_view from;
        std::string_view to;
        double time = 0.;
        int span_count = 0;
    };    

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;

        void AddStop(const std::string& stop_name, const geo::Coordinates& coord);
        void SetStopsDistance(const std::string& from, const std::string& to, double distance);
        void AddBus(const std::string& bus_name, const std::vector<std::string>& route,
            const std::vector<std::string>& end_points, bool is_roundtrip, size_t end_point_idx);

        const Bus* FindBusByName(std::string_view bus_name) const;
        const Stop* FindStopByName(std::string_view stop_name) const;

        double GetStopsDistance(std::string_view from, std::string_view to) const;
        std::optional<std::set<std::string_view>> GetBusesByStop(std::string_view stop_name) const;
        std::optional<RouteInfo> GetRouteInfoByBusName(const std::string& name) const;
        const std::unordered_map<std::string_view, const Bus*>& GetBusData() const;
        //const std::unordered_map<std::string_view, std::set<std::string_view>>& GetStopPerBuses() const;
        std::set<std::string_view> GetSortedStopsInRoutes() const;
        
        //transport_router
        const std::deque<Bus>& GetBuses() const;
        std::set<std::string_view> GetSortedStopsInTask() const;
        std::vector<TimeAndSpanCount> GetRouteTimeAndSpan(std::string_view bus_name, double bus_velocity) const;
    private:
        const Stop* GetStopPtr(const std::string& name, const geo::Coordinates& coord);
        std::string_view GetCopyStopName(std::string_view name);
        const Bus* GetBusPtr(const std::string& name, const std::vector<std::string_view>& route,
            const std::vector<std::string_view>& end_points, bool is_roundtrip, size_t end_point_idx);
        std::string_view GetCopyBusName(std::string_view name);

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, const Stop*> stop_data_;
        std::unordered_map<std::string_view, const Bus*> bus_data_;
        mutable std::unordered_map<std::string_view, std::set<std::string_view>> stop_buses_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopHasher> stops_distance_;
        std::set<std::string_view> stops_in_routes_;
        std::set<std::string_view> stops_in_task_;
    };

}

