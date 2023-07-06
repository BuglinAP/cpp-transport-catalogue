#include <cassert>
#include <fstream>
#include <iostream>
#include <string_view>

#include "request_handler.h"

using namespace std;

void PrintUsage(std::ostream& stream = std::cerr) 
{
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[])
{
    if (argc != 2) 
    {
        PrintUsage();
        system("pause");
        return 1;
    }

    const std::string_view mode(argv[1]);

    transport_catalogue::TransportCatalogue catalogue;
    transport_catalogue::TransportCatalogueHandler catalogue_handler(catalogue);

    if (mode == "make_base"sv) 
    {
        transport_catalogue::JsonReader json(catalogue, std::cin);
        catalogue_handler.LoadDataFromJson(json);
        catalogue_handler.SerializeData();

    }
    else if (mode == "process_requests"sv) 
    {
        transport_catalogue::JsonReader json(catalogue, std::cin);
        catalogue_handler.LoadSerializeSettings(json);
        catalogue_handler.DeserializeData();
        catalogue_handler.LoadRequestsAndAnswer(json);
    } 
    else
    {
        PrintUsage();
        return 1;
    }
    return 0;
}