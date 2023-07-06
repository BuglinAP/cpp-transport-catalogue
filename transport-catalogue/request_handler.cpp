#include <fstream>
#include <memory>

#include "request_handler.h"

using namespace std::literals;

namespace transport_catalogue
{
	bool TransportCatalogueHandler::InitRouter()
	{
		if (!router_)
		{
			return ReInitRouter();
		}
		return true;
	}

	void TransportCatalogueHandler::LoadDataFromJson(JsonReader& json)
	{
		json.ReadRequests();
		render_settings_ = json.LoadRenderSettings();
		serialize_settings_ = json.LoadSerializeSettings();
		routing_settings_ = json.LoadRoutingSettings();
	}

	void TransportCatalogueHandler::LoadSerializeSettings(JsonReader& json)
	{
		serialize_settings_ = json.LoadSerializeSettings();
	}

	void TransportCatalogueHandler::LoadRequestsAndAnswer(JsonReader& json)
	{
		if (!InitRouter())
		{
			std::cerr << "Can't init Transport Router"s << std::endl;
			return;
		}
		json.GenerateOutput(render_settings_.value(), *router_);
	}

	bool TransportCatalogueHandler::SerializeData()
	{
		if (!serialize_settings_)
		{
			std::cerr << "Can't find Serialize Settings : "s << std::endl;
			return false;
		}
		serialize::Serializator serializator(serialize_settings_.value());
		serializator.AddTransportCatalogue(catalogue_);
		if (render_settings_)
		{
			serializator.AddRenderSettings(render_settings_.value());
		}
		if (routing_settings_)
		{
			InitRouter();
			router_->InitRouter();
			serializator.AddTransportRouter(*router_.get());
		}
		return serializator.Serialize();
	}

	bool TransportCatalogueHandler::DeserializeData()
	{
		if (!serialize_settings_)
		{
			std::cerr << "Can't find Serialize Settings : "s << std::endl;
			return false;
		}
		serialize::Serializator serializator(serialize_settings_.value());
		if (serializator.Deserialize(catalogue_, render_settings_, router_))
		{
			if (router_)
			{
				routing_settings_ = router_->GetSettings();
			}
			return true;
		}
		return false;
	}

	bool TransportCatalogueHandler::ReInitRouter()
	{
		if (routing_settings_)
		{
			router_ = std::make_unique<transport_catalogue::TransportRouter>(catalogue_, routing_settings_.value());
			return true;
		}
		else
		{
			std::cerr << "Can't find routing settings"s << std::endl;
			return false;
		}
	}
} // namespace transport_catalogue
