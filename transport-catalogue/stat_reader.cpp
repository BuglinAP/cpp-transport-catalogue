#include "stat_reader.h"

using namespace std::literals;

namespace stat_reader
{
	StatReader::StatReader(transport_catalogue::TransportCatalogue& transport_catalogue)
		: transport_catalogue_(transport_catalogue)
	{
	}

	void StatReader::OutputRequests(std::istream& input_stream)
	{
		int num_request;
		input_stream >> num_request;
		input_stream.ignore();

		for (int i = 0; i < num_request; ++i)
		{
			RequestData request;
			std::string input;
			std::getline(input_stream, input);
			auto pos = input.find(' ');

			request.type = input.substr(0, pos);
			request.query = input.substr(pos + 1);
			requests_queue_.push_back(request);
		}

		for (const auto& request : requests_queue_)
		{
			if (request.type == "Bus"s)
			{
				transport_catalogue::BusInfo bus_info = transport_catalogue_.GetBusInfo(request.query);
				if (bus_info.amount_stops == 0)
				{
					std::cout << "Bus " << bus_info.name << ": "s << "not found" << std::endl;
				}
				else
				{
					std::cout << "Bus "
						<< bus_info.name << ": "s
						<< bus_info.amount_stops << " stops on route, "s
						<< bus_info.uniq_stops << " unique stops, "s
						<< bus_info.route_length << " route length, "s
						<< bus_info.curvature << " curvature"s << std::endl;
				}
			}

			if (request.type == "Stop"s)
			{
				const transport_catalogue::Stop* myStop = transport_catalogue_.FindStop(request.query);
				if (myStop == nullptr)
				{
					std::cout << "Stop " << request.query << ": not found" << std::endl;
				}
				else
				{
					std::set<std::string_view> stop_info = transport_catalogue_.GetStopInfo(request.query);
					if (stop_info.empty())
					{
						std::cout << "Stop " << request.query << ": no buses" << std::endl;
					}
					else
					{
						std::cout << "Stop " << request.query << ": "s << "buses ";
						for (const std::string_view busname : stop_info)
						{
							std::cout << std::string(busname) << " ";
						}
						std::cout << std::endl;
					}
				}
			}
		}
	}
}//namespace stat_reader