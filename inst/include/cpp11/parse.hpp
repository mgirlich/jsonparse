#define STRICT_R_HEADERS
#include "cpp11.hpp"
#include "json_utils.hpp"

using simdjson::ondemand::json_type;

auto bad_json_type_message(simdjson::ondemand::value element, const std::string& expected, const std::string& path) {
    return "Cannot convert a JSON " + json_type_to_string(element) + " to " + expected + " at path " + path;
}

int parse_scalar_bool(simdjson::ondemand::value element, const std::string& path) {
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
    switch (element.type()) {
    case json_type::number:
        // TODO this is unsafe; add handling of big integers
        return static_cast<int>(int64_t(element));
        break;
    case json_type::null:
        return NA_INTEGER;
        break;
    default:
        throw std::runtime_error(bad_json_type_message(element, "int", path));
    }
}

double parse_scalar_double(simdjson::ondemand::value element, const std::string& path) {
    switch (element.type()) {
    case json_type::number:
        return static_cast<double>(element);
        break;
    case json_type::null:
        return static_cast<double>(NA_REAL);
        break;
    default:
        throw std::runtime_error(bad_json_type_message(element, "double", path));
    }
}

SEXPREC* parse_scalar_string(simdjson::ondemand::value element, const std::string& path) {
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
SEXP parse_homo_array_bool(simdjson::ondemand::value json, const std::string& path) {
    simdjson::ondemand::array array = safe_get_array(json, path);

    int n = array.count_elements();
    SEXP val = PROTECT(Rf_allocVector(LGLSXP, n));

    int i = 0;
    for (auto element : array) {
        SET_LOGICAL_ELT(val, i, parse_scalar_bool(element.value(), path));
        i++;
    }

    UNPROTECT(1);
    return val;
}

SEXP parse_homo_array_int(simdjson::ondemand::value json, const std::string& path) {
    simdjson::ondemand::array array = safe_get_array(json, path);

    int n = array.count_elements();
    SEXP val = PROTECT(Rf_allocVector(INTSXP, n));

    int i = 0;
    for (auto element : array) {
        SET_INTEGER_ELT(val, i, parse_scalar_int(element.value(), path));
        i++;
    }

    UNPROTECT(1);
    return val;
}

SEXP parse_homo_array_double(simdjson::ondemand::value json, const std::string& path) {
    simdjson::ondemand::array array = safe_get_array(json, path);

    int n = array.count_elements();
    SEXP val = PROTECT(Rf_allocVector(REALSXP, n));

    int i = 0;
    for (auto element : array) {
        SET_REAL_ELT(val, i, parse_scalar_double(element.value(), path));
        i++;
    }

    UNPROTECT(1);
    return val;
}

SEXP parse_homo_array_string(simdjson::ondemand::value json, const std::string& path) {
    simdjson::ondemand::array array = safe_get_array(json, path);

    int n = array.count_elements();
    SEXP val = PROTECT(Rf_allocVector(STRSXP, n));

    int i = 0;
    for (auto element : array) {
        SET_STRING_ELT(val, i, parse_scalar_string(element.value(), path));
        i++;
    }

    UNPROTECT(1);
    return val;
}
