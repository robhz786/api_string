#include <string_view>

std::string_view get_str();
void set_str(std::string_view str);


void function()
{
    auto s = get_str();
    set_str(s);
}
