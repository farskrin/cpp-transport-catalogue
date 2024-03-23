#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <set>

using namespace geo;
using namespace model;
using namespace std;

namespace io {

    /**
     * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
     */
    Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return {nan, nan};
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);
        auto comma2 = str.find(',', not_space2);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2, comma2 - not_space2)));

        return {lat, lng};
    }

    std::vector<std::pair<std::string, double>> ParseDistanceToStop(std::string_view str) {
        std::vector<std::pair<std::string, double>> result;
        auto comma = str.find(',');
        auto comma2 = str.find(',', comma + 1);
        if (comma2 == str.npos) return result;

        for (size_t pos = comma2; pos != str.npos; pos = str.find(',', pos + 1)) {
            auto num_start = str.find_first_not_of(' ', pos + 1);
            auto num_end = str.find('m', num_start);
            std::string snum = std::string(str.substr(num_start, num_end - num_start));
            double dnum = std::stod(snum);
            //--
            auto stop_start = str.find_first_not_of(' ', num_end + 4);
            auto stop_end = str.find(',', stop_start);
            std::string stop = std::string(str.substr(stop_start, stop_end - stop_start));
            result.push_back(std::move(std::make_pair(stop, dnum)));
        }

        return result;
    }

    /**
     * Удаляет пробелы в начале и конце строки
     */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    /**
     * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
     */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == line.npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return {std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1))};
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void InputReader::ApplyCommands(model::TransportCatalogue& catalogue) const {
        //local optimization
        std::vector<CommandDescription> commands_stop;
        std::vector<CommandDescription> commands_bus;
        for (const auto& command : commands_) {
            if (command.command == "Stop") {
                Coordinates coord = ParseCoordinates(command.description);                
                catalogue.AddStop(command.id, coord);
                commands_stop.push_back(command);
            }
            else
            {
                commands_bus.push_back(command);
            }
        }

        for (const auto& command : commands_stop) {
            auto dist_to_stop = ParseDistanceToStop(command.description);
            for (const auto& [to_stop, dist] : dist_to_stop) {
                catalogue.SetStopsDistance(command.id, to_stop, dist);
            }
        }

        for (const auto& command : commands_bus) {
            if (command.command == "Bus") {
                std::vector<std::string_view> route = ParseRoute(command.description);
                catalogue.AddBus(command.id, route);
            }
        }
    }

}
