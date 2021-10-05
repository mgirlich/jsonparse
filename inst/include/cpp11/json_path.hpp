#pragma once

#include <vector>
#include <string>

enum PathType {str, ind};

class JSON_Path_Element {
private:
  int index;
  std::string_view key;
  PathType type;

public:
  JSON_Path_Element(const int & index) {
    this->index = index;
    this->type = PathType::ind;
  }

  JSON_Path_Element(const std::string_view & key) {
    this->key = key;
    this->type = PathType::str;
  }

  void update(const int & index) {
    this->index = index;
    // this->type = PathType::ind;
  }

  void update(const std::string_view & key) {
    this->key = key;
    // this->type = PathType::str;
  }

  std::string to_path() const {
    switch (this->type) {
    case PathType::ind:
      return "[" + std::to_string(this->index) + "]";
      break;
    case PathType::str:
      return "/" + std::string(this->key);
      break;
    }
  }
};

class JSON_Path {
private:
  std::vector<JSON_Path_Element> path_elements;
  int level = 0;
public:
  void insert(int index) {
    path_elements.push_back(JSON_Path_Element(index));
  }

  void insert(const std::string_view & key) {
    path_elements.push_back(JSON_Path_Element(key));
  }

  template <typename T>
  void insert_dummy();

  template<>
  void insert_dummy<int>() {
    this->insert(-1);
  }

  template<>
  void insert_dummy<std::string_view>() {
    this->insert(std::string_view(""));
  }

  void replace(int index) {
    path_elements.back().update(index);
  }

  void replace(const std::string_view& key) {
    path_elements.back().update(key);
  }

  void drop() {
    path_elements.pop_back();
  }

  std::string path() const {
    std::string out = "";
    for (auto & it : path_elements) {
      out += it.to_path();
    }

    return out;
  }
};
