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
        GenerateOutput();
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
        domain::Stop stop;
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
        for (const domain::Stop& stop : request_stops_)
        {
            transport_catalogue_.AddStop(stop);
        }
        int i = 0;
        for (const domain::Stop& stop : request_stops_)
        {
            transport_catalogue_.SetDistance(stop.name, distances_to_stops_[i]);
            ++i;
        }
        for (const auto& bus : request_buses_)
        {
            transport_catalogue_.AddBus(bus);
        }
    }

    void JsonReader::GenerateOutput()
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
                    RenderMap(request, answers);
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
            json::Node bus_output = json::Dict
            {
                {"curvature"s, bus_info.value().curvature},
                {"request_id"s, id},
                {"route_length"s, bus_info.value().route_length},
                {"stop_count"s, bus_info.value().amount_stops},
                {"unique_stop_count"s, bus_info.value().uniq_stops}
            };
            result.emplace_back(bus_output);
        }
        else
        {
            json::Node empty_bus_output = json::Dict
            {
                {"error_message"s , "not found"s},
                {"request_id"s, id}
            };
            result.emplace_back(empty_bus_output);
        }
    }

    void JsonReader::OutputStopInfo(const json::Node& request, json::Array& result) const
    {
        const std::string& stop_name = request.AsDict().at("name"s).AsString();
        int id = request.AsDict().at("id"s).AsInt();
        const domain::Stop* stop = transport_catalogue_.FindStop(stop_name);
        if (stop == nullptr)
        {
            json::Node empty_stop_output = json::Dict
            {
                {"error_message"s , "not found"s},
                {"request_id"s, id}
            };
            result.emplace_back(empty_stop_output);

        }
        else
        {
            auto stop_buses = transport_catalogue_.GetStopBuses(stop_name);
            json::Array routes;
            std::copy(stop_buses.begin(), stop_buses.end(), std::back_inserter(routes));
            json::Node stop_output = json::Dict
            {
                {"buses"s, routes},
                {"request_id"s,  id}
            };
            result.emplace_back(stop_output);
        }
    }

    void JsonReader::RenderMap(const json::Node& request, json::Array& result) const
    {
        int id = request.AsDict().at("id"s).AsInt();
        std::ostringstream out;
        map_renderer::MapRenderer renderer;
        renderer.SetSettings(LoadRenderSettings());
        renderer.RenderMap(transport_catalogue_).Render(out);
        json::Node answer_map = json::Dict
        {
            {"map"s, out.str()},
            {"request_id"s, id}
        };
        result.emplace_back(answer_map);
    }

    map_renderer::RenderSettings JsonReader::LoadRenderSettings() const
    {
        if (data_document_.GetRoot().IsMap() && data_document_.GetRoot().AsDict().count("render_settings"s) > 0)
        {
            auto& render_settings = data_document_.GetRoot().AsDict().at("render_settings"s);
            if (render_settings.IsMap())
            {
                return detail_load::Settings(render_settings.AsDict());
            }
        }
        return {};
    }

    namespace detail_load
    {
        map_renderer::RenderSettings Settings(const json::Dict& data)
        {
            map_renderer::RenderSettings result;

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