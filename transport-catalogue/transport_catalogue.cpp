#include "transport_catalogue.h"

namespace model {

    void TransportCatalogue::AddStop(const std::string& stop_name, const geo::Coordinates& coord) {
        auto stop_ptr = GetStopPtr(stop_name, coord);
        stop_data_[stop_ptr->name] = stop_ptr;
        stops_in_task_.insert(stop_ptr->name);
    }

    void TransportCatalogue::SetStopsDistance(const std::string& from, const std::string& to, double distance) {    
        auto pair_stops = std::make_pair(FindStopByName(from), FindStopByName(to));
        stops_distance_[pair_stops] = distance;
    }

    void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<std::string>& route,
        const std::vector<std::string>& end_points, bool is_roundtrip, size_t end_point_idx) {

        std::vector<std::string_view> copy_route(route.size());
        std::transform(route.begin(), route.end(), copy_route.begin(), [&](std::string_view s) {
            return GetCopyStopName(s);
            });
        std::vector<std::string_view> copy_end_points(end_points.size());
        std::transform(end_points.begin(), end_points.end(), copy_end_points.begin(), [&](std::string_view s) {
            return GetCopyStopName(s);
            });
        auto bus_ptr = GetBusPtr(bus_name, copy_route, copy_end_points, is_roundtrip, end_point_idx);
        bus_data_[bus_ptr->name] = bus_ptr;
        //--
        for (std::string_view stop_name : copy_route) {
            stop_buses_[stop_name].insert(GetCopyBusName(bus_name));
            stops_in_routes_.insert(stop_name);
        }
    }

    const Bus* TransportCatalogue::FindBusByName(std::string_view bus_name) const {
        if (bus_data_.count(bus_name)) {
            return bus_data_.at(bus_name);
        }
        return nullptr;
    }

    const Stop* TransportCatalogue::FindStopByName(std::string_view stop_name) const {
        if (stop_data_.count(stop_name)) {
            return stop_data_.at(stop_name);
        }
        return nullptr;
    }

    double TransportCatalogue::GetStopsDistance(std::string_view from, std::string_view to) const
    {
        auto pair_stops_from = std::make_pair(FindStopByName(from), FindStopByName(to));
        auto pair_stops_to = std::make_pair(FindStopByName(to), FindStopByName(from));
        if (stops_distance_.count(pair_stops_from)) {
            return stops_distance_.at(pair_stops_from);
        }
        if (stops_distance_.count(pair_stops_to)) {
            return stops_distance_.at(pair_stops_to);
        }
        else
        {
            throw std::logic_error("GetStopsDistance: stops pair not contain in index");
        }
    }

    std::optional<std::set<std::string_view>> TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
        if (stop_data_.size() != stop_buses_.size()) {
            for (const auto& [name, data] : stop_data_) {
                stop_buses_[name];
            }
        }
        if (stop_buses_.count(stop_name)) {
            return stop_buses_.at(stop_name);
        }
        return std::nullopt;
    }

    std::optional<RouteInfo> TransportCatalogue::GetRouteInfoByBusName(const std::string& name) const {

        if (bus_data_.count(name)) {
            auto bus = bus_data_.at(name);
            auto route = bus->route;

            double geographic_length = 0.0;
            double road_length = 0.0;
            size_t route_size = route.size();
            for (size_t i = 0; i + 1 < route_size; i++) {
                geo::Coordinates from = FindStopByName(route[i])->coord;
                geo::Coordinates to = FindStopByName(route[i + 1])->coord;
                double geographic_dist = geo::ComputeDistance(from, to);
                geographic_length += geographic_dist;
                double road_dist = GetStopsDistance(route[i], route[i + 1]);
                road_length += road_dist;
            }

            std::set<std::string_view> unique_stop(route.begin(), route.end());
            RouteInfo info;
            info.route_name = bus->name;
            info.stop_count = route_size;
            info.unique_stop_count = unique_stop.size();
            info.length = road_length;
            info.curvature = road_length / geographic_length;
            return info;
        }

        return std::nullopt;
    }

    const std::deque<Bus>& TransportCatalogue::GetBuses() const {
        return buses_;
    }

    const std::unordered_map<std::string_view, const Bus*>& TransportCatalogue::GetBusData() const {
        return bus_data_;
    }

    std::set<std::string_view> TransportCatalogue::GetSortedStopsInRoutes() const {
        return stops_in_routes_;
    }

    std::set<std::string_view> TransportCatalogue::GetSortedStopsInTask() const {    
        return stops_in_task_;
    }

    std::vector<TimeAndSpanCount> TransportCatalogue::GetRouteTimeAndSpan(std::string_view bus_name, double bus_velocity) const {
        std::vector<TimeAndSpanCount> dist_time_span;

        auto bus = bus_data_.at(bus_name);
        auto route = bus->route;
        std::vector<std::string_view> end_points = bus->end_points;        
        size_t route_size = bus->is_roundtrip ? route.size() : bus->end_point_idx + 1;

        for (size_t i = 0; i + 1 < route_size; i++) {       //from
		double road_time = 0.;
		int span_count = 0;
        double road_time_back = 0.;
            for (size_t j = i + 1; j < route_size; j++) {   //to                
                
                road_time += GetStopsDistance(route[j - 1], route[j]) / bus_velocity;
                span_count++;               
				dist_time_span.emplace_back(route[i], route[j], road_time, span_count);

                if (!bus->is_roundtrip) {
                    road_time_back += GetStopsDistance(route[j], route[j - 1]) / bus_velocity;
                    dist_time_span.emplace_back(route[j], route[i], road_time_back, span_count);
                }
            }
        }

        return dist_time_span;
    }

    /*const std::unordered_map<std::string_view, std::set<std::string_view>>& TransportCatalogue::GetStopPerBuses() const {
        return stop_buses_;
    }*/

    const Stop* TransportCatalogue::GetStopPtr(const std::string& name, const geo::Coordinates& coord)
    {
        if (const auto it = stop_data_.find(name); it != stop_data_.end()) {
            return it->second;
        }
        return &stops_.emplace_back(name, coord);
    }

    std::string_view TransportCatalogue::GetCopyStopName(std::string_view name) {
        if (const auto it = stop_data_.find(name); it != stop_data_.end()) {
            return it->first;
        }
        throw std::logic_error("GetCopyStopName: stop_data not contain stop name");
    }

    const Bus* TransportCatalogue::GetBusPtr(const std::string& name, const std::vector<std::string_view>& route,
        const std::vector<std::string_view>& end_points, bool is_roundtrip, size_t end_point_idx)
    {
        if (const auto it = bus_data_.find(name); it != bus_data_.end()) {
            return it->second;
        }
        return &buses_.emplace_back(name, route, end_points, is_roundtrip, end_point_idx);
    }

    std::string_view TransportCatalogue::GetCopyBusName(std::string_view name) {
        if (const auto it = bus_data_.find(name); it != bus_data_.end()) {
            return it->first;
        }
        throw std::logic_error("GetCopyBusName: bus_data not contain bus name");
    }
}