// All test files should include the <testthat.h>
// header file.
#include <cpp11/utils.hpp>
#include <testthat.h>

context("new_named_list") {
  test_that("can create a named list") {
    using namespace cpp11::literals;

    SEXP list = new_named_list(std::vector<std::string>({"a", "b"}));
    cpp11::list expected = cpp11::list({"a"_nm = R_NilValue, "b"_nm = R_NilValue});

    expect_true(cpp11::list(list) == expected);
  }
}

context("new_df") {
  test_that("can create a data frame") {
    using namespace cpp11::literals;

    SEXP df = new_df(std::vector<std::string>({"a", "b"}), 2);
    auto x = cpp11::logicals({true, false});
    SET_VECTOR_ELT(df, 0, x);
    SET_VECTOR_ELT(df, 1, x);
    cpp11::data_frame expected = cpp11::data_frame({"a"_nm = x, "b"_nm = x});

    expect_true(cpp11::list(df) == expected);
  }
}

context("name_to_index") {
  test_that("can find name") {
    auto haystack = std::vector<std::string>({"a", "b"});

    expect_true(name_to_index(haystack, "a") == 0);
    expect_true(name_to_index(haystack, "b") == 1);
    expect_error(name_to_index(haystack, "c"));
  }
}
