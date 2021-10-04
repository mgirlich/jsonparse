#include <cpp11/column_class.hpp>

template <typename T>
T parse_default_value(cpp11::sexp default_sexp) {
    return T(cpp11::r_vector<T>(default_sexp)[0]);
}

std::pair<std::unordered_map<std::string, std::unique_ptr<Column>>, std::vector<std::string>> parse_sub_spec(cpp11::list spec) {
    std::unordered_map<std::string, std::unique_ptr<Column>> fields;
    std::vector<std::string> col_order;

    for (cpp11::list element : spec) {
        std::string type = cpp11::r_string(cpp11::strings(element["type"])[0]);
        std::string key = cpp11::r_string(cpp11::strings(element["path"])[0]);
        // cpp11::message(type);
        // cpp11::message(key);
        col_order.push_back(key);
        cpp11::sexp default_sexp = element["default"];

        if (type == "lgl") {
            cpp11::r_bool default_val = parse_default_value<cpp11::r_bool>(default_sexp);
            fields[key] = std::make_unique<Column_Scalar<bool>>(default_val);
        } else if (type == "int") {
            auto default_val = parse_default_value<int>(default_sexp);
            fields[key] = std::make_unique<Column_Scalar<int>>(default_val);
        } else if (type == "dbl") {
            auto default_val = parse_default_value<double>(default_sexp);;
            fields[key] = std::make_unique<Column_Scalar<double>>(default_val);
        } else if (type == "str") {
            auto default_val = parse_default_value<cpp11::r_string>(default_sexp);
            fields[key] = std::make_unique<Column_Scalar<std::string>>(default_val);
        } else if (type == "lgl_vec") {
            if (Rf_isNull(default_sexp)) {
                fields[key] = std::make_unique<Column_Vector<bool>>(cpp11::list());
            } else {
                auto default_val = cpp11::logicals(default_sexp);
                fields[key] = std::make_unique<Column_Vector<bool>>(default_val);
            }
        } else if (type == "int_vec") {
            if (Rf_isNull(default_sexp)) {
                fields[key] = std::make_unique<Column_Vector<int>>(cpp11::list());
            } else {
                auto default_val = cpp11::integers(default_sexp);
                fields[key] = std::make_unique<Column_Vector<int>>(default_val);
            }
        } else if (type == "dbl_vec") {
            if (Rf_isNull(default_sexp)) {
                fields[key] = std::make_unique<Column_Vector<double>>(cpp11::list());
            } else {
                auto default_val = cpp11::doubles(default_sexp);
                fields[key] = std::make_unique<Column_Vector<double>>(default_val);
            }
        } else if (type == "str_vec") {
            if (Rf_isNull(default_sexp)) {
                fields[key] = std::make_unique<Column_Vector<std::string>>(cpp11::list());
            } else {
                auto default_val = cpp11::strings(default_sexp);
                fields[key] = std::make_unique<Column_Vector<std::string>>(default_val);
            }
        } else if (type == "df") {
            auto spec_info = parse_sub_spec(element["fields"]);
            fields[key] = std::make_unique<Column_Df>(spec_info.first, spec_info.second);
        } else {
            cpp11::message(type);
            cpp11::stop("Unsupported type!");
        }
    }

    return std::make_pair(std::move(fields), col_order);
}

Parser_Dataframe parse_spec_collector_df(cpp11::list spec) {
    auto spec_info = parse_sub_spec(spec);
    return Parser_Dataframe(spec_info.first, spec_info.second);
}

Parser_Object parse_spec_collector_object(cpp11::list spec) {
    std::unordered_map<std::string, std::unique_ptr<Parser>> fields;
    std::unordered_map<std::string, SEXP> default_values;
    std::vector<std::string> nms;

    for (cpp11::list element : spec) {
        std::string key = cpp11::r_string(cpp11::strings(element["path"])[0]);
        std::string type = cpp11::r_string(cpp11::strings(element["type"])[0]);
        // TODO must adapt to name
        nms.push_back(key);
        cpp11::sexp default_sexp = element["default"];

        if (type == "lgl") {
            fields[key] = std::make_unique<Parser_Scalar<bool>>();
            default_values[key] = cpp11::as_sexp(parse_default_value<cpp11::r_bool>(default_sexp));
        } else if (type == "int") {
            fields[key] = std::make_unique<Parser_Scalar<int>>();
            default_values[key] = cpp11::as_sexp(parse_default_value<int>(default_sexp));
        } else if (type == "dbl") {
            fields[key] = std::make_unique<Parser_Scalar<double>>();
            default_values[key] = cpp11::as_sexp(parse_default_value<double>(default_sexp));
        } else if (type == "str") {
            fields[key] = std::make_unique<Parser_Scalar<std::string>>();
            default_values[key] = cpp11::as_sexp(parse_default_value<cpp11::r_string>(default_sexp));
        } else if (type == "lgl_vec") {
            fields[key] = std::make_unique<Parser_HomoArray<bool>>();
            default_values[key] = cpp11::logicals(default_sexp);
        } else if (type == "int_vec") {
            fields[key] = std::make_unique<Parser_HomoArray<int>>();
            default_values[key] = cpp11::integers(default_sexp);
        } else if (type == "dbl_vec") {
            fields[key] = std::make_unique<Parser_HomoArray<double>>();
            default_values[key] = cpp11::doubles(default_sexp);
        } else if (type == "str_vec") {
            fields[key] = std::make_unique<Parser_HomoArray<std::string>>();
            default_values[key] = cpp11::strings(default_sexp);
        } else if (type == "list") {
            fields[key] = std::make_unique<Parser_Object>(parse_spec_collector_object(element["fields"]));
            default_values[key] = default_sexp;
        } else if (type == "df") {
            fields[key] = std::make_unique<Parser_Dataframe>(parse_spec_collector_df(element["fields"]));
            default_values[key] = default_sexp;
        } else {
            cpp11::stop("Unsupported type!");
        }
    }

    return Parser_Object(fields, default_values, nms);
}

std::unique_ptr<Parser> parse_spec(cpp11::list element) {
    std::string type = cpp11::r_string(cpp11::strings(element["type"])[0]);

    if (type == "lgl_vec") {
        return std::make_unique<Parser_HomoArray<bool>>();
    } else if (type == "int_vec") {
        return std::make_unique<Parser_HomoArray<int>>();
    } else if (type == "dbl_vec") {
        return std::make_unique<Parser_HomoArray<double>>();
    } else if (type == "str_vec") {
        return std::make_unique<Parser_HomoArray<std::string>>();
    } else if (type == "list") {
        return std::make_unique<Parser_Object>(parse_spec_collector_object(element["fields"]));
    } else if (type == "df") {
        return std::make_unique<Parser_Dataframe>(parse_spec_collector_df(element["fields"]));
    } else {
        Rprintf(type.c_str());
        cpp11::stop("Unsupported type!");
    }
}
