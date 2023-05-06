#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"
#include "geo.h"
#include <iostream>

int main()
{
	transport_catalogue::TransportCatalogue tc;
	transport_catalogue::InputReader ir(tc);
	ir.ReadRequests(std::cin);
	transport_catalogue::StatReader sr(tc);
	sr.OutputRequests(std::cin);
	system("pause");
	return 0;
}