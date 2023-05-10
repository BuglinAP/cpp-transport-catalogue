#include "transport_catalogue.h"
#include "geo.h"

using namespace std::literals;
using namespace domain;

namespace transport_catalogue
{
	namespace detail
	{
		std::size_t PairStopHasher::operator()(const std::pair<const  Stop*, const  Stop*>& pair_stop) const
		{
			constexpr std::size_t hash_multiplier = 42;

			// Вычисляем хеш для каждого указателя на Stop
			std::size_t hash1 = 0;
			std::size_t hash2 = 0;

			for (char c : pair_stop.first->name)
			{
				hash1 = hash1 * hash_multiplier + c;
			}

			for (char c : pair_stop.second->name)
			{
				hash2 = hash2 * hash_multiplier + c;
			}

			// Комбинируем хеши указателей в одно хеш-значение
			return hash1 * hash_multiplier + hash2;
		}
	}//namespace detail

	void TransportCatalogue::AddStop(Stop stop)
	{
		stops_.push_back(std::move(stop));
		Stop* stop_ptr = &stops_.back();
		stopname_to_stop_.emplace(stop_ptr->name, stop_ptr);
	}

	Stop* TransportCatalogue::FindStop(std::string_view stop) const
	{
		if (stopname_to_stop_.count(stop) == 0)
		{
			return nullptr;
		}
		return stopname_to_stop_.at(stop);
	}

	std::set<std::string> TransportCatalogue::GetStopBuses(std::string_view stop) const
	{
		if (stopname_to_busnames_.count(stop) == 0)
		{
			return {};
		}
		return stopname_to_busnames_.at(stop);
	}

	void TransportCatalogue::SetDistance(const std::string& stop, std::vector<std::pair<std::string, int>>& distances_to_stops)
	{
		if (distances_to_stops.size() == 0)
		{
			return;
		}
		Stop* stop_ptr = stopname_to_stop_.at(stop);
		for (const auto& [stopname, distance] : distances_to_stops)
		{
			Stop* another_stop_ptr = stopname_to_stop_.at(stopname);
			stops_distances_.emplace(std::make_pair(stop_ptr, another_stop_ptr), distance);
		}
	}

	int TransportCatalogue::GetDistance(const Stop* stop_ptr, const Stop* anoter_stop_ptr) const
	{
		auto first_it = stops_distances_.find(std::make_pair(stop_ptr, anoter_stop_ptr));
		auto second_it = stops_distances_.find(std::make_pair(anoter_stop_ptr, stop_ptr));

		if (first_it != stops_distances_.end())
		{
			return stops_distances_.at(std::make_pair(stop_ptr, anoter_stop_ptr));
		}
		else if (second_it != stops_distances_.end())
		{
			return stops_distances_.at(std::make_pair(anoter_stop_ptr, stop_ptr));
		}
		else
		{
			return 0;
		}
	}

	void TransportCatalogue::AddBus(const std::tuple<std::string, bool, std::vector<std::string>>& bus_input)
	{
		Bus bus_add;
		std::vector<Stop*> stops_view;

		bus_add.name = std::get<0>(bus_input);
		bus_add.is_roundtrip = std::get<1>(bus_input);
        for (const std::string& stop_name : std::get<2>(bus_input))
        {
            stops_view.push_back(FindStop(stop_name));
        }
		bus_add.stops = stops_view;
		buses_.push_back(bus_add);
		Bus* bus_ptr = &buses_.back();
		busname_to_bus_.emplace(bus_ptr->name, bus_ptr);
		for (Stop* stop : stops_view)
		{
			stopname_to_busnames_[stop->name].insert(bus_ptr->name);
		}
	}

	const Bus* TransportCatalogue::FindBus(std::string_view bus) const
	{
		if (busname_to_bus_.count(bus) == 0)
		{
			return nullptr;
		}
		return busname_to_bus_.at(bus);
	}

	std::optional <BusInfo> TransportCatalogue::GetBusInfo(std::string_view bus_name) const
	{
		BusInfo bus_info;
		double compute_length = 0;
		int get_distance_length = 0;
		const Bus* bus = FindBus(bus_name);

		if (bus == nullptr)
		{
			return {};
		}
		bus_info.name = bus_name;
		std::vector<Stop*> stops_view = bus->stops;
		if (stops_view.size() == 0)
		{
			return bus_info;
		}
		for (size_t i = 0; i < stops_view.size() - 1; ++i)
		{
			const Stop* current_stop = stops_view[i];
			const Stop* next_stop = stops_view[i + 1];
			compute_length += ComputeDistance(current_stop->coordinates, next_stop->coordinates);
			get_distance_length += GetDistance(current_stop, next_stop);
		}

		if (bus->is_roundtrip)
		{
			bus_info.amount_stops = stops_view.size();
		}
		else
		{
			bus_info.amount_stops = stops_view.size() * 2 - 1;
			compute_length += compute_length;
			for (auto it = stops_view.rbegin(); it != stops_view.rend(); ++it)
			{
				if (it != stops_view.rbegin())
				{
					const Stop* r_current_stop = *it;
					const Stop* r_next_stop = *(it - 1);
					get_distance_length += GetDistance(r_next_stop, r_current_stop);
				}
			}
		}
		std::unordered_set<Stop*> uniq_stops(stops_view.begin(), stops_view.end());

		bus_info.uniq_stops = uniq_stops.size();
		bus_info.route_length = get_distance_length;
		bus_info.curvature = get_distance_length / compute_length;
		return bus_info;
	}
    
    const std::unordered_map<std::string_view, Bus*> &TransportCatalogue::GetBusnameToBus() const
    {
        return busname_to_bus_;
    }

    const std::unordered_map<std::string_view, Stop*> &TransportCatalogue::GetStopnameToStop() const 
    {
        return stopname_to_stop_;
    }
    
    const std::unordered_map<std::string_view, std::set<std::string>>&TransportCatalogue::GetStopnameToBusnames() const
    {
         return stopname_to_busnames_;
    }
}//namespace transport_catalogue