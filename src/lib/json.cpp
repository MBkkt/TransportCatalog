#include "json.h"


using namespace std;

namespace Json {

    Node loadArray(istream & input) {
        vector<Node> result;

        for (char c; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(loadNode(input));
        }

        return Node(move(result));
    }

    Node loadBool(istream & input) {
        string s;
        while (isalpha(input.peek())) {
            s.push_back(input.get());
        }
        return Node(s == "true");
    }

    Node loadNumber(istream & input) {
        bool is_negative = false;
        if (input.peek() == '-') {
            is_negative = true;
            input.get();
        }
        int int_part = 0;
        while (isdigit(input.peek())) {
            int_part *= 10;
            int_part += input.get() - '0';
        }
        if (input.peek() != '.') {
            return Node(int_part * (is_negative ? -1 : 1));
        }
        input.get();  // '.'
        double result    = int_part;
        double frac_mult = 0.1;
        while (isdigit(input.peek())) {
            result += frac_mult * (input.get() - '0');
            frac_mult /= 10;
        }
        return Node(result * (is_negative ? -1 : 1));
    }

    Node loadString(istream & input) {
        string line;
        getline(input, line, '"');
        return Node(move(line));
    }

    Node loadDict(istream & input) {
        Dict result;

        for (char c; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            string key = loadString(input).asString();
            input >> c;
            result.emplace(move(key), loadNode(input));
        }

        return Node(move(result));
    }

    Node loadNode(istream & input) {
        char c;
        input >> c;

        if (c == '[') {
            return loadArray(input);
        } else if (c == '{') {
            return loadDict(input);
        } else if (c == '"') {
            return loadString(input);
        } else if (c == 't' || c == 'f') {
            input.putback(c);
            return loadBool(input);
        } else {
            input.putback(c);
            return loadNumber(input);
        }
    }

    Document load(istream & input) {
        return Document {loadNode(input)};
    }

    template<>
    void printValue<string>(string const & value, ostream & output) {
        output << '"' << value << '"';
    }

    template<>
    void printValue<bool>(const bool & value, std::ostream & output) {
        output << std::boolalpha << value;
    }

    template<>
    void printValue<std::vector<Node>>(std::vector<Node> const & nodes, std::ostream & output) {
        output << '[';
        bool first = true;
        for (const Node & node : nodes) {
            if (!first) {
                output << ", ";
            }
            first = false;
            printNode(node, output);
        }
        output << ']';
    }

    template<>
    void printValue<Dict>(Dict const & dict, std::ostream & output) {
        output << '{';
        bool first = true;
        for (auto const &[key, node]: dict) {
            if (!first) {
                output << ", ";
            }
            first = false;
            printValue(key, output);
            output << ": ";
            printNode(node, output);
        }
        output << '}';
    }

    void printNode(Json::Node const & node, ostream & output) {
        visit([&output](auto const & value) {printValue(value, output);},
              node.getBase());
    }

    void print(Document const & document, ostream & output) {
        printNode(document.getRoot(), output);
    }

}
