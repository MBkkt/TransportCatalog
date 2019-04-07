#ifndef __SPHERE_H__
#define __SPHERE_H__
#pragma once

#include <cmath>

namespace Sphere {

    double convertDegreesToRadians(double degrees);

    struct Point {
        double latitude;
        double longitude;

        static Point fromDegrees(double latitude, double longitude);
    };

    double distance(Point lhs, Point rhs);
}

#endif /* __SPHERE_H__ */
