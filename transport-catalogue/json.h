#pragma once
#include "geo.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <algorithm>

namespace json
{
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    Node LoadBool(std::istream& input);

    Node LoadArray(std::istream& input);

    Node LoadNumber(std::istream& input);

    Node LoadString(std::istream& input);

    Node LoadNull(std::istream& input);

    Node LoadDict(std::istream& input);

    Node LoadNode(std::istream& input);

    class Node 
    {
    public:

        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        Node(const Array& array);
        Node(Array&& array);
        Node(const Dict& map);
        Node(Dict&& map);
        Node(int value);
        Node(std::string value);
        Node(bool bul);
        Node(double doub);
        Node(std::nullptr_t);

        const Value& GetValue() const { return value_; }

        const Array& AsArray() const;
        const Dict& AsDict() const;
        int AsInt() const;
        const std::string& AsString() const;
        double AsDouble() const;
        bool AsBool() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        bool operator==(const Node& node) const 
        {
            return value_ == node.value_;
        }

        bool operator!=(const Node& node) const
        {
            return value_ != node.value_;
        }

    private:
        Value value_;
    };

    class Document
    {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& rhs) const
        {
            return root_ == rhs.root_;
        }

        bool operator!=(const Document& rhs) const 
        {
            return !(root_ == rhs.root_);
        }

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    inline void PrintValue(int value, std::ostream& out);

    inline void PrintValue(double value, std::ostream& out);

    inline void PrintValue(std::nullptr_t, std::ostream& out);
    
    inline void PrintValue(const std::string& value, std::ostream& out);

    inline void PrintValue(bool bul, std::ostream& out);

    inline void PrintValue(const Array& array, std::ostream& out);

    inline void PrintValue(const Dict& map, std::ostream& out);

    inline void PrintNode(const Node& node, std::ostream& out);

    void Print(const Document& doc, std::ostream& output);

}// namespace json