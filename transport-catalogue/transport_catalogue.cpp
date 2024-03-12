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

    void TransportCatalogue::AddStop(const std::string& stop_name, Stop* stop) {
        const std::string_view copy_name = GetClonedStop(stop_name);
        stop_data_[copy_name] = stop;
    }

    void TransportCatalogue::AddBus(const std::string& bus_name, const Bus* bus) {
        const std::string_view copy_name = GetClonedStop(bus_name);
        bus_data_[copy_name] = bus;
    }

    const Bus* TransportCatalogue::FindBusByName(std::string_view bus_name) const {
        if (bus_data_.count(bus_name)) {
            return bus_data_.at(bus_name);
        }
        return nullptr;
    }

    Stop* TransportCatalogue::FindStopByName(std::string_view stop_name) const {
        if (stop_data_.count(stop_name)) {
            return stop_data_.at(stop_name);
        }
        return nullptr;
    }

    std::optional<std::set<std::string_view>> TransportCatalogue::FindStopBusesByName(std::string_view stop_name) const {
        if (stop_data_.count(stop_name)) {
            return stop_data_.at(stop_name)->stop_buses;
        }
        return std::nullopt;
    }

    std::string_view TransportCatalogue::GetClonedStop(std::string_view s) {
        const auto it = stop_data_.find(s);
        if (it != stop_data_.end()) {
            return it->first;
        }
        return stops_.emplace_back(s);
    }

    std::string_view TransportCatalogue::GetClonedBus(std::string_view s) {
        const auto it = bus_data_.find(s);
        if (it != bus_data_.end()) {
            return it->first;
        }
        return buses_.emplace_back(s);
    }

}