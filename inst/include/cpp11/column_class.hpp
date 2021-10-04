#pragma once

#define STRICT_R_HEADERS
#include "parser_class.hpp"

template <typename T>
class Column_Scalar : public virtual Column {
};

template <>
class Column_Scalar<bool> : public virtual Column {
protected:
  int default_val;
  SEXP out;
  int* out_data;
public:
  Column_Scalar(int default_val) {
    this->default_val = default_val;
  }

  inline void reserve(int n) {
    this->out = PROTECT(Rf_allocVector(LGLSXP, n));
    this->out_data = LOGICAL(out);
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    *this->out_data = parse_scalar_bool(json, path);
    ++this->out_data;
  }

  inline void add_default() {
    *this->out_data = this->default_val;
    ++this->out_data;
  }

  inline SEXP get_value() const {
    UNPROTECT(1);
    return this->out;
  }
};

template <>
class Column_Scalar<int> : public virtual Column {
protected:
  int default_val;
  SEXP out;
  int* out_data;
public:
  Column_Scalar(int default_val) {
    this->default_val = default_val;
  }

  inline void reserve(int n) {
    this->out = PROTECT(Rf_allocVector(INTSXP, n));
    this->out_data = INTEGER(out);
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    *this->out_data = parse_scalar_int(json, path);
    ++this->out_data;
  }

  inline void add_default() {
    *this->out_data = this->default_val;
    ++this->out_data;
  }

  inline SEXP get_value() const {
    UNPROTECT(1);
    return this->out;
  }
};

template <>
class Column_Scalar<double> : public virtual Column {
protected:
  double default_val;
  SEXP out;
  double* out_data;
public:
  Column_Scalar(double default_val) {
    this->default_val = default_val;
  }

  inline void reserve(int n) {
    this->out = PROTECT(Rf_allocVector(REALSXP, n));
    this->out_data = REAL(out);
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    *this->out_data = parse_scalar_double(json, path);
    ++this->out_data;
  }

  inline void add_default() {
    *this->out_data = this->default_val;
    ++this->out_data;
  }

  inline SEXP get_value() const {
    UNPROTECT(1);
    return this->out;
  }
};

template <>
class Column_Scalar<std::string> : public virtual Column {
protected:
  SEXP default_val;
  SEXP out;
  SEXP* out_data;
public:
  // TODO simplify constructor to just use SEXP
  Column_Scalar(std::string default_val) {
    // TODO could probably be done nicer
    if (cpp11::is_na(cpp11::r_string(default_val))) {
      this->default_val = R_NaString;
    } else {
      this->default_val = Rf_mkChar(std::string(default_val).c_str());
    }
  }

  inline void reserve(int n) {
    this->out = PROTECT(Rf_allocVector(STRSXP, n));
    this->out_data = STRING_PTR(out);
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    // TODO should this use SET_STRING_ELT
    *this->out_data = parse_scalar_string(json, path);
    ++this->out_data;
  }

  inline void add_default() {
    *this->out_data = this->default_val;
    ++this->out_data;
  }

  inline SEXP get_value() const {
    UNPROTECT(1);
    return this->out;
  }
};


template <typename T>
class Column_Vector : public virtual Column {
protected:
  SEXP default_val;
  SEXP val;
  int i = 0;
public:
  Column_Vector(SEXP default_val) {
    this->default_val = default_val;
  }

  inline void reserve(int n) {
    this->val = PROTECT(Rf_allocVector(VECSXP, n));
    this->i = 0;
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    SEXP vec = parse_homo_array<T>(json, path);
    int vec_size = Rf_length(vec);
    if (vec_size == 0) {
      SET_VECTOR_ELT(this->val, this->i, R_NilValue);
    } else {
      SET_VECTOR_ELT(this->val, this->i, vec);
    }
    this->i++;
  }

  inline void add_default() {
    SET_VECTOR_ELT(this->val, this->i, this->default_val);
    this->i++;
  }

  inline SEXP get_value() const {
    UNPROTECT(1);
    return this->val;
  }
};

class Column_Df : public virtual Column {
protected:
  std::unordered_map<std::string, std::unique_ptr<Column>> val;
  std::unordered_map<std::string, bool> key_found;
  std::vector<std::string> col_order;
  int size = 0;

public:
  Column_Df(std::unordered_map<std::string, std::unique_ptr<Column>>& cols,
            std::vector<std::string> col_order) {
    for (auto & col : cols) {
      this->val.insert({col.first, std::move(col.second)});
      this->key_found[col.first] = false;
    }

    this->col_order = col_order;
  };

  inline void reserve(int n) {
    for (auto& it : val) {
      (*it.second).reserve(n);
    }
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    simdjson::ondemand::object object = safe_get_object(json, path);

    int row = 0;
    path.insert_dummy(); // insert dummy so that we can always replace the path
    for (auto field : object) {
      path.replace(row++);

      path.insert_dummy();
      std::string key = safe_get_key(field);
      if (this->val.find(key) != val.end()) {
        path.replace(key);
        (*this->val[key]).add_value(field.value(), path);
        this->key_found[key] = true;
      }
      path.drop();
    }
    path.drop();

    for (auto& it : this->key_found) {
      if (!it.second) {
        (*this->val[it.first]).add_default();
      }
      it.second = false;
    }

    this->size++;
  }

  inline void add_default() {
    for (auto& it : val) {
      (*this->val[it.first]).add_default();
    }

    this->size++;
  }

  inline SEXP get_value() const {
    SEXP out = new_df(this->col_order, this->size);

    int i = 0;
    for (cpp11::r_string col : col_order) {
      // TODO nicer solution
      std::string col_string = std::string(col);
      auto it = this->val.find(col_string);
      SET_VECTOR_ELT(out, i, (*(*it).second).get_value());
      i++;
    }

    return out;
  }
};
