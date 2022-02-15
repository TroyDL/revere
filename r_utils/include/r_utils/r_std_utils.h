#ifndef r_utils_r_std_utils_h
#define r_utils_r_std_utils_h

#include <memory>
#include <functional>
#include <string>

namespace r_utils
{
namespace r_std_utils
{
template<typename T>
class raii_ptr
{
public:
    raii_ptr() :
        _thing(nullptr),
        _dtor()
    {
    }
    raii_ptr(std::function<void(T*)> dtor) :
       _thing(nullptr),
       _dtor(dtor)
    {
    }

    raii_ptr(T* thing, std::function<void(T*)> dtor) :
        _thing(thing),
        _dtor(dtor)
    {
    }

    raii_ptr(raii_ptr&& obj) noexcept :
        _thing(std::move(obj._thing)),
        _dtor(std::move(obj._dtor))
    {
        obj._thing = nullptr;
    }

    ~raii_ptr() noexcept
    {
        _clear();
    }

    raii_ptr& operator=(T* thing)
    {
        _clear();

        _thing = thing;

        return *this;
    }

    raii_ptr& operator=(raii_ptr&& obj) noexcept
    {
        if (this != &obj)
        {
            _clear();

            _thing = std::move(obj._thing);
            obj._thing = nullptr;
            _dtor = std::move(obj._dtor);
            obj._dtor = nullptr;
        }

        return *this;
    }    

    T* get() const {return _thing;}
    T*& raw() {return _thing;}

    operator bool() const {return _thing != nullptr;}

    void reset() noexcept
    {
        _clear();
    }

private:
    void _clear() noexcept
    {
        if(_thing)
        {
            _dtor(_thing);
            _thing = nullptr;
        }
    }
    T* _thing;
    std::function<void(T*)> _dtor;
};

std::string get_env(const std::string& name);

}

}

#endif
