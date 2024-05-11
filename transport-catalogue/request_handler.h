#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include <unordered_set>
#include <optional>


// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
/*
namespace handler {

    class RequestHandler {
    public:
        // MapRenderer понадобится в следующей части итогового проекта
        RequestHandler(const model::TransportCatalogue& db, const renderer::MapRenderer& renderer);

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<model::RouteInfo> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через
        const std::unordered_set<BusPtr>* GetBusesByStop(std::string_view stop_name) const;

        // Этот метод будет нужен в следующей части итогового проекта
        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const model::TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
    };

}
*/

