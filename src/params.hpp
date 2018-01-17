#pragma once

#include <string>
#include <map>
#include "sptr.hpp"

struct Texture;
struct Value;

struct Params : Object {

    static sptr<Params> create();

    virtual void insert(const std::string &k, const std::string &v) = 0;
    virtual void insert(const std::string &k, const sptr<Texture> &v) = 0;
    virtual void insert(const std::string &k, const sptr<Value> &v) = 0;

    virtual void merge(const sptr<Params> &p) = 0;
    virtual void merge(const std::map<const std::string, const std::string> &strings) = 0;
    virtual void merge(const std::map<const std::string, const sptr<Texture>> &textures) = 0;
    virtual void merge(const std::map<const std::string, const sptr<Value>> &values) = 0;

    virtual sptr<Value>   value(const std::string &k) const = 0;
    virtual std::string   string(const std::string &k) const = 0;
    virtual sptr<Texture> texture(const std::string &k) const = 0;
};
