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

// defined here because it is needed in Parser_Dataframe
class Column {
public:
  virtual ~Column() {};

  // = 0 to declare the functions as pure virtual
  virtual inline void reserve(int n) = 0;
  virtual inline void add_value(simdjson::ondemand::value, JSON_Path& path) = 0;
  virtual inline void finalize_row() = 0;
  virtual inline SEXP get_value() const = 0;
};

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
  std::unordered_map<std::string_view, std::unique_ptr<Parser>> fields;
  std::unordered_map<std::string_view, SEXP> default_values;
  std::unordered_map<std::string_view, bool> key_found;
  std::vector<std::string> field_order;
  std::vector<std::unique_ptr<std::string>> string_view_protection;

public:
  Parser_Object(std::unordered_map<std::string, std::unique_ptr<Parser>>& fields,
                std::unordered_map<std::string, SEXP>& default_values,
                std::vector<std::string> field_order) {
    this->field_order = field_order;

    for (std::string& field_name : field_order) {
      this->string_view_protection.push_back(std::make_unique<std::string>(field_name));
      this->fields.insert({*string_view_protection.back(), std::move(fields[field_name])});
      this->default_values.insert({*string_view_protection.back(), std::move(default_values[field_name])});
      this->key_found.insert({*string_view_protection.back(), false});
    }
  };

  inline SEXP parse_json(simdjson::ondemand::value json, JSON_Path& path) {
    for (auto& it : this->key_found) {
      it.second = false;
    }

    SEXP out = PROTECT(new_named_list(field_order));

    path.insert_dummy<std::string_view>(); // insert dummy so that we can always replace the path
    simdjson::ondemand::object object = safe_get_object(json, path);
    for (auto field : object) {
      std::string_view key = safe_get_key(field);

      auto it = this->fields.find(key);
      if (it != fields.end()) {
        path.replace(key);
        this->key_found[key] = true;
        int index = name_to_index(this->field_order, key);
        auto value = (*(*it).second).parse_json(field.value(), path);
        SET_VECTOR_ELT(out, index, value);
      }
    }
    path.drop();

    for (auto& it : this->key_found) {
      if (!it.second) {
        int index = name_to_index(this->field_order, it.first);
        SET_VECTOR_ELT(out, index, default_values[it.first]);
      }
    }

    UNPROTECT(1);
    return out;
  }
};



class Parser_Dataframe : public virtual Parser{
protected:
  std::unordered_map<std::string_view, std::unique_ptr<Column>> cols;
  std::vector<std::string> col_order;
  std::vector<std::unique_ptr<std::string>> string_view_protection;
  int current_row = 0;

public:
  Parser_Dataframe(std::unordered_map<std::string, std::unique_ptr<Column>>& cols,
                   const std::vector<std::string> col_order) {
    for (auto & col : cols) {
      this->string_view_protection.push_back(std::make_unique<std::string>(col.first));
      this->cols.insert({*string_view_protection.back(), std::move(col.second)});
    }

    this->col_order = col_order;
  };

  inline SEXP parse_json(simdjson::ondemand::value json, JSON_Path& path) {
    simdjson::ondemand::array array = safe_get_array(json, path);

    int size = array.count_elements();
    for (auto& col : this->cols) {
      (*col.second).reserve(size);
    }

    path.insert_dummy<int>();
    this->current_row = 0;
    for (auto element : array) {
      path.replace(this->current_row);
      // TODO allow null instead of object?
      simdjson::ondemand::object object = safe_get_object(element.value(), path);

      path.insert_dummy<std::string_view>();
      for (auto field : object) {
        std::string_view key = safe_get_key(field);

        auto it = this->cols.find(key);
        if (it != cols.end()) {
          path.replace(key);
          (*(*it).second).add_value(field.value(), path);
        }
      }
      path.drop();

      for (auto& col : this->cols) {
        (*col.second).finalize_row();
      }
      this->current_row++;
    }
    path.drop();

    SEXP out = PROTECT(new_df(this->col_order, size));
    for (auto& col : this->cols) {
      int index = name_to_index(this->col_order, col.first);
      SET_VECTOR_ELT(out, index, (*col.second).get_value());
    }

    UNPROTECT(1);
    return out;
  }
};
