#pragma once

#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"
#include <iostream>
#include <variant>
#include <memory>

using namespace std::literals;

namespace routing {

	struct RoutingSettings {
		int bus_wait_time = 0;		//	min
		double bus_velocity = 0.;	//	m/min
	};

	struct WaitItem {
		WaitItem(double time, std::string_view stop_name)
			: time(time), stop_name(stop_name) {
		}		
		double time = 0.;
		std::string stop_name;
		std::string type = "Wait"s;
	};

	struct BusItem {
		BusItem(double time, const std::string& bus_name, int span_count)
			: time(time), bus_name(bus_name), span_count(span_count){
		}		
		double time = 0.;
		std::string bus_name;
		std::string type = "Bus"s;
		int span_count = 0;
	};

	using Item = std::variant<WaitItem, BusItem>;

	struct ResponseData {
		double total_time = 0.;
		std::vector<Item> items;
	};

	class TransportRouter {
	public:
		using Weight = double;
		using Graph = graph::DirectedWeightedGraph<Weight>;
		using Router = graph::Router<Weight>;

		TransportRouter(const model::TransportCatalogue& catalogue, const RoutingSettings& settings);

		std::optional<ResponseData> BuildRoute(std::string_view from, std::string_view to) const;

	private:
		struct StopVertices {
			graph::VertexId start = 0;
			graph::VertexId end = 0;
		};

		struct EdgeHash {
		public:
			size_t operator()(const graph::Edge<Weight>& edge) const {
				return hasher_(edge.from) + 37 * hasher_(edge.to) + 37 * 37 * hasher_(edge.weight);
			}
		private:
			std::hash<size_t> hasher_;
		};
		
		void BuildStopsVertices(const std::set<std::string_view>& stops);
		void AddBusEdges(const model::Bus& bus);
		void BuildGraph(const std::deque<model::Bus>& buses);

		const model::TransportCatalogue& catalogue_;
		RoutingSettings settings_;
		std::unordered_map<std::string_view, StopVertices> stop_to_vertex_;
		std::unordered_map<graph::Edge<Weight>, Item, EdgeHash> edge_to_item_;
		std::unique_ptr<Graph> graph_ = nullptr;
		std::unique_ptr<Router> router_ = nullptr;
	};
}
