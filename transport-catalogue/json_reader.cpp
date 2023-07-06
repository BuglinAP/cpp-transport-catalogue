#include "json_reader.h"

using namespace std::literals;

namespace transport_catalogue
{
	JsonReader::JsonReader(TransportCatalogue& transport_catalogue, std::istream& input_stream)
		: transport_catalogue_(transport_catalogue)
		, data_document_(json::Load(input_stream))
	{
	}

	void JsonReader::ReadRequests()
	{
		ReadBaseRequests();
		LoadBaseRequestsToCatalog();
	}

	void JsonReader::ReadBaseRequests()
	{
		const auto& base_requests = ((data_document_.GetRoot()).AsDict()).at("base_requests"s);
		for (const auto& base : base_requests.AsArray())
		{
			const auto& request = base.AsDict();
			if (request.at("type"s) == "Stop"s)
			{
				ReadStop(request);
			}
			else if (request.at("type"s) == "Bus"s)
			{
				ReadBus(request);
			}
		}
	}

	void JsonReader::ReadStop(const json::Dict& request_stop)
	{
		Stop stop;
		std::vector<std::pair<std::string, int>> distances_to_stops;

		stop.name = request_stop.at("name"s).AsString();
		stop.coordinates.lat = request_stop.at("latitude"s).AsDouble();
		stop.coordinates.lng = request_stop.at("longitude"s).AsDouble();
		auto road_distances = request_stop.at("road_distances"s);
		for (const auto& distances : road_distances.AsDict())
		{
			distances_to_stops.push_back(make_pair(distances.first, distances.second.AsInt()));
		}
		request_stops_.push_back(stop);
		distances_to_stops_.push_back(distances_to_stops);
	}

	void JsonReader::ReadBus(const json::Dict& request_bus)
	{
		std::string bus_name;
		bool is_roundtrip;
		std::vector<std::string> bus_stops;

		bus_name = request_bus.at("name"s).AsString();
		is_roundtrip = request_bus.at("is_roundtrip"s).AsBool();
		for (const auto& stop : request_bus.at("stops"s).AsArray())
		{
			bus_stops.push_back(stop.AsString());
		}
		request_buses_.push_back(std::make_tuple(bus_name, is_roundtrip, bus_stops));
	}

	void JsonReader::LoadBaseRequestsToCatalog()
	{
		for (const Stop& stop : request_stops_)
		{
			transport_catalogue_.AddStop(stop);
		}
		int i = 0;
		for (const Stop& stop : request_stops_)
		{
			transport_catalogue_.SetDistance(stop.name, distances_to_stops_[i]);
			++i;
		}
		for (const auto& [bus_name, is_roundtrip, bus_stops] : request_buses_)
		{
			transport_catalogue_.AddBus(bus_name, is_roundtrip, bus_stops);
		}
	}

	void JsonReader::GenerateOutput(const RenderSettings& render_settings, TransportRouter& router)
	{
		std::ostream& out = std::cout;

		auto& requests = data_document_.GetRoot().AsDict().at("stat_requests"s);
		if (requests.IsArray())
		{
			json::Array answers;
			for (const auto& request : requests.AsArray())
			{
				const std::string& type = request.AsDict().at("type"s).AsString();
				if (type == "Bus"s)
				{
					OutputBusInfo(request, answers);
				}
				else if (type == "Stop"s)
				{
					OutputStopInfo(request, answers);
				}
				else if (type == "Map"s)
				{
					RenderMap(request, answers, render_settings);
				}
				else if (type == "Route"s)
				{
					OutputRouteInfo(request, answers, router);
				}
			}
			json::Print(json::Document{ answers }, out);
		}
	}

	void JsonReader::OutputBusInfo(const json::Node& request, json::Array& result) const
	{
		const std::string& bus_name = request.AsDict().at("name"s).AsString();
		int id = request.AsDict().at("id"s).AsInt();
		auto bus_info = transport_catalogue_.GetBusInfo(bus_name);
		if (bus_info.has_value())
		{
			json::Node bus_output =
				json::Builder{}.StartDict().
				Key("curvature"s).Value(bus_info.value().curvature).
				Key("route_length"s).Value(bus_info.value().route_length).
				Key("stop_count"s).Value(bus_info.value().amount_stops).
				Key("unique_stop_count"s).Value(bus_info.value().uniq_stops).
				Key("request_id"s).Value(id).
				EndDict().Build().AsDict();
			result.emplace_back(bus_output);
		}
		else
		{
			json::Node empty_bus_output =
				json::Builder{}.StartDict().
				Key("error_message"s).Value("not found"s).
				Key("request_id"s).Value(id).
				EndDict().Build().AsDict();
			result.emplace_back(empty_bus_output);
		}
	}

