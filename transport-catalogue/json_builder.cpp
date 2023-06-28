#include "json_builder.h"

namespace json
{
	using namespace std::literals;

	const Node& Builder::Build() const
	{
		if (is_empty_ || !nodes_stack_.empty())
		{
			throw std::logic_error("Builder state is invalid"s);
		}
		return root_;
	}

	Builder::KeyValueContext Builder::Key(std::string key)
	{
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && !key_)
		{
			key_ = std::move(key);
			return KeyValueContext(*this);
		}
		throw std::logic_error("Incorrect place for key : "s + key);
	}

	Builder& Builder::Value(Node::Value value)
	{
		Node new_node = std::visit([](auto val) { return Node(val); }, value);

		if (is_empty_)
		{
			root_ = new_node;
			is_empty_ = false;
			return *this;
		}
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && key_)
		{
			const_cast<Dict&>(nodes_stack_.back()->AsDict()).insert({ *key_, new_node });
			key_.reset();
			return *this;
		}
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray())
		{
			const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(new_node);
			return *this;
		}
		throw std::logic_error("Incorrect place for value"s);
	}

	Builder::DictItemContext Builder::StartDict()
	{
		Value(Dict{});
		AddRef();
		return DictItemContext(*this);
	}

	Builder& Builder::EndDict()
	{
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && !key_)
		{
			nodes_stack_.pop_back();
			return *this;
		}
		throw std::logic_error("Incorrect place for EndDict"s);
	}

	Builder::ArrayItemContext Builder::StartArray()
	{
		Value(Array{});
		AddRef();
		return ArrayItemContext(*this);
	}

	Builder& Builder::EndArray()
	{
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray())
		{
			nodes_stack_.pop_back();
			return *this;
		}
		throw std::logic_error("Incorrect place for EndArray"s);
	}

	void Builder::AddRef()
	{
		if (nodes_stack_.empty())
		{
			nodes_stack_.push_back(&root_);
			return;
		}
		if (nodes_stack_.back()->IsArray())
		{
			auto p = &nodes_stack_.back()->AsArray().back();
			nodes_stack_.push_back(const_cast<Node*>(p));
			return;
		}
		if (nodes_stack_.back()->IsDict())
		{
			auto p = &nodes_stack_.back()->AsDict().at(*key_);
			nodes_stack_.push_back(const_cast<Node*>(p));
			return;
		}
	}

	// ---- Вспомогательные классы для проверки корректности времени компиляции ----

	Builder::DictItemContext::DictItemContext(Builder& builder)
		: builder_(builder)
	{
	}

	Builder::KeyValueContext Builder::DictItemContext::Key(std::string key)
	{
		return builder_.Key(std::move(key));
	}

	Builder& Builder::DictItemContext::EndDict()
	{
		return builder_.EndDict();
	}

	Builder::ArrayItemContext::ArrayItemContext(Builder& builder) : builder_(builder) {}

	Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value)
	{
		builder_.Value(std::move(value));
		return ArrayItemContext{ builder_ };
	}

	Builder::DictItemContext Builder::ArrayItemContext::StartDict()
	{
		return builder_.StartDict();
	}

	Builder::ArrayItemContext Builder::ArrayItemContext::StartArray()
	{
		return builder_.StartArray();
	}

	Builder& Builder::ArrayItemContext::EndArray()
	{
		return builder_.EndArray();
	}

	Builder::KeyValueContext::KeyValueContext(Builder& builder) : builder_(builder) {}

	Builder::DictItemContext Builder::KeyValueContext::Value(Node::Value value)
	{
		builder_.Value(std::move(value));
		return DictItemContext{ builder_ };
	}

	Builder::DictItemContext Builder::KeyValueContext::StartDict()
	{
		return builder_.StartDict();
	}

	Builder::ArrayItemContext Builder::KeyValueContext::StartArray()
	{
		return builder_.StartArray();
	}
} // namespace json