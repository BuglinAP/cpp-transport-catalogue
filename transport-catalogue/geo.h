#pragma once

#include <cmath>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

namespace geo
{
    constexpr int EARTH_RADIUS = 6371000;

    struct Coordinates
    {
        double lat;
        double lng;
        bool operator==(const Coordinates& other) const
        {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const
        {
            return !(*this == other);
        }
    };

    inline double ComputeDistance(Coordinates from, Coordinates to)
    {
        using namespace std;
        if (from == to)
        {
            return 0;
        }
        const double dr = M_PI / 180.;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
            + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) * EARTH_RADIUS;
    }
}//namespace geo