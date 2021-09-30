#define STRICT_R_HEADERS
#include "cpp11.hpp"
#include "cpp11/R.hpp"

#if __cplusplus >= 201703L
#include "cpp11/simdjson.cpp"
#include "cpp11/simdjson.h"
#include <cpp11/parser_class.hpp>
#endif


[[cpp11::register]]
cpp11::sexp parse_json(cpp11::r_string json, cpp11::list spec) {
  using namespace simdjson;
  ondemand::parser parser;

  auto json2 = R"(  {
    "lgl_true": true, "lgl_false": false, "lgl_null": null,
    "int_1": 1, "int_null": null,
    "dbl_1.5": 1.5, "dbl_null": null,
    "str_empty": "", "str_abc": "abc", "str_null": null
  }  )"_padded;
  auto doc = parser.iterate(json);

  auto parser_lgl = Parser_Scalar<bool>();
  auto path = JSON_Path();

  return parser_lgl.parse_json(doc["lgl_true"].value(), path);
}
