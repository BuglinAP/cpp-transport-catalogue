#pragma once
#include "geo.h"
#include "domain.h"

#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <unordered_set>
#include <iostream>
#include <set>
#include <optional>

namespace transport_catalogue
{
	namespace detail
	{
		struct PairStopHasher
		{
			std::size_t operator()(const std::pair<const Stop*, const Stop*>& pair_stop) const;
		};
	}// namespace detail

	class TransportCatalogue final
	{
	public:
		void AddStop(Stop stop);

		const Stop* FindStop(std::string_view stop) const;

		std::set<std::string> GetStopBuses(std::string_view stop) const;

		void SetDistance(const std::string& stop, std::vector<std::pair<std::string, int>>& distances_to_stops);

		int GetDistance(const Stop* stop_ptr, const Stop* anoter_stop_ptr) const;

		void AddBus(const std::string& bus_name, bool is_roundtrip, const std::vector<std::string>& bus_stops);

		const Bus* FindBus(std::string_view bus) const;

		std::optional <BusInfo> GetBusInfo(std::string_view bus) const;

		const std::unordered_map<std::string_view, Bus*>& GetBusnameToBus() const;

		const std::unordered_map<std::string_view, Stop*>& GetStopnameToStop() const;

		const std::unordered_map<std::string_view, std::set<std::string>>& GetStopnameToBusnames() const;

		const std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::PairStopHasher>& GetDistances() const;

	private:
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;

		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, std::set<std::string>> stopname_to_busnames_;

		std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::PairStopHasher> stops_distances_;
	};
}//namespace transport_catalogue