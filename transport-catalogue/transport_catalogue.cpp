#include "transport_catalogue.h"

namespace model {

    void TransportCatalogue::AddStop(const std::string& stop_name, const geo::Coordinates& coord) {
        auto stop_ptr = GetStopPtr(stop_name, coord);        
        stop_data_[stop_ptr->name] = stop_ptr;
    }

    void TransportCatalogue::AddStopsDistance(const std::string& stop1, const std::string& stop2, double distance)
    {
        auto pair_stops = std::make_pair(FindStopByName(stop1), FindStopByName(stop2));
        stops_distance_[pair_stops] = distance;
    }

    void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<std::string_view>& route) {

        std::vector<std::string_view> copy_route(route.size());
        std::transform(route.begin(), route.end(), copy_route.begin(), [&](std::string_view s) {
            return GetCopyStopName(s);
            });
        auto bus_ptr = GetBusPtr(bus_name, copy_route);
        bus_data_[bus_ptr->name] = bus_ptr;
        //--
        for (std::string_view stop_name : copy_route) {            
            stop_buses_[stop_name].insert(GetCopyBusName(bus_name));
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

    double TransportCatalogue::GetStopsDistance(std::string_view stop1, std::string_view stop2) const
    {
        auto pair_stops12 = std::make_pair(FindStopByName(stop1), FindStopByName(stop2));
        auto pair_stops21 = std::make_pair(FindStopByName(stop2), FindStopByName(stop1));
        if (stops_distance_.count(pair_stops12)) {
            return stops_distance_.at(pair_stops12);
        }
        if (stops_distance_.count(pair_stops21)) {
            return stops_distance_.at(pair_stops21);
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
                double geographic_dist = ComputeDistance(from, to);
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

    const Bus* TransportCatalogue::GetBusPtr(const std::string& name, const std::vector<std::string_view>& route)
    {
        if (const auto it = bus_data_.find(name); it != bus_data_.end()) {
            return it->second;
        }
        return &buses_.emplace_back(name, route);
    }

    std::string_view TransportCatalogue::GetCopyBusName(std::string_view name) {        
        if (const auto it = bus_data_.find(name); it != bus_data_.end()) {
            return it->first;
        }
        throw std::logic_error("GetCopyBusName: bus_data not contain bus name");
    }
}