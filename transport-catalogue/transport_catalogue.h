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
			std::size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*>& pair_stop) const;
		};
	}// namespace detail

	class TransportCatalogue final
	{
	public:
		void AddStop(domain::Stop stop);

		domain::Stop* FindStop(const std::string_view stop) const;

		std::set<std::string> GetStopBuses(std::string_view stop) const;

		void SetDistance(const std::string& stop, std::vector<std::pair<std::string, int>>& distances_to_stops);

		int GetDistance(const domain::Stop* stop_ptr, const domain::Stop* anoter_stop_ptr) const;

		void AddBus(const std::tuple<std::string, bool, std::vector<std::string>>& bus_input);

		const domain::Bus* FindBus(std::string_view bus) const;

		std::optional <domain::BusInfo> GetBusInfo(std::string_view bus) const;
        
        const std::unordered_map<std::string_view, domain::Bus*>& GetBusnameToBus() const;
        
        const std::unordered_map<std::string_view, domain::Stop*>& GetStopnameToStop() const;
		
        const std::unordered_map<std::string_view, std::set<std::string>>& GetStopnameToBusnames() const;

	private:
		std::deque<domain::Bus> buses_;
		std::unordered_map<std::string_view, domain::Bus*> busname_to_bus_;

		std::deque<domain::Stop> stops_;
		std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, std::set<std::string>> stopname_to_busnames_;

		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, detail::PairStopHasher> stops_distances_;
	};
}//namespace transport_catalogue