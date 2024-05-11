#include "transport_router.h"


namespace routing {

	TransportRouter::TransportRouter(const model::TransportCatalogue& catalogue, const RoutingSettings& settings)		
		: catalogue_(catalogue), settings_(settings) {

		BuildStopsVertices(catalogue.GetSortedStopsInTask());
		BuildGraph(catalogue.GetBuses());
		router_ = std::make_unique<Router>(*graph_);
	}

	std::optional<ResponseData> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
		std::optional<ResponseData> response;
		graph::VertexId id_from = stop_to_vertex_.at(from).start;
		graph::VertexId id_to = stop_to_vertex_.at(to).start;

		if (auto route = router_->BuildRoute(id_from, id_to)) {
			response.emplace(ResponseData());
			response->total_time = route->weight;

			for (auto edge_id : route->edges) {
				graph::Edge<Weight> edge = graph_->GetEdge(edge_id);

				response->items.emplace_back(edge_to_item_.at(edge));
			}
		}

		return response;
	}

	void TransportRouter::BuildStopsVertices(const std::set<std::string_view>& stops) {
		graph::VertexId start = 0;
		graph::VertexId end = 1;
		stop_to_vertex_.reserve(stops.size());
		for (std::string_view stop : stops) {
			stop_to_vertex_.emplace(stop, StopVertices{ start, end });
			start += 2;
			end += 2;
		}
	}

	void TransportRouter::AddBusEdges(const model::Bus& bus) {
		const auto& dist_time_span = catalogue_.GetRouteTimeAndSpan(bus.name, settings_.bus_velocity);
		for (const auto& [from, to, time, span_count] : dist_time_span) {
			auto vertex_from = stop_to_vertex_[from].end;
			auto vertex_to = stop_to_vertex_[to].start;

			auto bus_edge = graph::Edge<Weight>{ vertex_from, vertex_to, time };
			graph_->AddEdge(bus_edge);
			edge_to_item_.emplace(bus_edge, BusItem(time, bus.name, span_count));
		}
	}

	void TransportRouter::BuildGraph(const std::deque<model::Bus>& buses) {
		graph_ = std::make_unique<Graph>(stop_to_vertex_.size() * 2);

		double bus_wait_time = static_cast<double>(settings_.bus_wait_time);
		auto wait_edge = graph::Edge<Weight>{};

		for (auto [stop_name, stop_vertices] : stop_to_vertex_) {
			wait_edge = graph::Edge<Weight>{ stop_vertices.start, stop_vertices.end, bus_wait_time };

			graph_->AddEdge(wait_edge);
			edge_to_item_.emplace(wait_edge, WaitItem(bus_wait_time, stop_name));
		}

		for (const auto& bus : buses) {
			AddBusEdges(bus);
		}
	}


}