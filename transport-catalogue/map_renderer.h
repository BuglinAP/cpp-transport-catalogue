#pragma once

#include "svg.h"
#include "transport_catalogue.h"
#include "domain.h"

#include <cmath>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include <deque>
#include <algorithm>
#include <iostream>
#include <variant>

namespace map_renderer
{
    namespace detail
    {
        std::deque<geo::Coordinates> FilterCoordinates(const transport_catalogue::TransportCatalogue& catalogue,
			const std::unordered_map<std::string_view, domain::Stop*>& stops);

        inline const double EPSILON = 1e-6;
        inline bool IsZero(double value)
        {
            return std::abs(value) < EPSILON;
        }

		class SphereProjector 
		{
		public:
			template <typename PointInputIt>
			SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
				double max_height, double padding)
				: padding_(padding) {
				if (points_begin == points_end) 
				{
					return;
				}

				const auto [left_it, right_it]
					= std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) 
				{
					return lhs.lng < rhs.lng;
				});
				min_lon_ = left_it->lng;
				const double max_lon = right_it->lng;

				const auto [bottom_it, top_it]
					= std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) 
				{
					return lhs.lat < rhs.lat;
				});
				const double min_lat = bottom_it->lat;
				max_lat_ = top_it->lat;

				std::optional<double> width_zoom;
				if (!IsZero(max_lon - min_lon_)) 
				{
					width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
				}

				std::optional<double> height_zoom;
				if (!IsZero(max_lat_ - min_lat))
				{
					height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
				}

				if (width_zoom && height_zoom)
				{
					zoom_coeff_ = std::min(*width_zoom, *height_zoom);
				}
				else if (width_zoom)
				{
					zoom_coeff_ = *width_zoom;
				}
				else if (height_zoom)
				{
					zoom_coeff_ = *height_zoom;
				}
			}

			svg::Point operator()(geo::Coordinates coords) const
			{
				return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
						(max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
			}

		private:
			double padding_;
			double min_lon_ = 0;
			double max_lat_ = 0;
			double zoom_coeff_ = 0;
		};
    }//namespace detail
    
// парметры рендеринга
struct RenderSettings
{
    svg::Point size;

    double padding = 0.0;

    double line_width = 0.0;
    double stop_radius = 0;

    int bus_label_font_size = 0;
    svg::Point bus_label_offset;

    int stop_label_font_size = 0;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width = 0.0;

    std::vector<svg::Color> color_palette;
};

class MapRenderer final
{
public:
    using Buses = std::map<std::string_view, const domain::Bus*>;
    using Stops = std::map<std::string_view, const domain::Stop*>;
    using StopBuses = std::unordered_map<std::string_view, std::set<std::string>>;

    void SetSettings(const RenderSettings &settings);

    svg::Document RenderMap(const transport_catalogue::TransportCatalogue &catalogue);

private:
	RenderSettings settings_;

    void RenderLines(svg::Document& document, const Buses& buses, const detail::SphereProjector& sphere_projector) const;
    void RenderBusNames(svg::Document& document, const Buses& buses, const detail::SphereProjector& sphere_projector) const;
    void RenderStops(svg::Document& document, const Stops& stops, const StopBuses &stop_buses, const detail::SphereProjector& sphere_projector) const;
    void RenderStopNames(svg::Document& document, const Stops& stops, const StopBuses & stop_buses, const detail::SphereProjector& sphere_projector) const;
};
}// namespace map_renderer