// Generated by cpp11: do not edit by hand
// clang-format off


#include "cpp11/declarations.hpp"
#include <R_ext/Visibility.h>

// parse_json.cpp
cpp11::sexp parse_json(cpp11::strings json, cpp11::list spec);
extern "C" SEXP _jsonparse_parse_json(SEXP json, SEXP spec) {
  BEGIN_CPP11
    return cpp11::as_sexp(parse_json(cpp11::as_cpp<cpp11::decay_t<cpp11::strings>>(json), cpp11::as_cpp<cpp11::decay_t<cpp11::list>>(spec)));
  END_CPP11
}

extern "C" {
static const R_CallMethodDef CallEntries[] = {
    {"_jsonparse_parse_json", (DL_FUNC) &_jsonparse_parse_json, 2},
    {NULL, NULL, 0}
};
}

extern "C" attribute_visible void R_init_jsonparse(DllInfo* dll){
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
  R_forceSymbols(dll, TRUE);
}
