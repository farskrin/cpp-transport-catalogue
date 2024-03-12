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
            if (auto bus = transport_catalogue.FindBusByName(id); bus != nullptr) {
                const auto& info = bus->route_info;
                output << request << ": " << info.stop_count << " stops on route, " << info.unique_stop_count 
                    << " unique stops, " << std::setprecision(6) << info.route_length << " route length " << std::endl;
            }
            else
            {
                output << request << ": not found " << std::endl;
            }
        }
        if (command == "Stop") {
            if (auto stop = transport_catalogue.FindStopByName(id); stop != nullptr) {
                if (!stop->stop_buses.empty()) {
                    output << request << ": buses ";
                    for (const auto& bus : stop->stop_buses) {
                        output << bus << " ";
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