#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace transport_catalogue 
{
	struct Stop
	{
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Bus
	{
		std::string name;
		std::vector<const Stop*> stops;
		bool is_roundtrip;
	};

	struct BusInfo
	{
		std::string name;
		int amount_stops;
		int uniq_stops;
		double route_length;
		double curvature;
	};
} // namespace domain