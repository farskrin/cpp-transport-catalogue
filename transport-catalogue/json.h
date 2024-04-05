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

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    class Node : public Value {
    public:
        Node() = default;

        Node(const Value& rhs)
            : Value(rhs) {            
        }
        Node(Value&& rhs) noexcept
            : Value(std::move(rhs)) {
        }
        Node& operator=(const Value& rhs) {
            if (*this != Node(rhs)) {
                *this = rhs;
            }
            return *this;
        }
        Node& operator=(Value&& rhs) noexcept {
            std::swap(*this, rhs);
            return *this;
        }

        bool IsInt() const { return std::holds_alternative<int>(*this); }
        bool IsDouble() const { return std::holds_alternative<int>(*this) || std::holds_alternative<double>(*this); }
        bool IsPureDouble() const { return std::holds_alternative<double>(*this); }
        bool IsBool() const { return std::holds_alternative<bool>(*this); }
        bool IsString() const { return std::holds_alternative<std::string>(*this); }
        bool IsNull() const { return std::holds_alternative<std::nullptr_t>(*this); }
        bool IsArray() const { return std::holds_alternative<Array>(*this); }
        bool IsMap() const { return std::holds_alternative<Dict>(*this); }

        int AsInt() const {
            if (IsInt()) {
                return std::get<int>(*this);
            }
            throw std::logic_error("AsInt");
        }
        bool AsBool() const {
            if (IsBool()) {
                return std::get<bool>(*this);
            }
            throw std::logic_error("AsBool");
        }
        double AsDouble() const {
            if (IsInt()) {
                return static_cast<double>(std::get<int>(*this));
            }
            if (IsDouble()) {
                return std::get<double>(*this);
            }
            throw std::logic_error("AsDouble");
        }
        const std::string& AsString() const {
            if (IsString()) {
                return std::get<std::string>(*this);
            }
            throw std::logic_error("AsString");
        }
        const Array& AsArray() const {
            if (IsArray()) {
                return std::get<Array>(*this);
            }
            throw std::logic_error("AsArray");
        }
        const Dict& AsMap() const {
            if (IsMap()) {
                return std::get<Dict>(*this);
            }
            throw std::logic_error("AsMap");
        }

        const Value& GetValue() const { return *this; }

        bool operator==(const Node& rhs) const {
            return this == &rhs;
        }
        bool operator!=(const Node& rhs) const {
            return this != &rhs;
        }

    private:

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