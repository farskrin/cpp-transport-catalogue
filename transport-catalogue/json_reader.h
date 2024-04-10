#pragma once

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "request_handler.h"


/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
* 
 */

namespace io {

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     * Возвращает маршрут и конечные точки маршрута
     */
    std::pair<std::vector<std::string>, std::vector<std::string>> ParseRoute(const json::Node stops_node, bool is_roundtrip);


    class JsonReader {
    public:
        explicit JsonReader(std::istream& input)
            : doc_(json::Load(input)) {
        }

        void ApplyBaseRequests(model::TransportCatalogue& catalogue) const;

        renderer::RenderSettings ParseRenderSettings() const;

        void ApplyStatRequests(const model::TransportCatalogue& catalogue) const;

    private:
        json::Document doc_;
    };


}














