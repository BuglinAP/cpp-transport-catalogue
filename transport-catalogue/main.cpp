#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"
#include "geo.h"
#include <iostream>

int main()
{
	transport_catalogue::TransportCatalogue tc;
	input_reader::InputReader ir(tc);
	ir.ReadRequests(std::cin);
	stat_reader::StatReader sr(tc);
	sr.OutputRequests(std::cin);
	return 0;
}