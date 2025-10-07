// C++ standard library JSON benchmark using std::variant, std::unordered_map, and std::vector
// NOTE: C++ standard library does not have built-in JSON support until C++23
// This implementation uses a simple recursive descent parser

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory>
#include <cctype>
#include <stdexcept>

// Forward declaration
struct JsonValue;

// JSON types using standard library containers
using JsonNull = std::monostate;
using JsonBool = bool;
using JsonNumber = double;
using JsonString = std::string;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::unordered_map<std::string, JsonValue>;

// JsonValue can be any of these types
struct JsonValue {
    std::variant<JsonNull, JsonBool, JsonNumber, JsonString, JsonArray, JsonObject> data;

    JsonValue() : data(JsonNull{}) {}

    template<typename T>
    JsonValue(T&& value) : data(std::forward<T>(value)) {}
};

// Simple JSON parser
class JsonParser {
private:
    const std::string& input;
    size_t pos;

    void skip_whitespace() {
        while (pos < input.size() && std::isspace(input[pos])) {
            pos++;
        }
    }

    char peek() {
        skip_whitespace();
        return (pos < input.size()) ? input[pos] : '\0';
    }

    char consume() {
        skip_whitespace();
        return (pos < input.size()) ? input[pos++] : '\0';
    }

    bool match(char c) {
        if (peek() == c) {
            consume();
            return true;
        }
        return false;
    }

    std::string parse_string() {
        if (consume() != '"') throw std::runtime_error("Expected '\"'");

        std::string result;
        while (pos < input.size() && input[pos] != '"') {
            if (input[pos] == '\\') {
                pos++;
                if (pos >= input.size()) throw std::runtime_error("Unexpected end in string");
                switch (input[pos]) {
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    case '\\': result += '\\'; break;
                    case '"': result += '"'; break;
                    default: result += input[pos];
                }
                pos++;
            } else {
                result += input[pos++];
            }
        }

        if (pos >= input.size()) throw std::runtime_error("Unterminated string");
        pos++; // consume closing '"'
        return result;
    }

    double parse_number() {
        size_t start = pos;
        if (input[pos] == '-') pos++;

        while (pos < input.size() && std::isdigit(input[pos])) pos++;

        if (pos < input.size() && input[pos] == '.') {
            pos++;
            while (pos < input.size() && std::isdigit(input[pos])) pos++;
        }

        if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E')) {
            pos++;
            if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) pos++;
            while (pos < input.size() && std::isdigit(input[pos])) pos++;
        }

        return std::stod(input.substr(start, pos - start));
    }

    JsonValue parse_value();

    JsonArray parse_array() {
        consume(); // '['
        JsonArray arr;

        if (peek() == ']') {
            consume();
            return arr;
        }

        while (true) {
            arr.push_back(parse_value());
            if (match(',')) continue;
            if (match(']')) break;
            throw std::runtime_error("Expected ',' or ']'");
        }

        return arr;
    }

    JsonObject parse_object() {
        consume(); // '{'
        JsonObject obj;

        if (peek() == '}') {
            consume();
            return obj;
        }

        while (true) {
            std::string key = parse_string();
            if (!match(':')) throw std::runtime_error("Expected ':'");
            obj[key] = parse_value();

            if (match(',')) continue;
            if (match('}')) break;
            throw std::runtime_error("Expected ',' or '}'");
        }

        return obj;
    }

public:
    JsonParser(const std::string& str) : input(str), pos(0) {}

    JsonValue parse() {
        return parse_value();
    }
};

JsonValue JsonParser::parse_value() {
    char c = peek();

    if (c == '{') return JsonValue(parse_object());
    if (c == '[') return JsonValue(parse_array());
    if (c == '"') return JsonValue(parse_string());
    if (c == 't') {
        if (input.substr(pos, 4) == "true") {
            pos += 4;
            return JsonValue(true);
        }
        throw std::runtime_error("Invalid token");
    }
    if (c == 'f') {
        if (input.substr(pos, 5) == "false") {
            pos += 5;
            return JsonValue(false);
        }
        throw std::runtime_error("Invalid token");
    }
    if (c == 'n') {
        if (input.substr(pos, 4) == "null") {
            pos += 4;
            return JsonValue(JsonNull{});
        }
        throw std::runtime_error("Invalid token");
    }
    if (c == '-' || std::isdigit(c)) {
        return JsonValue(parse_number());
    }

    throw std::runtime_error("Unexpected character");
}

// Get RSS (Resident Set Size) in kilobytes from /proc/self/status
long get_rss_kb()
{
#ifdef __linux__
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line))
    {
        if (line.substr(0, 6) == "VmRSS:")
        {
            // Extract the number from "VmRSS:    12345 kB"
            size_t start = line.find_first_of("0123456789");
            size_t end = line.find(" kB");
            if (start != std::string::npos && end != std::string::npos)
            {
                return std::stol(line.substr(start, end - start));
            }
        }
    }
#endif
    return -1; // Not available on non-Linux systems
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    // Measure RSS before loading
    long rss_before = get_rss_kb();

    std::ifstream input_file(argv[1]);
    if (!input_file.is_open())
    {
        std::cerr << "Failed to open the file: " << argv[1] << std::endl;
        return 1;
    }

    std::vector<JsonValue> json_array;
    std::string line;

    while (std::getline(input_file, line))
    {
        if (line.empty()) continue;

        try {
            JsonParser parser(line);
            json_array.push_back(parser.parse());
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
            continue;
        }
    }

    // Measure RSS after loading
    long rss_after = get_rss_kb();

    if (rss_before >= 0 && rss_after >= 0)
    {
        long rss_diff = rss_after - rss_before;
        std::cout << "cppstd RSS: " << rss_diff << " KB" << std::endl;
    }
    else
    {
        std::cerr << "RSS measurement not available on this platform" << std::endl;
    }

    return 0;
}
