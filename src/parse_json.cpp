#define STRICT_R_HEADERS
#include "cpp11.hpp"
#include "cpp11/R.hpp"

#if __cplusplus >= 201703L
#include "cpp11/simdjson.cpp"
#include "cpp11/simdjson.h"
#include <cpp11/parse_spec.hpp>
#endif


[[cpp11::register]]
cpp11::sexp parse_json(cpp11::strings json, cpp11::list spec) {
  cpp11::strings json_strings = cpp11::strings(json);
  cpp11::list spec_list = cpp11::list(spec);
  simdjson::ondemand::parser parser;
  simdjson::padded_string content = simdjson::padded_string(std::string_view(std::string(json_strings[0])));
  simdjson::ondemand::document doc = parser.iterate(content);

  simdjson::ondemand::value value = doc;

  auto collector_ptr = parse_spec(spec_list);
  auto path = JSON_Path();
  auto parsed = (*collector_ptr).parse_json(value, path);

  return parsed;
}