	void JsonReader::OutputStopInfo(const json::Node& request, json::Array& result) const
	{
		const std::string& stop_name = request.AsDict().at("name"s).AsString();
		int id = request.AsDict().at("id"s).AsInt();
		const Stop* stop = transport_catalogue_.FindStop(stop_name);
		if (stop == nullptr)
		{
			json::Node empty_stop_output =
				json::Builder{}.StartDict().
				Key("error_message"s).Value("not found"s).
				Key("request_id"s).Value(id).
				EndDict().Build().AsDict();
			result.emplace_back(empty_stop_output);

		}
		else
		{
			auto stop_buses = transport_catalogue_.GetStopBuses(stop_name);
			json::Array buses;
			std::copy(stop_buses.begin(), stop_buses.end(), std::back_inserter(buses));
			json::Node stop_output = json::Builder{}.StartDict().
				Key("buses"s).Value(buses).
				Key("request_id"s).Value(id).
				EndDict().Build().AsDict();
			result.emplace_back(stop_output);
		}
	}

	void JsonReader::RenderMap(const json::Node& request, json::Array& result, const RenderSettings& render_settings) const
	{
		int id = request.AsDict().at("id"s).AsInt();
		const auto& buses = transport_catalogue_.GetBusnameToBus();
		const auto& stops = transport_catalogue_.GetStopnameToStop();
		const auto& stop_buses = transport_catalogue_.GetStopnameToBusnames();
		std::ostringstream out;

		MapRenderer renderer;
		renderer.SetSettings(render_settings);
		renderer.RenderMap(buses, stops, stop_buses).Render(out);
		json::Node answer_map =
			json::Builder{}.StartDict().
			Key("map"s).Value(out.str()).
			Key("request_id"s).Value(id).
			EndDict().Build().AsDict();
		result.emplace_back(answer_map);
	}

	void JsonReader::OutputRouteInfo(const json::Node& request, json::Array& result, TransportRouter& router) const
	{
		int id = request.AsDict().at("id"s).AsInt();
		const auto& from = request.AsDict().at("from"s).AsString();
		const auto& to = request.AsDict().at("to"s).AsString();

		auto route = router.BuildRoute(from, to);

		if (!route.has_value())
		{
			json::Node error_message =
				json::Builder{}.StartDict().
				Key("request_id"s).Value(id).
				Key("error_message"s).Value("not found"s).
				EndDict().Build().AsDict();
			result.emplace_back(error_message);
			return;
		}

		double total_time = 0;
		int wait_time = router.GetSettings().wait_time;
		json::Array items;
		for (const auto& edge : route.value())
		{
			total_time += edge.total_time;
			json::Dict wait_elem =
				json::Builder{}.StartDict().
				Key("type"s).Value("Wait"s).
				Key("stop_name"s).Value(std::string(edge.stop_from)).
				Key("time"s).Value(wait_time).
				EndDict().Build().AsDict();
			json::Dict ride_elem =
				json::Builder{}.StartDict().
				Key("type"s).Value("Bus"s).
				Key("bus"s).Value(std::string(edge.bus_name)).
				Key("span_count"s).Value(edge.span_count).
				Key("time"s).Value(edge.total_time - wait_time).
				EndDict().Build().AsDict();
			items.push_back(wait_elem);
			items.push_back(ride_elem);
		}
		json::Node route_output =
			json::Builder{}.StartDict().
			Key("request_id"s).Value(id).
			Key("total_time"s).Value(total_time).
			Key("items"s).Value(items).
			EndDict().Build().AsDict();
		result.emplace_back(route_output);
	}

	std::optional<RenderSettings> JsonReader::LoadRenderSettings() const
	{
		if (data_document_.GetRoot().IsDict() && data_document_.GetRoot().AsDict().count("render_settings"s) > 0)
		{
			auto& render_settings = data_document_.GetRoot().AsDict().at("render_settings"s);
			if (render_settings.IsDict())
			{
				return detail_load::Settings(render_settings.AsDict());
			}
		}
		return std::nullopt;
	}

