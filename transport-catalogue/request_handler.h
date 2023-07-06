#pragma once

#include "filesystem"
#include "optional"

#include "json_reader.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport_catalogue
{
	class TransportCatalogueHandler final
	{
	public:

		using Route = transport_catalogue::TransportRouter::TransportRoute;
		using RoutingSettings = transport_catalogue::RoutingSettings;

		explicit TransportCatalogueHandler(TransportCatalogue& catalogue)
			: catalogue_(catalogue) {}

		void LoadDataFromJson(JsonReader& json);

		void LoadSerializeSettings(JsonReader& json);

		void LoadRequestsAndAnswer(JsonReader& json);

		bool SerializeData();

		bool DeserializeData();

		bool ReInitRouter();

	private:
		bool InitRouter();

		TransportCatalogue& catalogue_;
		std::unique_ptr<TransportRouter> router_;

		std::optional<RenderSettings> render_settings_;
		std::optional<RoutingSettings> routing_settings_;
		std::optional<serialize::Serializator::Settings> serialize_settings_;
	};
} // namespace transport_catalogue
