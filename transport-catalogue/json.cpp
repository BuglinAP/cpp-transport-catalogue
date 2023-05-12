#include "json.h"

using namespace std::literals;

namespace json 
{

    Node LoadBool(std::istream& input)
    {
        std::string line;
        while (std::isalpha(input.peek()))
        {
            line.push_back(static_cast<char>(input.get()));
        }
        if (line == "true"sv)
        {
            return Node{ true };
        }
        else if (line == "false"sv)
        {
            return Node{ false };
        }
        else
        {
            throw ParsingError("Bool error"s);
        }
    }

    Node LoadArray(std::istream& input)
    {
        Array result;
        for (char c; input >> c && c != ']';)
        {
            if (c != ',')
            {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }
        if (!input)
        {
            throw ParsingError("Array parsing error"s);
        }
        return Node(result);
    }

    Node LoadNumber(std::istream& input)
    {
        using namespace std::literals;

        std::string parsed_num;

        auto read_char = [&parsed_num, &input]
        {
            parsed_num += static_cast<char>(input.get());
            if (!input)
            {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        auto read_digits = [&input, read_char]
        {
            if (!std::isdigit(input.peek()))
            {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek()))
            {
                read_char();
            }
        };

        if (input.peek() == '-')
        {
            read_char();
        }
        if (input.peek() == '0')
        {
            read_char();
        }
        else
        {
            read_digits();
        }

        bool is_int = true;
        if (input.peek() == '.')
        {
            read_char();
            read_digits();
            is_int = false;
        }

        if (int ch = input.peek(); ch == 'e' || ch == 'E')
        {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-')
            {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try
        {
            if (is_int)
            {
                try
                {
                    int temp = std::stoi(parsed_num);
                    return Node(temp);
                }
                catch (...)
                {

                }
            }
            auto temp = std::stod(parsed_num);
            return Node(temp);
        }
        catch (...)
        {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    Node LoadString(std::istream& input)
    {
        using namespace std::literals;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true)
        {
            if (it == end)
            {
                throw ParsingError("String parsing error"s);
            }
            const char ch = *it;
            if (ch == '"')
            {
                ++it;
                break;
            }
            else if (ch == '\\')
            {
                ++it;
                if (it == end)
                {
                    throw ParsingError("String parsing error"s);
                }
                const char escaped_char = *(it);
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
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r')
            {
                throw ParsingError("Unexpected end of line"s);
            }
            else
            {
                s.push_back(ch);
            }
            ++it;
        }
        return Node(s);
    }

    Node LoadNull(std::istream& input)
    {
        std::string line;
        while (std::isalpha(input.peek()))
        {
            line.push_back(static_cast<char>(input.get()));
        }
        if (line != "null")
        {
            throw ParsingError("null error"s);
        }
        return Node(nullptr);
    }

    Node LoadDict(std::istream& input)
    {
        Dict dict;
        for (char current_char; input >> current_char && current_char != '}';)
        {
            if (current_char == '"')
            {
                std::string key = LoadString(input).AsString();

                if (input >> current_char && current_char == ':')
                {
                    if (dict.find(key) != dict.end())
                    {
                        throw ParsingError("Duplicate key '"s + key + "' have been found"s);
                    }
                    dict.emplace(std::move(key), LoadNode(input));
                }
                else
                {
                    throw ParsingError(": is expected but '"s + current_char + "' has been found"s);
                }
            }
            else if (current_char != ',')
            {
                throw ParsingError(R"(',' is expected but ')"s + current_char + "' has been found"s);
            }
        }

        if (!input)
        {
            throw ParsingError("Dictionary parsing error"s);
        }

        return Node(dict);
    }

    Node LoadNode(std::istream& input)
    {
        input >> std::ws;
        char c;
        input >> c;

        if (c == '[')
        {
            return LoadArray(input);
        }
        else if (c == '{')
        {
            return LoadDict(input);
        }
        else if (c == '\"')
        {
            return LoadString(input);
        }
        else if (c == 't' || c == 'f')
        {
            input.putback(c);
            return LoadBool(input);
        }
        if (c == 'n')
        {
            input.putback(c);
            return LoadNull(input);
        }
        else
        {
            input.putback(c);
            return LoadNumber(input);
        }
    }

    const Array& Node::AsArray() const 
    {
        if (!IsArray())
        {
            throw std::logic_error("logic_error Array"s);
        }
        return std::get<Array>(*this);
    }

    const Dict& Node::AsDict() const
    {
        if (!IsMap())
        {
            throw std::logic_error("logic_error Map"s);
        }
        return std::get<Dict>(*this);
    }

    int Node::AsInt() const
    {
        if (!IsInt())
        {
            throw std::logic_error("logic_error"s);
        }
        return std::get<int>(*this);
    }

    const std::string& Node::AsString() const 
    {
        if (!IsString())
        {
            throw std::logic_error("logic_error String"s);
        }
        return std::get<std::string>(*this);
    }

    double Node::AsDouble() const
    {
        if (!IsDouble())
        {
            throw std::logic_error("logic_error"s);
        }
        if (IsPureDouble())
        {
            return std::get<double>(*this);
        }
        return std::get<int>(*this);
    }

    bool Node::AsBool() const
    {
        if (!IsBool())
        {
            throw std::logic_error("logic_error");
        }
        return std::get<bool>(*this);
    }

    bool Node::IsInt() const
    {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const
    {
        return std::holds_alternative<double>(*this) || std::holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const
    {
        return std::holds_alternative<double>(*this);
    }
    bool Node::IsBool() const 
    {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const 
    {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const 
    {
        return std::holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsArray() const 
    {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const
    {
        return std::holds_alternative<Dict>(*this);
    }

    Document::Document(Node root)
        : root_(std::move(root))
    {
    }

    const Node& Document::GetRoot() const
    {
        return root_;
    }

    Document Load(std::istream& input) 
    {
        return Document{ LoadNode(input) };
    }

    inline void PrintValue(int value, std::ostream& out)
    {
        out << value;
    }

    inline void PrintValue(double value, std::ostream& out)
    {
        out << value;
    }

    inline void PrintValue(std::nullptr_t, std::ostream& out)
    {
        out << "null"s;
    }

    inline void PrintValue(const std::string& value, std::ostream& out)
    {
        //  \n, \r, \", \t, \\.
        out << '"';
        for (const char c : value)
        {
            switch (c)
            {
            case '"':
                out << "\\\""sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\t':
                out << "\t"sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '\\':
                out << "\\\\"sv;
                break;
            default:
                out << c;
                break;
            }
        }
        out << '"';
    }

    inline void PrintValue(bool bul, std::ostream& out)
    {
        if (bul)
        {
            out << "true"s;
        }
        else out << "false"s;
    }

    inline void PrintValue(const Array& array, std::ostream& out)
    {
        out << "["s << std::endl;
        for (auto node = array.begin(); node != array.end(); ++node)
        {
            if (node + 1 == array.end())
            {
                PrintNode((*node), out);
                out << std::endl;
            }
            else
            {
                PrintNode((*node), out);
                out << ", " << std::endl;
            }
        }
        out << "]"s;
    }

    inline void PrintValue(const Dict& map, std::ostream& out)
    {
        out << "{"s << std::endl;
        for (auto i = map.begin(); i != map.end(); ++i)
        {
            auto temp = i;
            if (++temp == map.end())
            {
                PrintValue(i->first, out);
                out << ": "s;
                PrintNode(i->second, out);
                out << std::endl;
            }
            else
            {
                PrintValue(i->first, out);
                out << ": "s;
                PrintNode(i->second, out);
                out << ", "s << std::endl;
            }
        }
        out << "}"s;
    }

    inline void PrintNode(const Node& node, std::ostream& out)
    {
        std::visit(
            [&out](const auto& value) { PrintValue(value, out); },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) 
    {
        PrintNode(doc.GetRoot(), output);
    }
}// namespace json