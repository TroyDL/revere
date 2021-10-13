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

    ~raii_ptr()
    {
        if(_thing)
            _dtor(_thing);
    }

    raii_ptr& operator=(T* thing)
    {
        if(_thing)
            _dtor(_thing);

        _thing = thing;

        return *this;
    }

    raii_ptr& operator=(raii_ptr&& obj) noexcept
    {
        _thing = std::move(obj._thing);
        obj._thing = nullptr;
        _dtor = std::move(obj._dtor);

        return *this;
    }    

    T* get() const {return _thing;}
    T*& raw() {return _thing;}

    operator bool() const {return _thing != nullptr;}

private:
    T* _thing;
    std::function<void(T*)> _dtor;
};

std::string get_env(const std::string& name);

}

}

#endif
