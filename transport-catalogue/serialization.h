#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"

namespace serialize 
{
	class Serializator final 
	{
	public:
		using ProtoTransportCatalogue = transport_catalogue_serialize::TransportCatalogue;
		using TransportCatalogue = transport_catalogue::TransportCatalogue;

		using ProtoTransportRouter = transport_router_serialize::TransportRouter;
		using TransportRouter = transport_catalogue::TransportRouter;

		struct Settings {
			std::filesystem::path path;
		};

		Serializator(const Settings& settings) : settings_(settings) {};

		// Добавляет данные для сериализации
		void AddTransportCatalogue(const TransportCatalogue& catalogue);
		void AddRenderSettings(const transport_catalogue::RenderSettings& settings);
		void AddTransportRouter(const TransportRouter& router);

		bool Serialize();

		bool Deserialize(TransportCatalogue& catalogue,
			std::optional<transport_catalogue::RenderSettings>& settings,
			std::unique_ptr<TransportRouter>& router_);

	private:
		void Clear() noexcept;

		void SaveStops(const TransportCatalogue& catalogue);
		void LoadStops(TransportCatalogue& catalogue);

		void SaveBuses(const TransportCatalogue& catalogue);
		void LoadBuses(TransportCatalogue& catalogue);

		void SaveBusesStops(const transport_catalogue::Bus& bus, transport_catalogue_serialize::Bus& p_bus);
		void LoadBus(TransportCatalogue& catalogue, const transport_catalogue_serialize::Bus& p_bus) const;

		void SaveDistances(const TransportCatalogue& catalogue);
		void LoadDistances(TransportCatalogue& catalogue) const;

		void SaveRenderSettings(const transport_catalogue::RenderSettings& settings);
		void LoadRenderSettings(std::optional<transport_catalogue::RenderSettings>& settings) const;

		void SaveTransportRouter(const TransportRouter& router);
		void LoadTransportRouter(const TransportCatalogue& catalogue,
			std::unique_ptr<TransportRouter>& transport_router);

		void SaveTransportRouterSettings(const transport_catalogue::RoutingSettings& routing_settings);
		void LoadTransportRouterSettings(transport_catalogue::RoutingSettings& routing_settings) const;

		void SaveGraph(const TransportRouter::Graph& graph);
		void LoadGraph(const TransportCatalogue& catalogue, TransportRouter::Graph& graph);

		void SaveRouter(const std::unique_ptr<TransportRouter::Router>& router);
		void LoadRouter(const TransportCatalogue& catalogue, std::unique_ptr<TransportRouter::Router>& router);

		static transport_catalogue_serialize::Coordinates MakeProtoCoordinates(const geo::Coordinates& coordinates);
		static geo::Coordinates MakeCoordinates(const transport_catalogue_serialize::Coordinates& p_coordinates);

		static svg_serialize::Point MakeProtoPoint(const svg::Point& point);
		static svg::Point MakePoint(const svg_serialize::Point& p_point);

		static svg_serialize::Color MakeProtoColor(const svg::Color& color);
		static svg::Color MakeColor(const svg_serialize::Color& p_color);

		graph_serialize::RouteWeight MakeProtoWeight(const transport_catalogue::RouteWeight& weight) const;
		transport_catalogue::RouteWeight MakeWeight(const TransportCatalogue& catalogue,
			const graph_serialize::RouteWeight& p_weight) const;

		Settings settings_;

		ProtoTransportCatalogue proto_catalogue_;
		std::unordered_map<int, std::string_view> stop_name_by_id_;
		std::unordered_map<std::string_view, int> stop_id_by_name_;
		std::unordered_map<int, std::string_view> route_name_by_id_;
		std::unordered_map<std::string_view, int> route_id_by_name_;
	};
} // namespace serialize
