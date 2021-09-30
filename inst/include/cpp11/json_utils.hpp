#pragma once

#define STRICT_R_HEADERS
#include <cpp11.hpp>

#include "cpp11/simdjson.h"
#include "json_path.hpp"

inline std::string json_type_to_string(simdjson::ondemand::value element) {
  using simdjson::ondemand::json_type;
  switch (element.type()) {
  case json_type::null:
    return "null";
    break;
  case json_type::array:
    return "array";
    break;
  case json_type::boolean:
    return "boolean";
    break;
  case json_type::number:
    return "number";
    break;
  case json_type::object:
    return "object";
    break;
  case json_type::string:
    return "string";
    break;
  }
}

inline simdjson::ondemand::array safe_get_array(simdjson::ondemand::value json, JSON_Path& path) {
  simdjson::ondemand::array array;
  auto error = json.get_array().get(array);
  if (error) {
    cpp11::stop("Element at path " + path.path() + " is not an array.");
  }

  return array;
}

inline simdjson::ondemand::object safe_get_object(simdjson::ondemand::value json, JSON_Path& path) {
  simdjson::ondemand::object object;
  auto error = json.get_object().get(object);
  if (error) {
    cpp11::stop("Element at path " + path.path() + " is not an object.");
  }

  return object;
}

inline std::string safe_get_key(simdjson::simdjson_result<simdjson::ondemand::field> field) {
  std::string_view key_v;
  auto error = field.unescaped_key().get(key_v);
  if (error) {
    // TODO when could this actually happen??
    cpp11::stop("Something went wrong with the key");
  }

  return std::string(key_v);
}
