#include <string.hpp>

speudo_std::string get_str();
void set_str(const speudo_std::string& str);


void function()
{
    auto s = get_str();
    set_str(s);
}