	std::optional<RoutingSettings> JsonReader::LoadRoutingSettings() const
	{
		auto& routing_settings = data_document_.GetRoot().AsDict().at("routing_settings"s).AsDict();
		if (routing_settings.count("bus_wait_time"s) && routing_settings.at("bus_wait_time"s).IsInt()
			&&
			routing_settings.count("bus_velocity"s) && routing_settings.at("bus_velocity"s).IsInt())
		{
			RoutingSettings result;
			result.wait_time = routing_settings.at("bus_wait_time"s).AsInt();
			result.velocity = routing_settings.at("bus_velocity"s).AsDouble() * KMH_TO_MMIN;
			return result;
		}
		return std::nullopt;
	}

	std::optional<serialize::Serializator::Settings> JsonReader::LoadSerializeSettings() const
	{
		// загружаем параметры сериализации, если они есть
		if (data_document_.GetRoot().IsDict() && data_document_.GetRoot().AsDict().count("serialization_settings"s) > 0)
		{
			auto& serialization_settngs = data_document_.GetRoot().AsDict().at("serialization_settings"s);
			if (serialization_settngs.IsDict() && serialization_settngs.AsDict().count("file"s) > 0) {
				serialize::Serializator::Settings result;
				result.path = serialization_settngs.AsDict().at("file"s).AsString();
				return result;
			}
		}
		return std::nullopt;
	}

	namespace detail_load
	{
		RenderSettings Settings(const json::Dict& data)
		{
			RenderSettings result;

			if (data.count("width"s) != 0 && data.at("width"s).IsDouble())
			{
				result.size.x = data.at("width"s).AsDouble();
			}
			if (data.count("height"s) != 0 && data.at("height"s).IsDouble())
			{
				result.size.y = data.at("height"s).AsDouble();
			}
			if (data.count("padding"s) != 0 && data.at("padding"s).IsDouble())
			{
				result.padding = data.at("padding"s).AsDouble();
			}
			if (data.count("line_width"s) != 0 && data.at("line_width"s).IsDouble())
			{
				result.line_width = data.at("line_width"s).AsDouble();
			}
			if (data.count("stop_radius"s) != 0 && data.at("stop_radius"s).IsDouble())
			{
				result.stop_radius = data.at("stop_radius"s).AsDouble();
			}
			if (data.count("bus_label_font_size"s) != 0 && data.at("bus_label_font_size"s).IsInt())
			{
				result.bus_label_font_size = data.at("bus_label_font_size"s).AsInt();
			}
			if (data.count("bus_label_offset"s) != 0 && data.at("bus_label_offset"s).IsArray())
			{
				result.bus_label_offset = Offset(data.at("bus_label_offset"s).AsArray());
			}
			if (data.count("stop_label_font_size"s) != 0 && data.at("stop_label_font_size"s).IsInt())
			{
				result.stop_label_font_size = data.at("stop_label_font_size"s).AsInt();
			}
			if (data.count("stop_label_offset"s) != 0 && data.at("stop_label_offset"s).IsArray())
			{
				result.stop_label_offset = Offset(data.at("stop_label_offset"s).AsArray());
			}
			if (data.count("underlayer_color"s) != 0)
			{
				result.underlayer_color = Color(data.at("underlayer_color"s));
			}
			if (data.count("underlayer_width"s) != 0 && data.at("underlayer_width"s).IsDouble())
			{
				result.underlayer_width = data.at("underlayer_width"s).AsDouble();
			}
			if (data.count("color_palette"s) != 0 && data.at("color_palette"s).IsArray())
			{
				for (auto& color : data.at("color_palette"s).AsArray())
				{
					result.color_palette.push_back(Color(color));
				}
			}
			return result;
		}

		svg::Point Offset(const json::Array& offset)
		{
			svg::Point result;
			if (offset.size() > 1)
			{
				if (offset.at(0).IsDouble())
				{
					result.x = offset.at(0).AsDouble();
				}
				if (offset.at(1).IsDouble())
				{
					result.y = offset.at(1).AsDouble();
				}
			}
			return result;
		}

		svg::Color Color(const json::Node& color)
		{
			if (color.IsString())
			{
				return color.AsString();
			}
			else if (color.IsArray() && color.AsArray().size() == 3)
			{
				auto result_color = svg::Rgb(static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
					static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
					static_cast<uint8_t>(color.AsArray().at(2).AsInt()));
				return result_color;
			}
			else if (color.IsArray() && color.AsArray().size() == 4)
			{
				auto result_color = svg::Rgba(static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
					static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
					static_cast<uint8_t>(color.AsArray().at(2).AsInt()),
					color.AsArray().at(3).AsDouble());
				return result_color;
			}
			else
			{
				return svg::NoneColor;
			}
		}
	}//namespace detail_load
}//namespace transport_catalogue