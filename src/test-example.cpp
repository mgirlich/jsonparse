// All test files should include the <testthat.h>
// header file.
#include <cpp11/parse.hpp>
#include <testthat.h>

context("parse_scalar") {
  using namespace simdjson;
  ondemand::parser parser;
  auto json = R"(  {
    "lgl_true": true, "lgl_false": false, "lgl_null": null,
    "int_1": 1, "int_null": null,
    "dbl_1.5": 1.5, "dbl_null": null,
    "str_empty": "", "str_abc": "abc", "str_null": null
  }  )"_padded;
  auto doc = parser.iterate(json);

  test_that("can parse a scalar bool") {
    expect_true(parse_scalar_bool(doc["lgl_true"].value(), "") == 1);
    expect_true(parse_scalar_bool(doc["lgl_false"].value(), "") == 0);
    expect_true(parse_scalar_bool(doc["lgl_null"].value(), "") == NA_LOGICAL);
  }

  test_that("can parse a scalar integer") {
    expect_true(parse_scalar_int(doc["int_1"].value(), "") == 1);
    expect_true(parse_scalar_int(doc["int_null"].value(), "") == NA_INTEGER);
  }

  test_that("can parse a scalar double") {
    expect_true(parse_scalar_double(doc["dbl_1.5"].value(), "") == 1.5);
    expect_true(cpp11::is_na(parse_scalar_double(doc["dbl_null"].value(), "")));
  }

  test_that("can parse a scalar integer") {
    expect_true(cpp11::r_string(parse_scalar_string(doc["str_empty"].value(), "")) == "");
    expect_true(cpp11::r_string(parse_scalar_string(doc["str_abc"].value(), "")) == "abc");
    expect_true(parse_scalar_string(doc["str_null"].value(), "") == NA_STRING);
  }
}

context("parse_homo_array") {
  using namespace simdjson;
  ondemand::parser parser;
  auto json = R"(  {
    "lgl": [true, null, false],
    "int": [1, null, 2],
    "dbl": [1.5, null, -1.5],
    "str": ["", null, "abc"]
  }  )"_padded;
  auto doc = parser.iterate(json);

  test_that("can parse an array of bools") {
    cpp11::logicals x = parse_homo_array_bool(doc["lgl"].value(), "");
    expect_true(x[0] == true);
    expect_true(x[1] == NA_LOGICAL);
    expect_true(x[2] == false);
  }

  test_that("can parse an array of ints") {
    cpp11::integers x = parse_homo_array_int(doc["int"].value(), "");
    expect_true(x[0] == 1);
    expect_true(x[1] == NA_INTEGER);
    expect_true(x[2] == 2);
  }

  test_that("can parse an array of doubles") {
    cpp11::doubles x = parse_homo_array_double(doc["dbl"].value(), "");
    expect_true(x[0] == 1.5);
    expect_true(cpp11::is_na(x[1]));
    expect_true(x[2] == -1.5);
  }

  test_that("can parse an array of strings") {
    cpp11::strings x = parse_homo_array_string(doc["str"].value(), "");
    expect_true(x[0] == "");
    expect_true(x[1] == NA_STRING);
    expect_true(x[2] == "abc");
  }
}
