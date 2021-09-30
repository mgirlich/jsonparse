#pragma once

#define STRICT_R_HEADERS
#include "cpp11.hpp"
#include "json_utils.hpp"

using simdjson::ondemand::json_type;

inline auto bad_json_type_message(simdjson::ondemand::value element, const std::string& expected, JSON_Path& path) {
    return "Cannot convert a JSON " + json_type_to_string(element) + " to " + expected + " at path " + path.path();
}

// Cannot use template function because
// * `parse_scalar_*()` have different return types
// * C++ 11 doesn't support the auto return type
inline int parse_scalar_bool(simdjson::ondemand::value element, JSON_Path& path) {
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

inline int parse_scalar_int(simdjson::ondemand::value element, JSON_Path& path) {
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

inline double parse_scalar_double(simdjson::ondemand::value element, JSON_Path& path) {
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

inline SEXPREC* parse_scalar_string(simdjson::ondemand::value element, JSON_Path& path) {
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

template <typename T>
inline SEXP parse_homo_array(simdjson::ondemand::value json, JSON_Path& path);

template <>
inline SEXP parse_homo_array<bool>(simdjson::ondemand::value json, JSON_Path& path) {
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

template<>
inline SEXP parse_homo_array<int>(simdjson::ondemand::value json, JSON_Path& path) {
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

template<>
inline SEXP parse_homo_array<double>(simdjson::ondemand::value json, JSON_Path& path) {
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

template<>
inline SEXP parse_homo_array<std::string>(simdjson::ondemand::value json, JSON_Path& path) {
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
