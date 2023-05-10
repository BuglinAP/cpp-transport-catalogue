#include "map_renderer.h"

using namespace std::literals;

namespace map_renderer
{
    namespace detail
    {
        std::deque<geo::Coordinates> FilterCoordinates(const transport_catalogue::TransportCatalogue& catalogue,
            const std::unordered_map<std::string_view, domain::Stop*>& stops)
        {
            std::deque<geo::Coordinates> result;
            for (const auto& stop : stops)
            {
                if (catalogue.GetStopBuses(stop.second->name).size() > 0)
                {
                    result.push_back(stop.second->coordinates);
                }
            }
            return result;
        }
    }//namespace detail

void MapRenderer::SetSettings(const RenderSettings &settings)
{
    settings_ = settings;
}

svg::Document MapRenderer::RenderMap(const transport_catalogue::TransportCatalogue &transport_catalogue) 
{
    const auto &buses = transport_catalogue.GetBusnameToBus();
    const auto &stops = transport_catalogue.GetStopnameToStop();

    const auto& stop_coordinates = detail::FilterCoordinates(transport_catalogue, stops);
    detail::SphereProjector sphere_projector(stop_coordinates.begin(), stop_coordinates.end(),
        settings_.size.x, settings_.size.y, settings_.padding);

    Buses sorted_buses;
    Stops sorted_stops;
    for (auto &bus : buses)
    {
        sorted_buses.insert(bus);
    }
    for (auto &stop : stops)
    {
        sorted_stops.insert(stop);
    }

    const auto &stop_buses = transport_catalogue.GetStopnameToBusnames();

    svg::Document document;
    RenderLines(document, sorted_buses, sphere_projector);
    RenderBusNames(document, sorted_buses, sphere_projector);
    RenderStops(document, sorted_stops, stop_buses, sphere_projector);
    RenderStopNames(document, sorted_stops, stop_buses, sphere_projector);
    return document;
}

void MapRenderer::RenderLines(svg::Document& document, const Buses& buses, const detail::SphereProjector& sphere_projector) const
{
    auto max_color_count = settings_.color_palette.size();
    size_t color_index = 0;
    for (const auto &bus : buses)
    {
        if (bus.second->stops.size() == 0)
        {
            continue;
        }
       
        svg::Polyline line;
        line.SetStrokeColor(settings_.color_palette.at(color_index % max_color_count)).
            SetFillColor(svg::NoneColor).SetStrokeWidth(settings_.line_width).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        for (auto iter = bus.second->stops.begin(); iter < bus.second->stops.end(); ++iter)
        {
            line.AddPoint(sphere_projector((*iter)->coordinates));
        }
        
        if (bus.second->is_roundtrip == false)
        {
            for (auto iter = std::next(bus.second->stops.rbegin()); iter < bus.second->stops.rend(); ++iter)
            {
                line.AddPoint(sphere_projector((*iter)->coordinates));
            }
        }
        document.Add(line);
        ++color_index;
    }
}

void MapRenderer::RenderBusNames(svg::Document &document, const Buses &buses, const detail::SphereProjector& sphere_projector) const
{
    auto max_color_count = settings_.color_palette.size();
    size_t color_index = 0;
    for (const auto &bus : buses) 
    {
        if (bus.second->stops.size() > 0) 
        {
         
            svg::Text text, underlayer_text;
            text.SetData(std::string(bus.first)).
                    SetPosition(sphere_projector(bus.second->stops.front()->coordinates)).
                    SetOffset(settings_.bus_label_offset).
                    SetFontSize(static_cast<std::uint32_t>(settings_.bus_label_font_size)).
                    SetFontFamily("Verdana"s).SetFontWeight("bold");
            underlayer_text = text;
        
            text.SetFillColor(settings_.color_palette.at(color_index % max_color_count));
            underlayer_text.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).
                    SetStrokeWidth(settings_.underlayer_width).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
           
            document.Add(underlayer_text);
            document.Add(text);
           
            if (bus.second->is_roundtrip == false &&
                    bus.second->stops.back() != bus.second->stops.front()) 
            {
                text.SetPosition(sphere_projector(bus.second->stops.back()->coordinates));
                underlayer_text.SetPosition(sphere_projector(bus.second->stops.back()->coordinates));
                document.Add(underlayer_text);
                document.Add(text);
            }
            ++color_index;
        }
    }
}

void MapRenderer::RenderStops(svg::Document &document, const Stops &stops, const StopBuses &stop_buses, const detail::SphereProjector& sphere_projector) const
{
    for (const auto &stop : stops)
    {
        if (stop_buses.count(stop.first) != 0)
        {
           
            svg::Circle circle;
            circle.SetCenter(sphere_projector(stop.second->coordinates)).
                    SetRadius(settings_.stop_radius).SetFillColor("white"s);
            document.Add(circle);
        }
    }
}

void MapRenderer::RenderStopNames(svg::Document &document, const Stops &stops, const StopBuses &stop_buses, const detail::SphereProjector& sphere_projector) const
{
    for (const auto &stop : stops) 
    {
        
        if (stop_buses.count(stop.first) != 0) 
        {
            
            svg::Text text, underlayer_text;
            text.SetData(std::string(stop.first)).SetPosition(sphere_projector(stop.second->coordinates)).
                    SetOffset(settings_.stop_label_offset).
                    SetFontSize(static_cast<std::uint32_t>(settings_.stop_label_font_size)).
                    SetFontFamily("Verdana");
            underlayer_text = text;
            
            text.SetFillColor("black");
            underlayer_text.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).
                    SetStrokeWidth(settings_.underlayer_width).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
           
            document.Add(underlayer_text);
            document.Add(text);
        }
    }
}
}// namespace map_renderer