 #include <api_string.hpp>

speudo_std::api_string get_str();
void set_str(const speudo_std::api_string& str);


void function()
{
    auto s = get_str();
    set_str(s);
}
