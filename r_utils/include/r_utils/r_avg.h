
#ifndef __r_utils_r_avg_h
#define __r_utils_r_avg_h

#include <cstdlib>
#include <cstdint>
#include <cmath>

namespace r_utils
{

template<typename T>
class r_exp_avg final
{
public:
    r_exp_avg(const r_exp_avg&) = default;
    r_exp_avg(r_exp_avg&&) = default;
    r_exp_avg(const T& init_value, T n) :
        _accumulator(init_value),
        _variance_accumulator(0),
        _update_count(0),
        _alpha(2./(n+1.))
    {
    }
    ~r_exp_avg() noexcept = default;
    r_exp_avg& operator=(const r_exp_avg&) = default;
    r_exp_avg& operator=(r_exp_avg&&) = default;

    T update(T new_val)
    {
        _accumulator = (T)((_alpha * new_val) + (1.0 - _alpha) * _accumulator);
        auto delta = new_val - _accumulator;
        _variance_accumulator += (delta * delta);
        ++_update_count;
        return _accumulator;
    }

    T variance() const
    {
        return _variance_accumulator / _update_count;
    }
    
    T standard_deviation() const
    {
        return std::sqrt(variance());
    }

private:
    T _accumulator;
    T _variance_accumulator;
    uint64_t _update_count;
    double _alpha;
};

}

#endif
