#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"
#include "geo.h"
#include <iostream>

int main()
{
	transport_catalogue::TransportCatalogue tc;
	transport_catalogue::JsonReader jr(tc, std::cin);
	jr.ReadRequests();
	return 0;
}