// All test files should include the <testthat.h>
// header file.
#include <cpp11/column_class.hpp>
#include <testthat.h>

std::string as_string(SEXP x) {
  return std::string(CHAR(Rf_asChar(x)));
}

context("Parser_Scalar") {
  using namespace simdjson;
  using namespace cpp11;

  auto json = R"(  {
    "lgl_true": true, "lgl_false": false, "lgl_null": null,
    "int_1": 1, "int_null": null,
    "dbl_1.5": 1.5, "dbl_null": null,
    "str_empty": "", "str_abc": "abc", "str_null": null
  }  )"_padded;
  ondemand::parser parser;
  auto doc = parser.iterate(json);

  auto parser_lgl = Parser_Scalar<bool>();
  auto parser_int = Parser_Scalar<int>();
  auto parser_dbl = Parser_Scalar<double>();
  auto parser_str = Parser_Scalar<std::string>();

  auto path = JSON_Path();

  test_that("can parse a scalar bool") {
    path.insert(std::string_view("lgl_true"));
    expect_true(parser_lgl.parse_json(doc["lgl_true"].value(), path) == as_sexp(true));

    path.replace(std::string_view("lgl_false"));
    expect_true(parser_lgl.parse_json(doc["lgl_false"].value(), path) == as_sexp(false));

    path.replace(std::string_view("lgl_null"));
    expect_true(parser_lgl.parse_json(doc["lgl_null"].value(), path) == Rf_ScalarLogical(NA_LOGICAL));
  }

  test_that("can parse a scalar integer") {
    expect_true(Rf_asInteger(parser_int.parse_json(doc["int_1"].value(), path)) == 1);
    expect_true(Rf_asInteger(parser_int.parse_json(doc["int_null"].value(), path)) == NA_INTEGER);
  }

  test_that("can parse a scalar double") {
    expect_true(Rf_asReal(parser_dbl.parse_json(doc["dbl_1.5"].value(), path)) == 1.5);
    expect_true(ISNA(REAL(parser_dbl.parse_json(doc["dbl_null"].value(), path))[0]));
  }

  test_that("can parse a scalar string") {
    expect_true(as_string(parser_str.parse_json(doc["str_empty"].value(), path)) == "");
    expect_true(as_string(parser_str.parse_json(doc["str_abc"].value(), path)) == "abc");
    expect_true(as_string(parser_str.parse_json(doc["str_null"].value(), path)) == as_string(NA_STRING));
 }
}

context("Parser_HomoArray") {
  using namespace simdjson;
  using namespace cpp11;

  auto json = R"(  {
    "lgl": [true, null, false],
    "int": [1, null, 2],
    "dbl": [1.5, null, -1.5],
    "str": ["", null, "abc"]
  }  )"_padded;
  ondemand::parser parser;
  auto doc = parser.iterate(json);

  auto parser_lgl = Parser_HomoArray<bool>();
  auto parser_int = Parser_HomoArray<int>();
  auto parser_dbl = Parser_HomoArray<double>();
  auto parser_str = Parser_HomoArray<std::string>();

  auto path = JSON_Path();

  test_that("can parse an array of bools") {
    logicals x = parser_lgl.parse_json(doc["lgl"].value(), path);
    expect_true(x[0] == true);
    expect_true(x[1] == NA_LOGICAL);
    expect_true(x[2] == false);
  }

  test_that("can parse an array of ints") {
    integers x = parser_int.parse_json(doc["int"].value(), path);
    expect_true(x[0] == 1);
    expect_true(x[1] == NA_INTEGER);
    expect_true(x[2] == 2);
  }

  test_that("can parse an array of doubles") {
    doubles x = parser_dbl.parse_json(doc["dbl"].value(), path);
    expect_true(x[0] == 1.5);
    expect_true(is_na(x[1]));
    expect_true(x[2] == -1.5);
  }

  test_that("can parse an array of strings") {
    strings x = parser_str.parse_json(doc["str"].value(), path);
    expect_true(x[0] == "");
    expect_true(x[1] == NA_STRING);
    expect_true(x[2] == "abc");
  }
}

