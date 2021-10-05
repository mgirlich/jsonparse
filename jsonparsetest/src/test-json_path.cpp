// All test files should include the <testthat.h>
// header file.
#include <cpp11/json_path.hpp>
#include <testthat.h>

context("json_path") {
  test_that("can create a path") {
    auto path = JSON_Path();
    path.insert(std::string_view("a"));
    expect_true(path.path() == "/a");

    path.insert(1);
    expect_true(path.path() == "/a[1]");

    path.replace(2);
    expect_true(path.path() == "/a[2]");

    path.drop();
    expect_true(path.path() == "/a");
  }
}
