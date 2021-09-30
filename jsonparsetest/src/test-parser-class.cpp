// All test files should include the <testthat.h>
// header file.
#include <cpp11/parser_class.hpp>
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
    path.insert("lgl_true");
    expect_true(parser_lgl.parse_json(doc["lgl_true"].value(), path) == as_sexp(true));

    path.replace("lgl_false");
    expect_true(parser_lgl.parse_json(doc["lgl_false"].value(), path) == as_sexp(false));

    path.replace("lgl_null");
    expect_true(parser_lgl.parse_json(doc["lgl_null"].value(), path) == Rf_ScalarLogical(NA_LOGICAL));
  }

  test_that("can parse a scalar integer") {
    expect_true(Rf_asInteger(parser_int.parse_json(doc["int_1"].value(), path)) == 1);
    expect_true(Rf_asInteger(parser_int.parse_json(doc["int_null"].value(), path)) == NA_INTEGER);
  }

  test_that("can parse a scalar double") {
    expect_true(Rf_asReal(parser_dbl.parse_json(doc["dbl_1.5"].value(), path)) == 1.5);
    expect_true(is_na(Rf_asReal(parser_dbl.parse_json(doc["dbl_null"].value(), path))));
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

  default_values["lgl_vec"] = cpp11::logicals({false, false});
  default_values["int_vec"] = cpp11::integers({-1, -2});
  default_values["dbl_vec"] = cpp11::doubles({-1.5, -2.5});
  default_values["str_vec"] = cpp11::strings({"x", "y", "z"});

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
    expect_true(doubles(x["dbl_vec"]) == doubles({1.5, NA_REAL, -1.5}));
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
    expect_true(as_cpp<std::string>(x["str"]) == "xyz");

    expect_true(logicals(x["lgl_vec"]) == logicals({false, false}));
    expect_true(integers(x["int_vec"]) == integers({-1, -2}));
    expect_true(doubles(x["dbl_vec"]) == doubles({-1.5, -2.5}));
    expect_true(strings(x["str_vec"]) == strings({"x", "y", "z"}));
  }
}