context("Parser_Object") {
  using namespace simdjson;
  using namespace cpp11;

  auto json = R"(  {
    "lgl": true,
    "int": 1,
    "dbl": 1.5,
    "str": "abc",
    "lgl_vec": [true, null, false],
    "int_vec": [1, null, 2],
    "dbl_vec": [1.5, null, -1.5],
    "str_vec": ["", null, "abc"]
  }  )"_padded;

  std::unordered_map<std::string, std::unique_ptr<Parser>> fields;
  fields["lgl"] = std::make_unique<Parser_Scalar<bool>>();
  fields["int"] = std::make_unique<Parser_Scalar<int>>();
  fields["dbl"] = std::make_unique<Parser_Scalar<double>>();
  fields["str"] = std::make_unique<Parser_Scalar<std::string>>();

  fields["lgl_vec"] = std::make_unique<Parser_HomoArray<bool>>();
  fields["int_vec"] = std::make_unique<Parser_HomoArray<int>>();
  fields["dbl_vec"] = std::make_unique<Parser_HomoArray<double>>();
  fields["str_vec"] = std::make_unique<Parser_HomoArray<std::string>>();

  std::unordered_map<std::string, SEXP> default_values;
  default_values["lgl"] = Rf_ScalarLogical(false);
  default_values["int"] = Rf_ScalarInteger(-1);
  default_values["dbl"] = Rf_ScalarReal(-1.5);
  default_values["str"] = Rf_mkChar("xyz");

  default_values["lgl_vec"] = logicals({false, false});
  default_values["int_vec"] = integers({-1, -2});
  default_values["dbl_vec"] = doubles({-1.5, -2.5});
  default_values["str_vec"] = strings({"x", "y", "z"});

  std::vector<std::string> field_order = std::vector<std::string>({
    "lgl", "int", "dbl", "str",
    "lgl_vec", "int_vec", "dbl_vec", "str_vec"
  });


  auto parser_ob = Parser_Object(fields, default_values, field_order);
  auto path = JSON_Path();

  ondemand::parser parser;
  auto doc = parser.iterate(json);
  simdjson::ondemand::value value = doc;
  test_that("can parse an object") {
    cpp11::list x = parser_ob.parse_json(value, path);
    expect_true(as_cpp<bool>(x["lgl"]) == true);
    expect_true(as_cpp<int>(x["int"]) == 1);
    expect_true(as_cpp<double>(x["dbl"]) == 1.5);
    expect_true(as_cpp<std::string>(x["str"]) == "abc");


    expect_true(logicals(x["lgl_vec"]) == logicals({true, NA_LOGICAL, false}));
    expect_true(integers(x["int_vec"]) == integers({1, NA_INTEGER, 2}));
    cpp11::doubles x_dbl_vec = doubles(x["dbl_vec"]);
    expect_true(x_dbl_vec[0] == 1.5);
    expect_true(is_na(x_dbl_vec[1]));
    expect_true(x_dbl_vec[2] == -1.5);
    expect_true(strings(x["str_vec"]) == strings({"", NA_STRING, "abc"}));
  }

  auto json2 = R"(  {}   )"_padded;
  auto doc2 = parser.iterate(json2);
  simdjson::ondemand::value value2 = doc2;
  test_that("uses `default_values` for missing fields") {
    cpp11::list x = parser_ob.parse_json(value2, path);
    expect_true(as_cpp<bool>(x["lgl"]) == false);
    expect_true(as_cpp<int>(x["int"]) == -1);
    expect_true(as_cpp<double>(x["dbl"]) == -1.5);
    expect_true(r_string(x["str"]) == "xyz");

    expect_true(logicals(x["lgl_vec"]) == logicals({false, false}));
    expect_true(integers(x["int_vec"]) == integers({-1, -2}));
    expect_true(doubles(x["dbl_vec"]) == doubles({-1.5, -2.5}));
    expect_true(strings(x["str_vec"]) == strings({"x", "y", "z"}));
  }
}

