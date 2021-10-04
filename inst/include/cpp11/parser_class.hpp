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
  virtual inline SEXP get_value() const = 0;
  virtual inline void add_default() = 0;
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



class Parser_Dataframe : public virtual Parser{
protected:
  std::unordered_map<std::string_view, std::unique_ptr<Column>> cols;
  std::unordered_map<std::string_view, bool> key_found;
  std::vector<std::string> col_order;
  std::vector<std::unique_ptr<std::string>> string_view_protection;

public:
  Parser_Dataframe(std::unordered_map<std::string, std::unique_ptr<Column>>& cols,
                   const std::vector<std::string> col_order) {
    for (auto & col : cols) {
      string_view_protection.push_back(std::make_unique<std::string>(col.first));
      // string_view_protection.push_back(std::move(std::make_unique<std::string>(col.first)));
      this->cols.insert({*string_view_protection.back(), std::move(col.second)});
      this->key_found[*string_view_protection.back()] = false;
    }

    this->col_order = col_order;
  };

  inline SEXP parse_json(simdjson::ondemand::value json, JSON_Path& path) {
    simdjson::ondemand::array array = safe_get_array(json, path);

    int size = array.count_elements();
    for (auto & col : this->cols) {
      (*this->cols[col.first]).reserve(size);
      this->key_found[col.first] = false;
    }

    path.insert_dummy();
    int n_rows = 0;
    for (auto element : array) {
      // TODO allow null instead of object?
      path.replace(n_rows);
      simdjson::ondemand::object object = safe_get_object(element.value(), path);

      path.insert_dummy();
      for (auto field : object) {
        std::string_view key_v;
        auto error = field.unescaped_key().get(key_v);
        if (error) {
          // TODO when could this actually happen??
          cpp11::stop("Something went wrong with the key");
        }

        if (this->cols.find(key_v) != cols.end()) {
          path.replace(key_v);
          (*this->cols[key_v]).add_value(field.value(), path);
          this->key_found[key_v] = true;
        }
      }
      path.drop();

      for (auto& it : this->key_found) {
        if (!it.second) {
          (*this->cols[it.first]).add_default();
        }
        it.second = false;
      }
      n_rows++;
    }
    path.drop();

    SEXP out = new_df(this->col_order, n_rows);

    int i = 0;
    for (std::string col : col_order) {
      auto it = this->cols.find(col);
      if (it != cols.end()) {
        SET_VECTOR_ELT(out, i, (*(*it).second).get_value());
      }
      i++;
    }

    return out;
  }
};
