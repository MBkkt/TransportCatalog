#include "sphere_projection.h"


using namespace std;

namespace Sphere {
GeoProjector::GeoProjector(Point start, Point end, int64_t diff) {
    if (diff < 2) {
        return;
    }
    lon = start.longitude;
    lat = start.latitude;
    lon_step_ = (end.longitude - start.longitude) / diff;
    lat_step_ = (end.latitude - start.latitude) / diff;
}

void GeoProjector::operator()(Point &point) const {
    point.longitude = lon + count_ * lon_step_;
    point.latitude = lat + count_++ * lat_step_;
}

Svg::Point Projector::operator()(Point point) const {
    auto idx = lon_points_.at(point.longitude);
    auto idy = lat_points_.at(point.latitude);
    return {
        idx * x_step + padding_,
        max_height_ - padding_ - idy * y_step
    };
}


}
