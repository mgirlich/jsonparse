#pragma once

#include <vector>
#include <string>

class JSON_Path_Element_Base {
public:
  virtual ~JSON_Path_Element_Base() {};

  virtual std::string to_path() const = 0;
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

  std::string to_path() const {
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

  std::string to_path() const {
    return "/" + key;
  }
};

template <>
class JSON_Path_Element<std::string_view> : public virtual JSON_Path_Element_Base {
private:
  std::string_view key;
public:
  JSON_Path_Element<std::string_view>(const std::string_view & key) {
    this->key = key;
  }

  std::string to_path() const {
    return "/" + std::string(key);
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

  void insert(const std::string_view key) {
    path_elements.push_back(std::make_unique<JSON_Path_Element<std::string_view>>(key));
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

  void replace(const std::string_view& key) {
    path_elements.back() = std::make_unique<JSON_Path_Element<std::string_view>>(key);
  }

  void drop() {
    path_elements.pop_back();
  }

  std::string path() const {
    std::string out = "";
    for (auto & it : path_elements) {
      out += (*it).to_path();
    }

    return out;
  }
};
