#include "transport_catalogue.h"

namespace model {

    TransportCatalogue::~TransportCatalogue() {
        for (auto& stop : stop_data_) {
            delete stop.second;
        }
        for (auto& bus : bus_data_) {
            delete bus.second;
        }
    }

    void TransportCatalogue::AddStop(const std::string& stop_name, const geo::Coordinates& coord) {
        const Stop* stop = new Stop{ stop_name, coord };
        const std::string_view copy_name = GetCopyStopName(stop_name);
        stop_data_[copy_name] = stop;
    }

    void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<std::string_view>& route) {

        std::vector<std::string_view> copy_route;
        copy_route.reserve(route.size());
        for (std::string_view stop_name : route) {
            std::string_view copy_stop_name = GetCopyStopName(stop_name);
            copy_route.push_back(copy_stop_name);
            //--
            stop_buses_[copy_stop_name].insert(GetCopyBusName(bus_name));
        }

        const Bus* bus = new Bus{ bus_name, copy_route };

        const std::string_view copy_name = GetCopyBusName(bus_name);
        bus_data_[copy_name] = bus;
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

    std::optional<std::set<std::string_view>> TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
        if (stop_data_.size() != stop_buses_.size()) {
            for (const auto& [key, value] : stop_data_) {
                stop_buses_[key];
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

            double route_length = 0.0;
            size_t route_size = route.size();
            for (size_t i = 0; i + 1 < route_size; i++) {
                geo::Coordinates from = FindStopByName(route[i])->coord;
                geo::Coordinates to = FindStopByName(route[i + 1])->coord;
                double distance = ComputeDistance(from, to);
                route_length += distance;
            }

            std::set<std::string_view> unique_stop(route.begin(), route.end());
            RouteInfo info;
            info.route_name = bus->name;
            info.stop_count = route_size;
            info.unique_stop_count = unique_stop.size();
            info.route_length = route_length;
            return info;
        }

        return std::nullopt;
    }

    std::string_view TransportCatalogue::GetCopyStopName(std::string_view name) {        
        if (const auto it = stop_data_.find(name); it != stop_data_.end()) {
            return it->first;
        }
        return stops_.emplace_back(name);
    }

    std::string_view TransportCatalogue::GetCopyBusName(std::string_view name) {        
        if (const auto it = bus_data_.find(name); it != bus_data_.end()) {
            return it->first;
        }
        return buses_.emplace_back(name);
    }
}