context("Parser_Dataframe") {
  using namespace simdjson;
  using namespace cpp11;

  auto json = R"(  [{
    "lgl": true,
    "int": 1,
    "dbl": 1.5,
    "str": "abc",
    "lgl_vec": [true, null],
    "int_vec": [1, null],
    "dbl_vec": [1.5, null],
    "str_vec": ["", null]
  },
  {
    "lgl": null,
    "int": 2,
    "dbl": 2.5,
    "str": "def",
    "lgl_vec": [null, false],
    "int_vec": [null, 2],
    "dbl_vec": [null, -1.5],
    "str_vec": [null, "abc"]
  },
  {}]  )"_padded;

  std::unordered_map<std::string, std::unique_ptr<Column>> cols;
  cols["lgl"] = std::make_unique<Column_Scalar<bool>>(false);
  cols["int"] = std::make_unique<Column_Scalar<int>>(-1);
  cols["dbl"] = std::make_unique<Column_Scalar<double>>(-1.5);
  cols["str"] = std::make_unique<Column_Scalar<std::string>>("xyz");

  cols["lgl_vec"] = std::make_unique<Column_Vector<bool>>(logicals({false, false}));
  cols["int_vec"] = std::make_unique<Column_Vector<int>>(integers({-1, -2}));
  cols["dbl_vec"] = std::make_unique<Column_Vector<double>>(doubles({-1.5, -2.5}));
  cols["str_vec"] = std::make_unique<Column_Vector<std::string>>(strings({"x", "y", "z"}));

  std::vector<std::string> col_order = std::vector<std::string>({
    "lgl", "int", "dbl", "str",
    "lgl_vec", "int_vec", "dbl_vec", "str_vec"
  });


  auto parser_df = Parser_Dataframe(cols, col_order);
  auto path = JSON_Path();

  ondemand::parser parser;
  auto doc = parser.iterate(json);
  simdjson::ondemand::value value = doc;
  test_that("can parse an array of objects") {
    list x = parser_df.parse_json(value, path);
    logicals x_lgl = x["lgl"];
    expect_true(x_lgl.size() == 3);

    expect_true(x_lgl == logicals({true, NA_LOGICAL, false}));
    expect_true(integers(x["int"]) == integers({1, 2, -1}));
    expect_true(doubles(x["dbl"]) == doubles({1.5, 2.5, -1.5}));
    expect_true(strings(x["str"]) == strings({"abc", "def", "xyz"}));

    auto x_lgl_vec = list(x["lgl_vec"]);
    expect_true(logicals(x_lgl_vec[0]) == logicals({true, NA_LOGICAL}));
    expect_true(logicals(x_lgl_vec[1]) == logicals({NA_LOGICAL, false}));
    expect_true(logicals(x_lgl_vec[2]) == logicals({false, false}));

    auto x_int_vec = list(x["int_vec"]);
    expect_true(integers(x_int_vec[0]) == integers({1, NA_INTEGER}));
    expect_true(integers(x_int_vec[1]) == integers({NA_LOGICAL, 2}));
    expect_true(integers(x_int_vec[2]) == integers({-1, -2}));

    auto x_dbl_vec = list(x["dbl_vec"]);
    auto x_dbl_vec0 = doubles(x_dbl_vec[0]);
    expect_true(x_dbl_vec0[0] == 1.5);
    expect_true(is_na(x_dbl_vec0[1]));

    auto x_dbl_vec1 = doubles(x_dbl_vec[1]);
    expect_true(is_na(x_dbl_vec1[0]));
    expect_true(x_dbl_vec1[1] == -1.5);
    expect_true(doubles(x_dbl_vec[2]) == doubles({-1.5, -2.5}));

    auto x_str_vec = list(x["str_vec"]);
    expect_true(strings(x_str_vec[0]) == strings({"", NA_STRING}));
    expect_true(strings(x_str_vec[1]) == writable::strings({NA_STRING, "abc"}));
    expect_true(strings(x_str_vec[2]) == strings({"x", "y", "z"}));
  }

  // TODO should check error message
  auto json1 = R"(  [{
    "lgl": 1,
  }]  )"_padded;
  doc = parser.iterate(json1);
  value = doc;
  expect_error(parser_df.parse_json(value, path));
}
