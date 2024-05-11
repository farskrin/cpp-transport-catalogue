#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std;
using namespace model;
using namespace io;

std::stringstream ReadFile(const std::string& file_name) {
    std::ifstream f(file_name);
    if (!f.is_open()) {
        throw std::runtime_error("Error: can not open file: "s + file_name);
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss;
}

int main() {

    TransportCatalogue catalogue;

    JsonReader reader(std::cin);
    reader.ApplyBaseRequests(catalogue);
    reader.ApplyStatRequests(catalogue);

    /*auto settings = reader.ParseRenderSettings();
    renderer::MapRenderer map_renderer(catalogue, settings);
    auto doc = map_renderer.RenderMap();
    doc.Render(std::cout);*/

    return 0;
}