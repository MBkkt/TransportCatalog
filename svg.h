#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Svg {

struct Point {
    double x = 0;
    double y = 0;
};

struct Rgb {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct Rgba : Rgb {
    double opacity;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
const Color NoneColor{};

class Object {
 public:
    virtual std::unique_ptr<Object> Copy() const = 0;

    virtual void Render(std::ostream &out) const = 0;

    virtual ~Object() = default;
};

template<typename Owner>
class CopyableObject : public Object {
 public:
    std::unique_ptr<Object> Copy() const override;

    virtual ~CopyableObject() = default;
};

template<typename Owner>
class PathProps {
 public:
    Owner &SetFillColor(const Color &color);

    Owner &SetStrokeColor(const Color &color);

    Owner &SetStrokeWidth(double value);

    Owner &SetStrokeLineCap(const std::string &value);

    Owner &SetStrokeLineJoin(const std::string &value);

    void RenderAttrs(std::ostream &out) const;

 protected:
    Color fill_color_;
    Color stroke_color_;
    double stroke_width_ = 1.0;
    std::optional<std::string> stroke_line_cap_;
    std::optional<std::string> stroke_line_join_;

 private:
    Owner &AsOwner();
};

class Circle : public CopyableObject<Circle>, public PathProps<Circle> {
 public:
    Circle &SetCenter(Point point);

    Circle &SetRadius(double radius);

    void Render(std::ostream &out) const override;

 private:
    Point center_;
    double radius_ = 1;
};

class Polyline : public CopyableObject<Polyline>, public PathProps<Polyline> {
 public:
    Polyline &AddPoint(Point point);

    void Render(std::ostream &out) const override;

 private:
    std::vector<Point> points_;
};

class Rectangle : public CopyableObject<Rectangle>, public PathProps<Rectangle> {
 public:
    Rectangle &SetTopLeftPoint(Point point);

    Rectangle &SetBottomRightPoint(Point point);

    void Render(std::ostream &out) const override;

 private:
    Point top_left_point_;
    Point bottom_right_point_;
};

class Text : public CopyableObject<Text>, public PathProps<Text> {
 public:
    Text &SetPoint(Point point);

    Text &SetOffset(Point point);

    Text &SetFontSize(uint32_t size);

    Text &SetFontFamily(const std::string &value);

    Text &SetFontWeight(const std::string &value);

    Text &SetData(const std::string &data);

    void Render(std::ostream &out) const override;

 private:
    Point point_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::optional<std::string> font_family_;
    std::optional<std::string> font_weight_;
    std::string data_;
};

class Document : public CopyableObject<Document> {
 public:
    Document() = default;

    Document(const Document &other);

    Document &operator=(const Document &other);

    Document(Document &&) = default;

    Document &operator=(Document &&) = default;

    template<typename ObjectType>
    void Add(ObjectType object);

    void Render(std::ostream &out) const override;

 private:
    std::vector<std::unique_ptr<Object>> objects_;
};

template<typename Owner>
std::unique_ptr<Object> CopyableObject<Owner>::Copy() const {
    return std::make_unique<Owner>(static_cast<const Owner &>(*this));
}

template<typename Owner>
Owner &PathProps<Owner>::AsOwner() {
    return static_cast<Owner &>(*this);
}

template<typename Owner>
Owner &PathProps<Owner>::SetFillColor(const Color &color) {
    fill_color_ = color;
    return AsOwner();
}

template<typename Owner>
Owner &PathProps<Owner>::SetStrokeColor(const Color &color) {
    stroke_color_ = color;
    return AsOwner();
}

template<typename Owner>
Owner &PathProps<Owner>::SetStrokeWidth(double value) {
    stroke_width_ = value;
    return AsOwner();
}

template<typename Owner>
Owner &PathProps<Owner>::SetStrokeLineCap(const std::string &value) {
    stroke_line_cap_ = value;
    return AsOwner();
}

template<typename Owner>
Owner &PathProps<Owner>::SetStrokeLineJoin(const std::string &value) {
    stroke_line_join_ = value;
    return AsOwner();
}

template<typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream &out) const {
    out << "fill=\"";
    RenderColor(out, fill_color_);
    out << "\" ";
    out << "stroke=\"";
    RenderColor(out, stroke_color_);
    out << "\" ";
    out << "stroke-width=\"" << stroke_width_ << "\" ";
    if (stroke_line_cap_) {
        out << "stroke-linecap=\"" << *stroke_line_cap_ << "\" ";
    }
    if (stroke_line_join_) {
        out << "stroke-linejoin=\"" << *stroke_line_join_ << "\" ";
    }
}

template<typename ObjectType>
void Document::Add(ObjectType object) {
    objects_.push_back(std::make_unique<ObjectType>(std::move(object)));
}

}
