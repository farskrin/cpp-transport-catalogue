#include "stat_reader.h"
#include <iomanip>

namespace io {

    void ParseAndPrintStat(const model::TransportCatalogue& transport_catalogue, std::string_view request,
        std::ostream& output) {
        
        auto space_pos = request.find(' ');
        if (space_pos == request.npos) {
            return;
        }

        auto not_space = request.find_first_not_of(' ', space_pos);
        if (not_space == request.npos) {
            return;
        }

        std::string_view command = request.substr(0, space_pos);
        std::string_view id = request.substr(not_space);

        if (command == "Bus") {
            if (auto info = transport_catalogue.GetRouteInfoByBusName(std::string(id)); info.has_value()) {
                output << request << ": " << info->stop_count << " stops on route, " << info->unique_stop_count
                    << " unique stops, " << std::setprecision(6) << info->route_length << " route length " << std::endl;
            }
            else
            {
                output << request << ": not found " << std::endl;
            }
        }
        if (command == "Stop") {
            if (auto buses = transport_catalogue.GetBusesByStop(id); buses.has_value()) {
                if (!buses.value().empty()) {
                    output << request << ": buses ";
                    for (const auto& bus_name : buses.value()) {
                        output << bus_name << " ";
                    }
                    output << std::endl;
                }
                else
                {
                    output << request << ": no buses " << std::endl;;
                }                
            }
            else
            {
                output << request << ": not found " << std::endl;
            }
        }


    }

}