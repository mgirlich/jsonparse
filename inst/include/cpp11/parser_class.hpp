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



class Parser_Object : public virtual Parser{
protected:
  std::unordered_map<std::string, std::unique_ptr<Parser>> fields;
  std::unordered_map<std::string, SEXP> default_values;
  std::unordered_map<std::string, bool> key_found;
  std::vector<std::string> field_order;

public:
  Parser_Object(std::unordered_map<std::string, std::unique_ptr<Parser>>& fields,
                std::unordered_map<std::string, SEXP>& default_values,
                std::vector<std::string> field_order) {
    this->default_values = default_values;

    for (cpp11::r_string nm : field_order) {
      this->field_order.push_back(std::string(nm));
    }

    for (auto & field : fields) {
      this->fields.insert({field.first, std::move(field.second)});
      this->key_found[field.first] = false;
    }
  };

  inline SEXP parse_json(simdjson::ondemand::value json, JSON_Path& path) {
    for (auto& it : this->key_found) {
      it.second = false;
    }

    SEXP out = new_named_list(field_order);

    path.insert_dummy(); // insert dummy so that we can always replace the path
    simdjson::ondemand::object object = safe_get_object(json, path);
    for (auto field : object) {
      std::string key = safe_get_key(field);
      if (this->fields.find(key) != fields.end()) {
        path.replace(key);

        this->key_found[key] = true;
        int index = name_to_index(this->field_order, key);
        SET_VECTOR_ELT(out, index, (*this->fields[key]).parse_json(field.value(), path));
      }
    }
    path.drop();

    for (auto& it : this->key_found) {
      if (!it.second) {
        int index = name_to_index(this->field_order, it.first);
        SET_VECTOR_ELT(out, index, default_values[it.first]);
      }
    }

    return out;
  }
};
