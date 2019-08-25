#include "svg_serialize.h"

using namespace std;

namespace Svg {

void SerializePoint(Point point, SvgProto::Point &proto) {
    proto.set_x(point.x);
    proto.set_y(point.y);
}

Point DeserializePoint(const SvgProto::Point &proto) {
    return {proto.x(), proto.y()};
}

void SerializeColor(const Color &color, SvgProto::Color &proto) {
    if (holds_alternative<monostate>(color)) {
        proto.set_is_none(true);
    } else if (holds_alternative<string>(color)) {
        const auto &name = get<string>(color);
        proto.set_name(name);
    } else {
        const bool has_opacity = holds_alternative<Rgba>(color);
        const Rgb &rgb = has_opacity ? get<Rgba>(color) : get<Rgb>(color);
        auto &rgba_proto = *proto.mutable_rgba();
        rgba_proto.set_red(rgb.red);
        rgba_proto.set_green(rgb.green);
        rgba_proto.set_blue(rgb.blue);
        if (has_opacity) {
            rgba_proto.set_has_opacity(true);
            rgba_proto.set_opacity(get<Rgba>(color).opacity);
        }
    }
}

Color DeserializeColor(const SvgProto::Color &proto) {
    if (proto.is_none()) {
        return monostate{};
    }

    if (!proto.has_rgba()) {
        return proto.name();
    }

    const auto &rgba_proto = proto.rgba();
    const auto red = static_cast<uint8_t>(rgba_proto.red());
    const auto green = static_cast<uint8_t>(rgba_proto.green());
    const auto blue = static_cast<uint8_t>(rgba_proto.blue());
    const Rgb rgb{red, green, blue};
    if (rgba_proto.has_opacity()) {
        return Rgba{rgb, rgba_proto.opacity()};
    } else {
        return rgb;
    }
}

}
