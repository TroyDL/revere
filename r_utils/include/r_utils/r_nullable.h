
#ifndef r_utils_r_nullable_h
#define r_utils_r_nullable_h

#include "r_utils/r_exception.h"

namespace r_utils
{

/// Allows for a nullable value on the stack.
template<typename T>
class r_nullable
{
public:
    r_nullable() :
        _value(),
        _is_null( true )
    {
    }

    r_nullable(const r_nullable& obj) :
        _value(obj._value),
        _is_null(obj._is_null)
    {
    }

    r_nullable(r_nullable&& obj) noexcept :
        _value(std::move(obj._value)),
        _is_null(std::move(obj._is_null))
    {
        obj._value = T();
        obj._is_null = true;
    }

    r_nullable( T value ) :
        _value( value ),
        _is_null( false )
    {
    }

    ~r_nullable() noexcept {}

    r_nullable& operator = ( const r_nullable& rhs )
    {
        this->_value = rhs._value;
        this->_is_null = rhs._is_null;
        return *this;
    }

    r_nullable& operator = (r_nullable&& rhs) noexcept
    {
        _value = std::move(rhs._value);
        _is_null = std::move(rhs._is_null);
        rhs._value = T();
        rhs._is_null = true;
        return *this;
    }

    r_nullable& operator = ( T rhs )
    {
        this->_value = rhs;
        this->_is_null = false;
        return *this;
    }

    operator bool() const
    {
        return !_is_null;
    }

    T& raw()
    {
        return _value;
    }

    const T& value() const
    {
        return _value;
    }

    void set_value( T value )
    {
        _value = value;
        _is_null = false;
    }

    T take()
    {
        _is_null = true;
        return std::move(_value);
    }

    void assign( T&& value )
    {
        _value = std::move(value);
        _is_null = false;
    }

    bool is_null() const
    {
        return _is_null;
    }

    void clear()
    {
        _value = T();
        _is_null = true;
    }

    friend bool operator == ( const r_nullable& lhs, const r_nullable& rhs )
    {
        return( lhs._is_null && rhs._is_null ) || (lhs._value == rhs._value);
    }

    friend bool operator == ( const r_nullable& lhs, const T& rhs )
    {
        return !lhs._is_null && lhs._value == rhs;
    }

    friend bool operator == ( const T& lhs, const r_nullable& rhs )
    {
        return rhs == lhs;
    }

    friend bool operator != ( const r_nullable& lhs, const r_nullable& rhs )
    {
        return !(lhs == rhs);
    }

    friend bool operator != ( const r_nullable& lhs, const T& rhs )
    {
        return !(lhs == rhs);
    }

    friend bool operator != ( const T& lhs, const r_nullable& rhs )
    {
        return !(lhs == rhs);
    }

private:
    T _value;
    bool _is_null;
};

template<typename T>
bool operator == ( const T& lhs, const r_nullable<T>& rhs )
{
    return rhs == lhs;
}

template<typename T>
bool operator != ( const T& lhs, const r_nullable<T>& rhs )
{
    return rhs != lhs;
}

}

#endif
