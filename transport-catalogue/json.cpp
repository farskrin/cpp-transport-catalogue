#include "json.h"

using namespace std;

namespace json {

    namespace {

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c = 0;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') {
                throw ParsingError("Array parsing error");
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c = 0;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c != '}') {
                throw ParsingError("Dict parsing error");
            }

            return Node(move(result));
        }

        bool LoadBool(istream& input) {
            /*bool b;
            input >> std::boolalpha >> b;
            return b;*/
            char str[6] = { '\0' };
            input.get(str, 5);
            if (std::string(str) == "true"s) {
                if (std::isalpha(input.peek())) {
                    throw ParsingError("Bool parsing error");
                }
                return true;
            }
            str[4] = input.get();
            if (std::string(str) == "false"s) {
                if (std::isalpha(input.peek())) {
                    throw ParsingError("Bool parsing error");
                }
                return false;
            }
            throw ParsingError("Bool parsing error");
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == ']' || c == '}') {
                throw ParsingError("Parsing error");
            }

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return Node(LoadString(input));
            }
            else if (std::isdigit(c) || (c == '-' && std::isdigit(input.peek()))) {        //std::isdigit(input.peek())
                input.putback(c);
                Number number = LoadNumber(input);
                if (std::holds_alternative<int>(number)) {
                    return Node(std::get<int>(number));
                }
                if (std::holds_alternative<double>(number)) {
                    return Node(std::get<double>(number));
                }
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return Node(LoadBool(input));
            }
            else
            {   //null branch
                input.putback(c);
                char str[5] = { '\0' };
                input.get(str, 5);
                if (std::string(str) == "null"s) {
                    if (std::isalpha(input.peek())) {
                        throw ParsingError("Null parsing error");
                    }
                    return Node(nullptr);
                }

                throw ParsingError("Null parsing error");
            }
            return Node();
        }




    }  // namespace

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& rhs) const {
        return root_ == rhs.root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {

        auto node = doc.GetRoot();
        PrintNode(node, PrintContext{ output });
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) { PrintValue(value, ctx); },
            node.GetValue());
    }

    void PrintValue(const Array& array, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        bool first = true;
        out << "[\n"sv;
        auto inner_ctx = ctx.Indented();
        for (const auto& node : array) {
            if (!first) {
                out << ",\n"sv;
            }
            first = false;
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    /*std::ostream& operator<<(std::ostream& out, std::pair<std::string, Node> value) {
        out << "\"";
        out << value.first << "\": ";
        PrintNode(value.second, out);
        return out;
    }*/

    void PrintValue(const Dict& dict, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        bool first = true;
        out << "{\n"sv;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : dict) {
            if (!first) {
                out << ",\n"sv;
            }
            first = false;
            inner_ctx.PrintIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, inner_ctx);            
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }

    void PrintString(std::string_view sv, std::ostream& out) {
        out.put('"');
        for (char c : sv) {
            switch (c) {
            case '\n':
                out << "\\n"sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '\"':
                out << "\\\""sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            case '\\':
                out << "\\\\"sv;
                break;
            default:
                out.put(c);
            }
        }
        out.put('"');
    }

    void PrintValue(std::string str, const PrintContext& ctx) {
        PrintString(str, ctx.out);
    }

    void PrintValue(const bool& b, const PrintContext& ctx) {
        ctx.out << std::boolalpha << b;
    }

}  // namespace json