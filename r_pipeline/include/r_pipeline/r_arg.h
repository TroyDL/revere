#ifndef r_pipeline_r_arg_h
#define r_pipeline_r_arg_h

#include "r_utils/r_nullable.h"
#include <string>
#include <vector>

namespace r_pipeline
{

class r_arg final
{
public:
    r_arg();
    r_arg(const std::string& name, const std::string& value);
    r_arg(const r_arg& obj);

    ~r_arg() noexcept;

    r_arg& operator=(const r_arg& obj);

    void set_name(const std::string& name);
    std::string get_name() const;

    bool has_value() const;
    void set_value(const std::string& value);
    r_utils::r_nullable<std::string> get_value() const;

private:
    std::string _name;
    r_utils::r_nullable<std::string> _value;
};

void add_argument(std::vector<r_arg>& arguments, const std::string& name, const std::string& value);

}

#endif
