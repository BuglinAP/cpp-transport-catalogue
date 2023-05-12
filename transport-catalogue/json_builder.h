#pragma once

#include <string>
#include <variant>
#include <vector>

#include "json.h"

namespace json 
{

    class Builder final 
    {
        class DictItemContext;
        class ArrayItemContext;
        class KeyValueContext;

    public:
        Builder() = default;

        // выбрасывает исключение std::logic_error если на момент вызова объект некорректен
        const Node& Build() const;

        KeyValueContext Key(std::string key);
        Builder& Value(Node::Value value);

        DictItemContext StartDict();
        Builder& EndDict();

        ArrayItemContext StartArray();
        Builder& EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        // состояние по умолчанию при создании словаря
        bool is_empty_ = true;
        // наличие введенного ключа
        bool has_key_ = false;
        std::string key_;
        // добавляет указатель на новый узел в nodes_stack_ в зависимости от типа value
        void AddRef(const Node& value);
    };

    // ---- Вспомогательные классы для проверки корректности времени компиляции ----

    class Builder::DictItemContext final
    {
    public:
        DictItemContext(Builder& builder);
        KeyValueContext Key(std::string key);
        Builder& EndDict();
    private:
        Builder& builder_;
    };

    class Builder::ArrayItemContext final
    {
    public:
        ArrayItemContext(Builder& builder);
        ArrayItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
    private:
        Builder& builder_;
    };

    class Builder::KeyValueContext final
    {
    public:
        KeyValueContext(Builder& builder);
        DictItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    private:
        Builder& builder_;
    };
} // namespace json