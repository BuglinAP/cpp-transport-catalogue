#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include <set>
#include <deque>
#include <string>
#include <string_view>
#include <fstream>

namespace stat_reader
{
	struct RequestData
	{
		std::string type;
		std::string query;
	};

	class StatReader
	{
	public:
		StatReader(transport_catalogue::TransportCatalogue& transport_catalogue);

		void OutputRequests(std::istream& input_stream);

	private:
		transport_catalogue::TransportCatalogue& transport_catalogue_;
		std::vector<RequestData> requests_queue_;
	};
}//namespace stat_reader