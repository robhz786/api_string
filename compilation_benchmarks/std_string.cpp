 #include <string>

std::string get_str();
void set_str(const std::string& str);


void function()
{
    auto s = get_str();
    set_str(s);
}
