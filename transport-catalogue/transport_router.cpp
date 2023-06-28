#include "transport_router.h"

namespace transport_catalogue
{
	bool operator<(const RouteWeight& left, const RouteWeight& right)
	{
		return left.total_time < right.total_time;
	}

	RouteWeight operator+(const RouteWeight& left, const RouteWeight& right)
	{
		RouteWeight result;
		result.total_time = left.total_time + right.total_time;
		return result;
	}

	bool operator>(const RouteWeight& left, const RouteWeight& right)
	{
		return left.total_time > right.total_time;
	}

	TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, const RoutingSettings& settings)
		: catalogue_(catalogue)
		, settings_(settings)
	{
		InitRouter();
	}

	void TransportRouter::InitRouter()
	{
		graph::DirectedWeightedGraph<RouteWeight>graph(CountStops());
		graph_ = std::move(graph);
		BuildEdges();
		router_ = std::make_unique<graph::Router<RouteWeight>>(graph_);
	}

	std::optional<TransportRouter::TransportRoute> TransportRouter::BuildRoute(const std::string& from, const std::string& to)
	{
		if (from == to)
		{
			return TransportRoute{};
		}
		auto from_id = id_by_stop_name_.at(from);
		auto to_id = id_by_stop_name_.at(to);
		auto route = router_->BuildRoute(from_id, to_id);
		if (!route)
		{
			return std::nullopt;
		}

		TransportRoute result;
		for (auto edge_id : route->edges)
		{
			const auto& edge = graph_.GetEdge(edge_id);
			RouterEdge route_edge;
			route_edge.bus_name = edge.weight.bus_name;
			route_edge.stop_from = stops_by_id_.at(edge.from)->name;
			route_edge.stop_to = stops_by_id_.at(edge.to)->name;
			route_edge.span_count = edge.weight.span_count;
			route_edge.total_time = edge.weight.total_time;
			result.push_back(route_edge);
		}
		return result;
	}

	const RoutingSettings& TransportRouter::GetSettings() const
	{
		return settings_;
	}

	RoutingSettings& TransportRouter::GetSettings()
	{
		return settings_;
	}

	void TransportRouter::BuildEdges()
	{
		for (const auto& [route_name, route] : catalogue_.GetBusnameToBus())
		{
			int stops_count = static_cast<int>(route->stops.size());
			for (int i = 0; i < stops_count - 1; ++i)
			{
				double route_time = settings_.wait_time;
				double route_time_back = settings_.wait_time;
				for (int j = i + 1; j < stops_count; ++j)
				{
					graph::Edge<RouteWeight> edge = MakeEdge(route, i, j);
					route_time += ComputeRouteTime(route, j - 1, j);
					edge.weight.total_time = route_time;
					graph_.AddEdge(edge);

					if (!route->is_roundtrip)
					{
						int i_back = stops_count - 1 - i;
						int j_back = stops_count - 1 - j;
						graph::Edge<RouteWeight> edge = MakeEdge(route, i_back, j_back);
						route_time_back += ComputeRouteTime(route, j_back + 1, j_back);
						edge.weight.total_time = route_time_back;
						graph_.AddEdge(edge);
					}
				}
			}
		}
	}

	size_t TransportRouter::CountStops()
	{
		size_t stops_counter = 0;
		const auto& stops = catalogue_.GetStopnameToStop();
		id_by_stop_name_.reserve(stops.size());
		stops_by_id_.reserve(stops.size());
		for (auto stop : stops)
		{
			id_by_stop_name_.insert({ stop.first, stops_counter });
			stops_by_id_.insert({ stops_counter++, stop.second });
		}
		return stops_counter;
	}

	graph::Edge<RouteWeight> TransportRouter::MakeEdge(const Bus* bus, int stop_from_index, int stop_to_index)
	{
		graph::Edge<RouteWeight> edge;
		edge.from = id_by_stop_name_.at(bus->stops.at(static_cast<size_t>(stop_from_index))->name);
		edge.to = id_by_stop_name_.at(bus->stops.at(static_cast<size_t>(stop_to_index))->name);
		edge.weight.bus_name = bus->name;
		edge.weight.span_count = static_cast<int>(stop_to_index - stop_from_index);
		return edge;
	}

	double TransportRouter::ComputeRouteTime(const Bus* bus, int stop_from_index, int stop_to_index)
	{
		auto split_distance = catalogue_.GetDistance
		(
			bus->stops.at(static_cast<size_t>(stop_from_index)),
			bus->stops.at(static_cast<size_t>(stop_to_index))
		);
		return split_distance / settings_.velocity;
	}
}  // namespace transport_catalogue