#define STRICT_R_HEADERS
#include "cpp11.hpp"
#include "cpp11/R.hpp"

#include <vector>

SEXP new_named_list(std::vector<std::string> nms) {
    int n_fields = nms.size();
    SEXP out = PROTECT(Rf_allocVector(VECSXP, n_fields));

    SEXP nms_sexp = PROTECT(Rf_allocVector(STRSXP, n_fields));
    int i = 0;
    for (auto nm : nms) {
        SET_STRING_ELT(nms_sexp, i, Rf_mkChar(nm.c_str()));
        i++;
    }
    Rf_setAttrib(out, Rf_install("names"), nms_sexp);
    UNPROTECT(1);

    UNPROTECT(1);
    return out;
}

SEXP new_df(std::vector<std::string> col_nms, int n_rows) {
    SEXP out = new_named_list(col_nms);

    // add row.names attribute
    SEXP row_attr = PROTECT(Rf_allocVector(INTSXP, 2));
    SET_INTEGER_ELT(row_attr, 0, NA_INTEGER);
    SET_INTEGER_ELT(row_attr, 1, -n_rows);
    Rf_setAttrib(out, Rf_install("row.names"), row_attr);
    UNPROTECT(1);

    // add class
    SEXP class_attr = PROTECT(Rf_allocVector(STRSXP, 3));
    SET_STRING_ELT(class_attr, 0, Rf_mkChar("tbl_df"));
    SET_STRING_ELT(class_attr, 1, Rf_mkChar("tbl"));
    SET_STRING_ELT(class_attr, 2, Rf_mkChar("data.frame"));
    Rf_setAttrib(out, Rf_install("class"), class_attr);
    UNPROTECT(1);

    return out;
}

int name_to_index(std::vector<std::string> haystack, std::string needle) {
    int index = 0;
    for (auto hay : haystack) {
        if (hay == needle) {
            return index;
        }
        index++;
    }

    cpp11::stop("name_to_index(): needle not found in haystack");
}
