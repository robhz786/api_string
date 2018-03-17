#include <private/api_string_memory.hpp>
#include <string> // char_traits
#include <stdexcept>
	
namespace speudo_std {
namespace private_ {

void throw_std_out_of_range(const char* msg)
{
    throw std::out_of_range(msg);
}


std::size_t str_length(const char* str)
{
    return std::char_traits<char>::length(str);
}
std::size_t str_length(const wchar_t* str)
{
    return std::char_traits<wchar_t>::length(str);
}
std::size_t str_length(const char16_t* str)
{
    return std::char_traits<char16_t>::length(str);
}
std::size_t str_length(const char32_t* str)
{
    return std::char_traits<char32_t>::length(str);
}


template <typename CharT>
inline int do_compare
    ( const CharT* lhs
    , std::size_t lhs_len
    , const CharT* rhs
    , std::size_t rhs_len)
{
    std::size_t min_len = std::min(lhs_len, rhs_len);
    int cmp = std::char_traits<CharT>::compare(lhs, rhs, min_len);
    return cmp != 0 ? cmp : (lhs_len == rhs_len ? 0 : (lhs_len < rhs_len? -1 : +1));
}

int str_compare
    ( const char* lhs
    , std::size_t lhs_len
    , const char* rhs
    , std::size_t rhs_len)
{
    return do_compare(lhs, lhs_len, rhs, rhs_len);
}

int str_compare
    ( const wchar_t* lhs
    , std::size_t lhs_len
    , const wchar_t* rhs
    , std::size_t rhs_len)
{
    return do_compare(lhs, lhs_len, rhs, rhs_len);
}

int str_compare
    ( const char16_t* lhs
    , std::size_t lhs_len
    , const char16_t* rhs
    , std::size_t rhs_len)
{
    return do_compare(lhs, lhs_len, rhs, rhs_len);
}

int str_compare
    ( const char32_t* lhs
    , std::size_t lhs_len
    , const char32_t* rhs
    , std::size_t rhs_len)
{
    return do_compare(lhs, lhs_len, rhs, rhs_len);
}


template class api_string_mem<std::allocator<char>>;
template class api_string_mem<std::allocator<char16_t>>;
template class api_string_mem<std::allocator<char32_t>>;
template class api_string_mem<std::allocator<wchar_t>>;


void api_string_init
    ( speudo_std::abi::api_string_data<char>& data
    , const char* src
    , std::size_t count)
{
    speudo_std::private_::api_string_init_impl(data, src, count, std::allocator<char>{});
}

void api_string_init
    ( speudo_std::abi::api_string_data<char16_t>& data
    , const char16_t* src
    , std::size_t count)
{
    speudo_std::private_::api_string_init_impl(data, src, count);
}

void api_string_init
    ( speudo_std::abi::api_string_data<char32_t>& data
    , const char32_t* src
    , std::size_t count)
{
    speudo_std::private_::api_string_init_impl(data, src, count);
}

void api_string_init
    ( speudo_std::abi::api_string_data<wchar_t>&  data
    , const wchar_t*  src
    , std::size_t count)
{
    speudo_std::private_::api_string_init_impl(data, src, count);
}


} // namespace speudo_std
} // namespace private_
