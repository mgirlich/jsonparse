#pragma once

#define STRICT_R_HEADERS
#include "cpp11.hpp"
#include "cpp11/R.hpp"

#if __cplusplus >= 201703L
#include <cpp11/parse.hpp>
#include <cpp11/utils.hpp>
#include <unordered_map>
#include <memory>
#endif

class Parser {
public:
  virtual ~Parser() {};

  virtual inline SEXP parse_json(simdjson::ondemand::value, JSON_Path& path) = 0;
};

template <typename T>
class Parser_Scalar : public virtual Parser {};

template <>
class Parser_Scalar<bool> : public virtual Parser {
public:
  inline SEXP parse_json(simdjson::ondemand::value json, JSON_Path& path) {
    return Rf_ScalarLogical(parse_scalar_bool(json, path));
  }
};

template <>
class Parser_Scalar<int> : public virtual Parser {
public:
  inline SEXP parse_json(simdjson::ondemand::value json, JSON_Path& path) {
    return Rf_ScalarInteger(parse_scalar_int(json, path));
  }
};

template <>
class Parser_Scalar<double> : public virtual Parser {
public:
  inline SEXP parse_json(simdjson::ondemand::value json, JSON_Path& path) {
    return Rf_ScalarReal(parse_scalar_double(json, path));
  }
};

template <>
class Parser_Scalar<std::string> : public virtual Parser {
public:
  inline SEXP parse_json(simdjson::ondemand::value json, JSON_Path& path) {
    return Rf_ScalarString(parse_scalar_string(json, path));
  }
};



template <typename T>
class Parser_HomoArray : public virtual Parser {
public:
  inline SEXP parse_json(simdjson::ondemand::value json, JSON_Path& path) {
    return parse_homo_array<T>(json, path);
  }
};
