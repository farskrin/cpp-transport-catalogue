#pragma once

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <cassert>
#include <sstream>

using namespace std::literals;

namespace json {

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        Node(std::nullptr_t) : value_(nullptr) {}
        Node(Array array) : value_(array) {}
        Node(Dict map) : value_(map) {}
        Node(bool value) : value_(value) {}
        Node(int value) : value_(value) {}
        Node(double value) : value_(value) {}
        Node(std::string value) : value_(value) {}

        bool IsInt() const { return std::holds_alternative<int>(value_); }
        bool IsDouble() const { return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_); }
        bool IsPureDouble() const { return std::holds_alternative<double>(value_); }
        bool IsBool() const { return std::holds_alternative<bool>(value_); }
        bool IsString() const { return std::holds_alternative<std::string>(value_); }
        bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
        bool IsArray() const { return std::holds_alternative<Array>(value_); }
        bool IsMap() const { return std::holds_alternative<Dict>(value_); }

        int AsInt() const {
            if (IsInt()) {
                return std::get<int>(value_);
            }
            throw std::logic_error("AsInt");
        }
        bool AsBool() const {
            if (IsBool()) {
                return std::get<bool>(value_);
            }
            throw std::logic_error("AsBool");
        }
        double AsDouble() const {
            if (IsInt()) {
                return static_cast<double>(std::get<int>(value_));
            }
            if (IsDouble()) {
                return std::get<double>(value_);
            }
            throw std::logic_error("AsDouble");
        }
        const std::string& AsString() const {
            if (IsString()) {
                return std::get<std::string>(value_);
            }
            throw std::logic_error("AsString");
        }
        const Array& AsArray() const {
            if (IsArray()) {
                return std::get<Array>(value_);
            }
            throw std::logic_error("AsArray");
        }
        const Dict& AsMap() const {
            if (IsMap()) {
                return std::get<Dict>(value_);
            }
            throw std::logic_error("AsMap");
        }

        const Value& GetValue() const { return value_; }

        bool operator==(const Node& rhs) const {
            return value_ == rhs.value_;
        }
        bool operator!=(const Node& rhs) const {
            return value_ != rhs.value_;
        }

    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;
        bool operator==(const Document& rhs) const;

    private:
        Node root_;
    };

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);


    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }
    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, const PrintContext& ctx);

    void PrintNode(const Node& node, const PrintContext& ctx);

    void PrintValue(const Array& array, const PrintContext& ctx);

    //std::ostream& operator<<(std::ostream& out, std::pair<std::string, Node> value);

    void PrintValue(const Dict& dict, const PrintContext& ctx);

    void PrintString(std::string_view sv, std::ostream& out);

    void PrintValue(std::string str, const PrintContext& ctx);

    void PrintValue(const bool& b, const PrintContext& ctx);

}  // namespace json