#include "sphere_projection.h"


using namespace std;

namespace Sphere {

Svg::Point Projector::operator()(Point point) const {
    auto idx = lon_points_.at(point.longitude);
    auto idy = lat_points_.at(point.latitude);
    return {
        idx * x_step + padding_,
        max_height_ - padding_ - idy * y_step
    };
}

}
