#ifndef __JSON_H__
#define __JSON_H__
#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>


namespace Json {

    class Node;

    using Dict = std::map<std::string, Node>;

    class Node: std::variant<bool, int, double, std::string, std::vector<Node>, Dict> {
    public:
        using variant::variant;

        variant const & getBase() const {
            return *this;
        }

        bool asBool() const {
            return std::get<bool>(*this);
        }

        int asInt() const {
            return std::get<int>(*this);
        }

        double asDouble() const {
            return std::holds_alternative<double>(*this) ? std::get<double>(*this)
                                                         : std::get<int>(*this);
        }

        auto const & asString() const {
            return std::get<std::string>(*this);
        }

        auto const & asArray() const {
            return std::get<std::vector<Node>>(*this);
        }

        auto const & asMap() const {
            return std::get<Dict>(*this);
        }

    };

    class Document {
    public:
        explicit Document(Node root): root(move(root)) {
        }

        Node const & getRoot() const {
            return root;
        }

    private:
        Node root;
    };

    Node loadNode(std::istream & input);

    Document load(std::istream & input);

    void printNode(Node const & node, std::ostream & output);

    template<typename Value>
    void printValue(Value const & value, std::ostream & output) {
        output << value;
    }

    template<>
    void printValue<bool>(bool const & value, std::ostream & output);

    template<>
    void printValue<std::string>(std::string const & value, std::ostream & output);

    template<>
    void printValue<std::vector<Node>>(std::vector<Node> const & nodes, std::ostream & output);

    template<>
    void printValue<Dict>(Dict const & dict, std::ostream & output);

    void print(Document const & document, std::ostream & output);

}

#endif /* __JSON_H__ */
