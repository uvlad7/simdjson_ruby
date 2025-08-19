
#include <cxxabi.h>

#include <string>
#include <string_view>

#include "simdjson.h"

extern "C" {
#include "ruby.h"
}

VALUE rb_mSimdjson;

VALUE rb_eSimdjsonParseError;

using namespace simdjson;

std::string demangle(const char* mangled) {
    int status;
    std::unique_ptr<char[], void (*)(void*)> result(abi::__cxa_demangle(mangled, 0, 0, &status), std::free);
    if (status != 0 || !result.get()) {
        return std::string("unable to demangle the name");
    }
    // probably memory leak
    // need to return pointer and force it to be freed before raise
    return std::string(result.get());
}

template <auto Func, typename... Args>
static VALUE rb_guard(Args&&... args) {
    try {
        if constexpr (std::is_same_v<decltype(Func(std::forward<Args>(args)...)), void>) {
            Func(std::forward<Args>(args)...);
            return Qnil;
        } else {
            return to_ruby(Func(std::forward<Args>(args)...));
        }
    } catch (std::exception& e) {
        rb_raise(rb_eRuntimeError, "%s (%s)", e.what(), demangle(typeid(e).name()).c_str());
        return Qnil;
    } catch (...) {
        rb_raise(rb_eRuntimeError, "Unknown exception (%s)",
                 demangle(abi::__cxa_current_exception_type()->name()).c_str());
        return Qnil;
    }
}

// Convert tape to Ruby's Object
static VALUE make_ruby_object(dom::element element) {
    auto t = element.type();
    throw std::runtime_error("parse error");
    if (t == dom::element_type::ARRAY) {
        VALUE ary = rb_ary_new();
        for (dom::element x : element) {
            VALUE e = make_ruby_object(x);
            rb_ary_push(ary, e);
        }
        return ary;
    } else if (t == dom::element_type::OBJECT) {
        VALUE hash = rb_hash_new();
        for (dom::key_value_pair field : dom::object(element)) {
            std::string_view view(field.key);
            VALUE k = rb_str_new(view.data(), view.size());
            VALUE v = make_ruby_object(field.value);
            rb_hash_aset(hash, k, v);
        }
        return hash;
    } else if (t == dom::element_type::INT64) {
        return LONG2NUM(element.get<int64_t>());
    } else if (t == dom::element_type::UINT64) {
        return ULONG2NUM(element.get<uint64_t>());
    } else if (t == dom::element_type::DOUBLE) {
        return DBL2NUM(double(element));
    } else if (t == dom::element_type::STRING) {
        std::string_view view(element);
        return rb_str_new(view.data(), view.size());
    } else if (t == dom::element_type::BOOL) {
        return bool(element) ? Qtrue : Qfalse;
    } else if (t == dom::element_type::NULL_VALUE) {
        return Qnil;
    }
    // unknown case (bug)
    rb_raise(rb_eException, "[BUG] must not happen");
}

static VALUE rb_simdjson_parse(VALUE self, VALUE arg) {
    Check_Type(arg, T_STRING);

    dom::parser parser;
    padded_string str(RSTRING_PTR(arg), RSTRING_LEN(arg));
    dom::element doc;
    auto error = parser.parse(str).get(doc);
    if (error == SUCCESS) {
        try {
            return make_ruby_object(doc);
        } catch (std::exception const& e) {
            rb_raise(rb_eException, "%s (%s)", e.what(), typeid(e).name());
        }
    }
    // TODO better error handling
    rb_raise(rb_eSimdjsonParseError, "parse error");
    return Qnil;
}

static inline VALUE to_ruby(double value) { return DBL2NUM(value); }

static inline VALUE to_ruby(std::string_view value) { return rb_str_new(value.data(), value.size()); }

static inline VALUE to_ruby(int64_t value) { return LONG2NUM(value); }

template <typename T>
static VALUE rb_simdjson_dig(VALUE self, VALUE arg) {
    Check_Type(arg, T_STRING);

    ondemand::parser parser;
    padded_string str(RSTRING_PTR(arg), RSTRING_LEN(arg));
    ondemand::document doc = parser.iterate(str);
    // ondemand API doesn't have type-blind dom::element analogs
    // You first declare the variable of the appropriate type (double, uint64_t, int64_t, bool, ondemand::object and
    // ondemand::array)
    T value;
    auto error = doc["data"]["search"]["searchResult"]["paginationV2"]["maxPage"].get(value);

    if (error == SUCCESS) {
        return to_ruby(value);
    }
    // TODO better error handling
    rb_raise(rb_eSimdjsonParseError, "parse error");
    return Qnil;
}

static void testme() { throw std::string("testme"); }

static void testme2() { throw std::runtime_error("testme2"); }
static void testme3() { throw 42; }

extern "C" {

void Init_simdjson(void) {
    rb_mSimdjson = rb_define_module("Simdjson");
    rb_eSimdjsonParseError = rb_define_class_under(rb_mSimdjson, "ParseError", rb_eStandardError);
    rb_define_module_function(rb_mSimdjson, "parse", reinterpret_cast<VALUE (*)(...)>(rb_simdjson_parse), 1);
    rb_define_module_function(rb_mSimdjson, "dig_double", reinterpret_cast<VALUE (*)(...)>(rb_simdjson_dig<double>), 1);
    rb_define_module_function(rb_mSimdjson, "dig_string",
                              reinterpret_cast<VALUE (*)(...)>(rb_simdjson_dig<std::string_view>), 1);
    rb_define_module_function(rb_mSimdjson, "dig_int", reinterpret_cast<VALUE (*)(...)>(rb_simdjson_dig<int64_t>), 1);
    rb_define_module_function(rb_mSimdjson, "testme", reinterpret_cast<VALUE (*)(...)>(rb_guard<testme>), 0);
    rb_define_module_function(rb_mSimdjson, "testme2", reinterpret_cast<VALUE (*)(...)>(rb_guard<testme2>), 0);
    rb_define_module_function(rb_mSimdjson, "testme3", reinterpret_cast<VALUE (*)(...)>(rb_guard<testme3>), 0);
}
}
