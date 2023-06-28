#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace transport_catalogue
{
	// перевод км/ч в м/мин
	constexpr static double KMH_TO_MMIN = 1000.0 / 60.0;

	struct RouteWeight
	{
		std::string_view bus_name;
		double total_time = 0;
		int span_count = 0;
	};

	struct RoutingSettings
	{
		int wait_time = 0;      // в минутах
		double velocity = 100;  // в метрах-в-минуту
	};

	bool operator<(const RouteWeight& left, const RouteWeight& right);
	bool operator>(const RouteWeight& left, const RouteWeight& right);
	RouteWeight operator+(const RouteWeight& left, const RouteWeight& right);

	class TransportRouter
	{
	public:

		struct RouterEdge
		{
			std::string_view bus_name;
			std::string_view stop_from;
			std::string_view stop_to;
			double total_time = 0;
			int span_count = 0;
		};

		using Graph = graph::DirectedWeightedGraph<RouteWeight>;
		using StopsById = std::unordered_map<size_t, const Stop*>;
		using IdsByStopName = std::unordered_map<std::string_view, size_t>;
		using Router = graph::Router<RouteWeight>;
		using TransportRoute = std::vector<RouterEdge>;

		TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, const RoutingSettings& settings);

		void InitRouter();

		std::optional<TransportRoute> BuildRoute(const std::string& from, const std::string& to);

		const RoutingSettings& GetSettings() const;
		RoutingSettings& GetSettings();

	private:
		const transport_catalogue::TransportCatalogue& catalogue_;
		RoutingSettings settings_;

		StopsById stops_by_id_;
		IdsByStopName id_by_stop_name_;

		Graph graph_;
		mutable std::unique_ptr<Router> router_;

		void BuildEdges();
		size_t CountStops();
		graph::Edge<RouteWeight> MakeEdge(const Bus* bus, int stop_from_index, int stop_to_index);
		double ComputeRouteTime(const Bus* bus, int stop_from_index, int stop_to_index);
	};
}  // namespace transport_catalogue