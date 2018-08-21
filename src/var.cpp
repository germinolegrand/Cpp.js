#include "var.h"

#include <fys/utility/is.h>

#include <cassert>
#include <cmath>
#include <iomanip>

#define UNIMPLEMENTED assert(0 && "UNIMPLEMENTED"); throw unavailable_operation();

var::var(std::string const& str):
    m_value(std::make_shared<var_t>(str))
{}

var::var(std::regex const& rgx):
    m_value(std::make_shared<var_t>(rgx))
{}

var::var(double d):
    m_value(std::make_shared<var_t>(d))
{}

var::var(bool b):
    m_value(std::make_shared<var_t>(b))
{}

var::var(std::nullptr_t):
    m_value(std::make_shared<var_t>(nullptr))
{}

var::var(function_t f):
    m_value(std::make_shared<var_t>(std::move(f)))
{}

var::var(object_t p):
    m_value(std::make_shared<var_t>(std::move(p)))
{}


bool var::is_undefined() const
{
    return !m_value;
}

bool var::is_null() const
{
    return m_value && std::holds_alternative<bool>(*m_value);
}

bool var::is_callable() const
{
    return m_value && std::holds_alternative<function_t>(*m_value);
}

///Conversions

std::string var::to_string() const
{
    if(!m_value)
        return "undefined";

    return std::visit([](auto&& arg) -> std::string{
        if constexpr(fys::is<std::nullptr_t>(arg)){
            return std::string("null");
        } else if constexpr(fys::is<bool>(arg)){
            return arg ? "true" : "false";
        } else if constexpr(fys::is<double>(arg)){
            std::stringstream strstr;
            strstr << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10 + 1) << arg;
            return strstr.str();
        } else if constexpr(fys::is<std::string>(arg)){
            return arg;
        } else if constexpr(fys::is<std::regex>(arg)){
            return "regex";
        } else if constexpr(fys::is<function_t>(arg)){
            return "function";
        } else if constexpr(fys::is<object_t>(arg)){
            std::stringstream strstr;
            strstr << '{';
            for(auto& [key, value]: arg){
                strstr << std::quoted(key);
                strstr << ':';
                if(value.m_value && std::holds_alternative<std::string>(*value.m_value)){
                    strstr << std::quoted(value.to_string());
                } else {
                    strstr << value.to_string();
                }
                strstr << ',';
            }
            auto str = strstr.str();
            if(str.size() > 1){
                str.back() = '}';
            } else {
                str += '}';
            }
            return str;
        } else throw unavailable_operation();
    }, *m_value);
}

std::regex var::to_regex() const
{
    UNIMPLEMENTED
}

double var::to_double() const
{
    if(!m_value)
        return 0;

    return std::visit([](auto&& arg) -> double{
        if constexpr(fys::is<std::nullptr_t>(arg)){
            return 0;
        } else if constexpr(fys::is<bool>(arg)){
            return arg;
        } else if constexpr(fys::is<double>(arg)){
            return arg;
        } else if constexpr(fys::is<std::string>(arg)){
            return std::stod(arg);
        } else if constexpr(fys::is<object_t>(arg)){
            if(auto it = arg.find("to_double");
                it != end(arg) && it->second.is_callable()){
                return (it->second)().to_double();
            }
            return 0;
        } else throw unavailable_operation();
    }, *m_value);
}

bool var::to_bool() const
{
    if(!m_value)
        return false;

    return std::visit([](auto&& arg) -> bool {
        if constexpr(fys::is<std::nullptr_t>(arg)){
            return false;
        } else if constexpr(fys::is<bool>(arg)){
            return arg;
        } else if constexpr(fys::is<double>(arg)){
            return arg != 0. && !std::isnan(arg);
        } else if constexpr(fys::is<std::string>(arg)){
            return !arg.empty();
        } else if constexpr(fys::is<object_t>(arg)){
            if(auto it = arg.find("to_bool");
                it != end(arg) && it->second.is_callable()){
                return (it->second)().to_bool();
            }
            return true;
        } else throw unavailable_operation();
    }, *m_value);
}

var var::operator()(std::vector<var> args)
{
    if(!m_value)
        throw undefined_value();

    if(auto func = std::get_if<function_t>(&*m_value); func){
        return (*func)(std::move(args));
    }

    if(auto obj = std::get_if<object_t>(&*m_value); obj){
        if(auto it = obj->find("operator()");
            it != end(*obj) && it->second.is_callable()){
            return it->second(std::move(args));
        }
    }

    throw unavailable_operation();
}

var& var::operator[](var property)
{
    if(!m_value)
        throw undefined_value();

    auto obj = std::get_if<object_t>(&*m_value);

    if(!obj)
        throw unavailable_operation();

    auto prop_str = property.to_string();

    if(auto it = obj->find(prop_str);
        it != end(*obj)){
        return it->second;
    }

    return obj->emplace(std::move(prop_str), var{}).first->second;
}

var const& var::operator[](var property) const
{
    if(!m_value)
        throw undefined_value();

    auto obj = std::get_if<object_t>(&*m_value);

    if(!obj)
        throw unavailable_operation();

    auto prop_str = property.to_string();

    if(auto it = obj->find(prop_str);
        it != end(*obj)){
        return it->second;
    }

    static const var undefined{};
    return undefined;
}


var operator+(var const& leftHS, var const& rightHS){
    if(std::holds_alternative<std::string>(*leftHS.m_value)
       || std::holds_alternative<std::string>(*rightHS.m_value)){
        return leftHS.to_string() + rightHS.to_string();
    }
    return leftHS.to_double() + rightHS.to_double();
}
