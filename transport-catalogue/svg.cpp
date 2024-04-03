#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    //void Circle::RenderObject(const RenderContext& context) const {
    //    auto& out = context.out;
    //    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    //    out << "r=\""sv << radius_ << "\" "sv;
    //    out << "/>"sv;
    //}

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, Color color)
    {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    bool operator==(const Point& lhs, const Point& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    void ColorPrinter::operator()(std::monostate) const {
        out << NoneColor;
    }

    void ColorPrinter::operator()(std::string s) const {
        out << s;
    }

    void ColorPrinter::operator()(Rgb rgb) const {
        const std::string r = std::to_string(rgb.red);
        const std::string g = std::to_string(rgb.green);
        const std::string b = std::to_string(rgb.blue);
        std::string color = "rgb("s + r + ","s + g + ","s + b + ")"s;
        out << color;
    }

    void ColorPrinter::operator()(Rgba rgba) const {
        out << "rgba("s << static_cast<int>(rgba.red) << ","s << static_cast<int>(rgba.green)
            << ","s << static_cast<int>(rgba.blue) << ","s << rgba.opacity << ")"s;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        //<polyline points = "20,40 22.9389,45.9549 29.5106,46.9098 24.7553,
        //51.5451 25.8779,58.0902 20,55 14.1221,58.0902 15.2447,51.5451 10.4894,
        //46.9098 17.0611,45.9549 20,40" / >
        auto& out = context.out;
        out << "<polyline points=\"";
        bool first = true;
        for (const auto& point : points_) {
            if (!first) {
                out << " ";
            }
            first = false;
            out << point.x << ',' << point.y;
        }
        out << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    std::string Text::ToEscapeCharacters(const std::string& str) const {
        std::string result;
        for (const auto& c : str) {
            if (simbols_.count(c)) {
                result += simbols_.at(c);
            }
            else
            {
                result += c;
            }
        }
        return result;
    }

    void Text::RenderObject(const RenderContext& context) const {
        //<text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">Hello C++</text>
        auto& out = context.out;
        out << "<text";
        RenderAttrs(context.out);
        out << " x=\"" << pos_.x << "\" y=\"" << pos_.y
            << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y
            << "\" font-size=\"" << size_ << "\"";
        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        out.put('>');
        out << ToEscapeCharacters(data_);
        out << "</text>"sv;
    }


    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        RenderContext ctx(out, 2, 2);
        for (const auto& obj : objects_) {
            obj->Render(ctx);
        }

        out << "</svg>"sv;
    }


}  // namespace svg


