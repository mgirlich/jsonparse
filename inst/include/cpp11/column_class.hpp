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
  bool added_value = false;
  bool needs_unprotect = false;

public:
  Column_Scalar(int default_val) {
    this->default_val = default_val;
  }

  ~Column_Scalar() {
    if (needs_unprotect) UNPROTECT(1);
  }

  inline void reserve(int n) {
    this->out = PROTECT(Rf_allocVector(LGLSXP, n));
    this->out_data = LOGICAL(out);
    this->needs_unprotect = true;
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    *this->out_data = parse_scalar_bool(json, path);
    ++this->out_data;
    this->added_value = true;
  }

  inline void finalize_row() {
    if (this->added_value) {
      this->added_value = false;
    }  else {
      *this->out_data = this->default_val;
      ++this->out_data;
    }
  }

  inline SEXP get_value() {
    UNPROTECT(1);
    this->needs_unprotect = false;
    return this->out;
  }
};

template <>
class Column_Scalar<int> : public virtual Column {
protected:
  int default_val;
  SEXP out;
  int* out_data;
  bool added_value = false;
  bool needs_unprotect = false;

public:
  Column_Scalar(int default_val) {
    this->default_val = default_val;
  }

  ~Column_Scalar() {
    if (needs_unprotect) UNPROTECT(1);
  }

  inline void reserve(int n) {
    this->out = PROTECT(Rf_allocVector(INTSXP, n));
    this->out_data = INTEGER(out);
    this->needs_unprotect = true;
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    *this->out_data = parse_scalar_int(json, path);
    ++this->out_data;
    this->added_value = true;
  }

  inline void finalize_row() {
    if (this->added_value) {
      this->added_value = false;
    }  else {
      *this->out_data = this->default_val;
      ++this->out_data;
    }
  }

  inline SEXP get_value() {
    UNPROTECT(1);
    this->needs_unprotect = false;
    return this->out;
  }
};

template <>
class Column_Scalar<double> : public virtual Column {
protected:
  double default_val;
  SEXP out;
  double* out_data;
  bool added_value = false;
  bool needs_unprotect = false;

public:
  Column_Scalar(double default_val) {
    this->default_val = default_val;
  }

  ~Column_Scalar() {
    if (needs_unprotect) UNPROTECT(1);
  }

  inline void reserve(int n) {
    this->out = PROTECT(Rf_allocVector(REALSXP, n));
    this->out_data = REAL(out);
    this->needs_unprotect = true;
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    *this->out_data = parse_scalar_double(json, path);
    ++this->out_data;
    this->added_value = true;
  }

  inline void finalize_row() {
    if (this->added_value) {
      this->added_value = false;
    }  else {
      *this->out_data = this->default_val;
      ++this->out_data;
    }
  }

  inline SEXP get_value() {
    UNPROTECT(1);
    this->needs_unprotect = false;
    return this->out;
  }
};

template <>
class Column_Scalar<std::string> : public virtual Column {
protected:
  SEXP default_val;
  SEXP out;
  SEXP* out_data;
  bool added_value = false;
  bool needs_unprotect = false;

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

  ~Column_Scalar() {
    if (needs_unprotect) UNPROTECT(1);
  }

  inline void reserve(int n) {
    this->out = PROTECT(Rf_allocVector(STRSXP, n));
    this->out_data = STRING_PTR(out);
    this->needs_unprotect = true;
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    // TODO should this use SET_STRING_ELT
    *this->out_data = parse_scalar_string(json, path);
    ++this->out_data;
    this->added_value = true;
  }

  inline void finalize_row() {
    if (this->added_value) {
      this->added_value = false;
    }  else {
      *this->out_data = this->default_val;
      ++this->out_data;
    }
  }

  inline SEXP get_value() {
    UNPROTECT(1);
    this->needs_unprotect = false;
    return this->out;
  }
};


template <typename T>
class Column_Vector : public virtual Column {
protected:
  SEXP default_val;
  SEXP val;
  int i = 0;
  bool added_value = false;
  bool needs_unprotect = false;

public:
  Column_Vector(SEXP default_val) {
    this->default_val = default_val;
  }

  ~Column_Vector() {
    if (needs_unprotect) UNPROTECT(1);
  }

  inline void reserve(int n) {
    this->val = PROTECT(Rf_allocVector(VECSXP, n));
    this->i = 0;
    this->needs_unprotect = true;
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
    this->added_value = true;
  }

  inline void finalize_row() {
    if (this->added_value) {
      this->added_value = false;
    }  else {
      SET_VECTOR_ELT(this->val, this->i, this->default_val);
      this->i++;
    }
  }

  inline SEXP get_value() {
    UNPROTECT(1);
    this->needs_unprotect = false;
    return this->val;
  }
};

class Column_Df : public virtual Column {
protected:
  std::unordered_map<std::string_view, std::unique_ptr<Column>> val;
  std::vector<std::string> col_order;
  std::vector<std::unique_ptr<std::string>> string_view_protection;
  int size = 0;
  bool added_value = false;

public:
  Column_Df(std::unordered_map<std::string, std::unique_ptr<Column>>& cols,
            std::vector<std::string> col_order) {
    for (auto & col : cols) {
      this->string_view_protection.push_back(std::make_unique<std::string>(col.first));
      this->val.insert({*string_view_protection.back(), std::move(col.second)});
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

    path.insert_dummy<std::string_view>(); // insert dummy so that we can always replace the path
    for (auto field : object) {
      std::string_view key = safe_get_key(field);

      auto it = this->val.find(key);
      if (it != val.end()) {
        path.replace(key);
        (*(*it).second).add_value(field.value(), path);
      }

    }
    path.drop();

    for (auto& col : this->val) {
      (*col.second).finalize_row();
    }

    this->size++;
    this->added_value = true;
  }

  inline void finalize_row() {
    if (this->added_value) {
      this->added_value = false;
    }  else {
      for (auto& it : val) {
        (*this->val[it.first]).finalize_row();
      }

      this->size++;
    }
  }

  inline SEXP get_value() {
    SEXP out = PROTECT(new_df(this->col_order, size));
    for (auto& col : this->val) {
      int index = name_to_index(this->col_order, col.first);
      SET_VECTOR_ELT(out, index, (*col.second).get_value());
    }

    UNPROTECT(1);
    return out;
  }
};

class Column_ListOfDf : public virtual Column {
protected:
  SEXP val;
  Parser_Dataframe df_parser;
  int current_row = 0;
  bool added_value = false;
  bool needs_unprotect = false;

public:
  // TODO what exactly is this syntax?
  // https://stackoverflow.com/a/43306073
  Column_ListOfDf(std::unordered_map<std::string, std::unique_ptr<Column>>& list_element,
                  std::vector<std::string> col_order) : df_parser(list_element, col_order) {
  }

  ~Column_ListOfDf() {
    if (needs_unprotect) UNPROTECT(1);
  }

  inline void reserve(int n) {
    this->val = PROTECT(Rf_allocVector(VECSXP, n));
    this->current_row = 0;
  }

  inline void add_value(simdjson::ondemand::value json, JSON_Path& path) {
    SET_VECTOR_ELT(this->val, this->current_row, this->df_parser.parse_json(json, path));
    this->current_row++;
    this->added_value = true;
  }

  inline void finalize_row() {
    if (this->added_value) {
      this->added_value = false;
    } else {
      SET_VECTOR_ELT(this->val, this->current_row, R_NilValue);
      this->current_row++;
    }
  }

  inline SEXP get_value() {
    UNPROTECT(1);
    this->needs_unprotect = false;
    return this->val;
  }
};
