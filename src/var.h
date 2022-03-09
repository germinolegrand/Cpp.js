#pragma once

#include <unordered_map>
#include <variant>
#include <string>
#include <regex>

class undefined_value{};
class unavailable_operation{};

class var
{
public:
    var() = default;
    var(std::string const& str);
    var(char const* c_str):var(std::string(c_str)){}
    var(std::regex const& rgx);
    var(double d);
    var(bool b);
    var(std::nullptr_t);
    var(std::function<var(std::vector<var> args)>);
    var(std::unordered_map<std::string, var> properties, var prototype = nullptr);
    // Plain function pointers convert to bool if not explicitely overloaded
    template<class T, class...Args>
    var(T(*lambda)(Args...)):var(std::function<T(Args...)>(lambda)){}

    var& operator=(var const& o){ m_value = o.m_value; return *this; }

    friend var operator+(var const&, var const&);
    friend var operator-(var const&, var const&);
    friend var operator*(var const&, var const&);
    friend var operator/(var const&, var const&);
    friend var operator%(var const&, var const&);

    friend bool operator<(var const&, var const&);
    friend bool operator>(var const&, var const&);
    friend bool operator<=(var const&, var const&);
    friend bool operator>=(var const&, var const&);

    var& operator+=(var const& o){ return *this = *this + o; }
    var& operator-=(var const& o){ return *this = *this - o; }
    var& operator*=(var const& o){ return *this = *this * o; }
    var& operator/=(var const& o){ return *this = *this / o; }
    var& operator%=(var const& o){ return *this = *this % o; }
    var& operator++(){ return *this = *this + 1.; }
    var& operator--(){ return *this = *this - 1.; }
    var operator++(int){ auto old = *this; ++(*this); return old; }
    var operator--(int){ auto old = *this; --(*this); return old; }

    bool is_undefined() const;
    bool is_null() const;
    bool is_callable() const;

    std::string to_string() const;
    std::regex to_regex() const;
    double to_double() const;
    bool to_bool() const;

    var operator()(std::vector<var> args = {});
    var& operator[](var property);
    var& operator[](char const* property){ return operator[](var(property)); };
    var const& operator[](var property) const;
    var const& operator[](char const* property) const { return operator[](var(property)); };

    static const var undefined;

private:
    using function_t = std::function<var(std::vector<var> args)>;

    template<class T>
    struct objectT
    {
        T prototype;
        std::unordered_map<std::string, T> properties;
    };
    using object_t = objectT<var>;

    using var_t = std::variant<
        std::nullptr_t,
        bool,
        double,
        std::string,
        std::regex,
        function_t,
        object_t>;

    std::shared_ptr<var_t> m_value;

    static var* findProperty(object_t& obj, std::string const& propertyName);
};

inline std::ostream& operator<<(std::ostream& os, var const& v)
{
    return os << v.to_string();
}

inline bool operator==(var const& a, std::string const& b){
    return a.to_string() == b;
}

inline bool operator==(var const& a, double const& b){
    return a.to_double() == b;
}

inline bool operator==(var const& a, var const& b){
    return a.to_string() == b.to_string();
}

var operator+(var const&, var const&);
var operator-(var const&, var const&);
var operator*(var const&, var const&);
var operator/(var const&, var const&);
var operator%(var const&, var const&);

bool operator<(var const&, var const&);
inline bool operator>(var const& a, var const& b){ return b < a; }
bool operator<=(var const& a, var const& b);
bool operator>=(var const& a, var const& b);
