#pragma GCC optimize("Ofast")

#include "svg.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <variant>

using namespace std;

namespace Svg {

/**********************************************************************************************************************/
namespace {

void RenderColor(ostream &out, monostate) {
    out << "none";
}

void RenderColor(ostream &out, const string &value) {
    out << value;
}

void RenderColor(ostream &out, Rgba rgba) {
    if (rgba.alpha) {
        out << "rgba";
    } else {
        out << "rgb";
    }
    out << "(" << static_cast<uint32_t>(rgba.red)
        << "," << static_cast<uint32_t>(rgba.green)
        << "," << static_cast<uint32_t>(rgba.blue);
    if (rgba.alpha) {
        out << "," << *rgba.alpha;
    }
    out << ")";
}
}

ostream &operator<<(ostream &out, const Color &color) {
    visit([&out](const auto &value) { RenderColor(out, value); },
          color);
    return out;
}

bool operator<(const Point &, const Point &) {
    return false;
}

bool operator<(const Polyline &, const Polyline &) {
    return false;
}

/**********************************************************************************************************************/
Circle &Circle::SetCenter(Point point) {
    center_ = point;
    return *this;
}

Circle &Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::Render(ostream &out) const {
    out << "<circle "
        << "cx=\"" << center_.x << "\" "
        << "cy=\"" << center_.y << "\" "
        << "r=\"" << radius_ << "\" ";
    PathProps::RenderAttrs(out);
    out << "/>";
}

/**********************************************************************************************************************/
Polyline &Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::Render(ostream &out) const {
    out << "<polyline points=\"";
    BeautyPrintHelper bph{" "};
    for (auto &point : points_) {
        out << bph << point.x << "," << point.y;
    }
    out << "\" ";
    PathProps::RenderAttrs(out);
    out << "/>";
}

/**********************************************************************************************************************/
Text &Text::SetPoint(Point point) {
    point_ = point;
    return *this;
}

Text &Text::SetOffset(Point point) {
    offset_ = point;
    return *this;
}

Text &Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text &Text::SetFontFamily(const string &value) {
    font_family_ = value;
    return *this;
}

Text &Text::SetData(const string &data) {
    data_ = data;
    return *this;
}

void Text::Render(ostream &out) const {
    out << "<text "
        << "x=\"" << point_.x << "\" "
        << "y=\"" << point_.y << "\" "
        << "dx=\"" << offset_.x << "\" "
        << "dy=\"" << offset_.y << "\" "
        << "font-size=\"" << font_size_ << "\" ";
    if (font_family_) {
        out << "font-family=\"" << *font_family_ << "\" ";
    }
    PathProps::RenderAttrs(out);
    out << ">" << data_ << "</text>";
}

/**********************************************************************************************************************/
void Document::Render(ostream &out) const {
    out << R"(<?xml version="1.0" encoding="UTF-8" ?><svg xmlns="http://www.w3.org/2000/svg" version="1.1">)";
    for (auto &object_ptr : objects_) {
        object_ptr->Render(out);
    }
    out << "</svg>";
}

/**********************************************************************************************************************/

}
