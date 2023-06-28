#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"

#include "svg.h"
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <sstream>

namespace transport_catalogue
{
    class JsonReader final
    {
    public:
        // при создании считывает данные из входного потока
        explicit JsonReader(TransportCatalogue& transport_catalogue, std::istream& input_stream);

        void ReadRequests(); //интерйфейс для отправки запросов к каталогу

    private:
        TransportCatalogue& transport_catalogue_;
        std::vector<Stop> request_stops_;
        std::vector<std::vector<std::pair<std::string, int>>> distances_to_stops_;
        std::vector<std::tuple<std::string, bool, std::vector<std::string>>> request_buses_;
        json::Document data_document_;

        void ReadBaseRequests();                        //  загрузка base requests в очередь запросов
        void ReadStop(const json::Dict& request_stop); // 
        void ReadBus(const json::Dict& request_stop); //

        void LoadBaseRequestsToCatalog(); // загрузка данных из очереди запросов в каталог

        void GenerateOutput(); // формирует и возвращает ответы на запросы
        void OutputBusInfo(const json::Node& request, json::Array& result) const; // ответ на запрос инфромации о маршруте
        void OutputStopInfo(const json::Node& request, json::Array& result) const; // ответ на запрос инфромации об остановке   
        void RenderMap(const json::Node& request, json::Array& result) const; // ответ на запрос построения карты маршрутов
        void OutputRouteInfo(const json::Node& request, json::Array& result, TransportRouter& router) const;

        RenderSettings LoadRenderSettings() const;

        RoutingSettings LoadRoutingSettings() const;

    };

    namespace detail_load
    {
        RenderSettings Settings(const json::Dict& data); // формирует настройки рендеринга

        svg::Point Offset(const json::Array& offset); // считывает пару значений (offset) из ноды

        svg::Color Color(const json::Node& node); // считывает значение цвета из ноды
    }//namespace load_detail
}//namespace transport_catalogue