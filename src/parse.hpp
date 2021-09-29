#define STRICT_R_HEADERS
#include "cpp11.hpp"
#include "json_utils.hpp"

auto bad_json_type_message(simdjson::ondemand::value element, const std::string& expected, const std::string& path) {
    return "Cannot convert a JSON " + json_type_to_string(element) + " to " + expected + " at path " + path;
}

int parse_scalar_bool(simdjson::ondemand::value element, const std::string& path) {
    using simdjson::ondemand::json_type;

    switch (element.type()) {
    case json_type::null:
        return NA_LOGICAL;
        break;
    case json_type::boolean:
        return bool(element) ? 1 : 0;
        break;
    default:
        throw std::runtime_error(bad_json_type_message(element, "bool", path));
    }
}

int parse_scalar_int(simdjson::ondemand::value element, const std::string& path) {
    using simdjson::ondemand::json_type;

    switch (element.type()) {
    case json_type::number:
        // TODO this is unsafe; add handling of big integers
        return int(int64_t(element));
        break;
    case json_type::null:
        return NA_INTEGER;
        break;
    default:
        throw std::runtime_error(bad_json_type_message(element, "int", path));
    }
}

double parse_scalar_double(simdjson::ondemand::value element, const std::string& path) {
    using simdjson::ondemand::json_type;

    switch (element.type()) {
    case json_type::number:
        return double(element);
        break;
    case json_type::null:
        return NA_REAL;
        break;
    default:
        throw std::runtime_error(bad_json_type_message(element, "double", path));
    }
}

SEXPREC* parse_scalar_string(simdjson::ondemand::value element, const std::string& path) {
    using simdjson::ondemand::json_type;

    switch (element.type()) {
    case json_type::string:
        return Rf_mkChar(std::string(std::string_view(element)).c_str());
        break;
    case json_type::null:
        return NA_STRING;
        break;
    default:
        throw std::runtime_error(bad_json_type_message(element, "string", path));
    }
}
