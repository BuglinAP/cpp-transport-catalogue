#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <sstream>

namespace input_reader
{
    class InputReader
    {
    public:
        InputReader(transport_catalogue::TransportCatalogue& transport_catalogue);

        void ReadRequests(std::istream& input_stream);

    private:
        transport_catalogue::TransportCatalogue& transport_catalogue_;
        std::vector<transport_catalogue::Stop> request_stops_;
        std::vector<std::vector<std::pair<std::string, int>>> distances_to_stops_;
        std::vector<std::pair<transport_catalogue::Bus, std::vector<std::string>>> request_buses_;

        void ReadStop(std::string& request_stop);

        void ReadBus(std::string& request_bus);

        std::vector<std::string> SplitBusStops(const std::string& str, const std::string& delimiter);
    };
}//namespace input_reader