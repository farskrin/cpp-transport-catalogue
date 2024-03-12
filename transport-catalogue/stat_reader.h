#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace io {

    void ParseAndPrintStat(const model::TransportCatalogue& transport_catalogue, std::string_view request,
                           std::ostream& output);
}
