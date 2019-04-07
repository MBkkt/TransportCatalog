#include "sphere.h"

using namespace std;

namespace Sphere {

    double const PI = 3.1415926535;

    double convertDegreesToRadians(double degrees) {
        return degrees * PI / 180.;
    }

    Point Point::fromDegrees(double latitude, double longitude) {
        return {
            convertDegreesToRadians(latitude),
            convertDegreesToRadians(longitude)
        };
    }

    double const EARTH_RADIUS = 6'371'000;

    double distance(Point lhs, Point rhs) {
        lhs = Point::fromDegrees(lhs.latitude, lhs.longitude);
        rhs = Point::fromDegrees(rhs.latitude, rhs.longitude);
        return acos(
            sin(lhs.latitude) * sin(rhs.latitude)
            + cos(lhs.latitude) * cos(rhs.latitude) * cos(abs(lhs.longitude - rhs.longitude))
        ) * EARTH_RADIUS;
    }
}
