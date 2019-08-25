#pragma once

#include "svg.h"
#include "svg.pb.h"

namespace Svg {

void SerializePoint(Point point, SvgProto::Point &proto);

Point DeserializePoint(const SvgProto::Point &proto);

void SerializeColor(const Color &color, SvgProto::Color &proto);

Color DeserializeColor(const SvgProto::Color &proto);

}
