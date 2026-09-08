#include "geo.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double grados_a_radianes(double grados) {
    return grados * (M_PI / 180.0);
}

double geo_haversine_metros(double lat1, double lon1, double lat2, double lon2) {
    const double radio = 6371000.0;
    double phi1 = grados_a_radianes(lat1);
    double phi2 = grados_a_radianes(lat2);
    double dphi = grados_a_radianes(lat2 - lat1);
    double dlmb = grados_a_radianes(lon2 - lon1);
    double a = sin(dphi / 2.0) * sin(dphi / 2.0) +
               cos(phi1) * cos(phi2) * sin(dlmb / 2.0) * sin(dlmb / 2.0);
    if (a > 1.0) {
        a = 1.0;
    }
    return 2.0 * radio * asin(sqrt(a));
}
