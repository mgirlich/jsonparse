#pragma once

#include <vector>
#include <string>

class JSON_Path_Element_Base {
public:
  virtual ~JSON_Path_Element_Base() {};

  virtual const std::string to_path() = 0;
};

template <typename T>
class JSON_Path_Element : public virtual JSON_Path_Element_Base {};

template <>
class JSON_Path_Element<int> : public virtual JSON_Path_Element_Base {
private:
  int index;
public:
  JSON_Path_Element<int>(const int & index) {
    this->index = index;
  }

  const std::string to_path() {
    return "[" + std::to_string(this->index) + "]";
  }
};

template <>
class JSON_Path_Element<std::string> : public virtual JSON_Path_Element_Base {
private:
  std::string key;
public:
  JSON_Path_Element<std::string>(const std::string & key) {
    this->key = key;
  }

  const std::string to_path() {
    return "/" + key;
  }
};

class JSON_Path {
private:
  std::vector<std::unique_ptr<JSON_Path_Element_Base>> path_elements;
public:
  void insert(int index) {
    path_elements.push_back(std::make_unique<JSON_Path_Element<int>>(index));
  }

  void insert(const std::string& key) {
    path_elements.push_back(std::make_unique<JSON_Path_Element<std::string>>(key));
  }

  void insert_dummy() {
    this->insert(-1);
  }

  void replace(int index) {
    path_elements.back() = std::make_unique<JSON_Path_Element<int>>(index);
  }

  void replace(const std::string& key) {
    path_elements.back() = std::make_unique<JSON_Path_Element<std::string>>(key);
  }

  void drop() {
    path_elements.pop_back();
  }

  const std::string path() {
    std::string out = "";
    for (auto & it : path_elements) {
      out += (*it).to_path();
    }

    return out;
  }
};
