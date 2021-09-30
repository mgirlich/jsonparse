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
