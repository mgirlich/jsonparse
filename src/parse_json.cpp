#define STRICT_R_HEADERS
#include "cpp11.hpp"
#include "cpp11/R.hpp"

#if __cplusplus >= 201703L
#include <cpp11/parser_class.hpp>
#endif


[[cpp11::register]]
cpp11::sexp parse_json(cpp11::r_string json, cpp11::list spec) {
    return R_NilValue;
}
