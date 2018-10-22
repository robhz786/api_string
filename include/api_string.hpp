#ifndef SPEUDO_STD_API_STRING_HPP
#define SPEUDO_STD_API_STRING_HPP

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#include <cstddef>

namespace speudo_std {

#if defined(API_STRING_TEST_MODE)

namespace api_string_test {
std::size_t allocations_count();
std::size_t deallocations_count();
void reset();
} // namespace api_string_test

#endif // defined(API_STRING_TEST_MODE)

namespace abi {

struct api_string_mem_base;

struct api_string_func_table
{
    typedef void(*func_void)(speudo_std::abi::api_string_mem_base*);
    typedef bool(*func_bool)(speudo_std::abi::api_string_mem_base*);
    typedef std::size_t(*func_size)(speudo_std::abi::api_string_mem_base*);

    unsigned long abi_version = 0;
    func_void adquire = nullptr;
    func_void release = nullptr;
    func_bool unique = nullptr;
    func_size bytes_capacity = nullptr;
};

struct api_string_mem_base
{
    const speudo_std::abi::api_string_func_table* const func_table;

    void adquire() { func_table->adquire(this); }
    void release() { func_table->release(this); }
    bool unique()  { return func_table->unique(this); }
    std::size_t bytes_capacity() { return func_table->bytes_capacity(this); }
};


/**
    This class template basically defines the ABI of basic_api_string

    The `small` object is used in SSO (small string optimization) mode
    The `big` object is used otherwise.
    We are in SSO mode, if, and only if, `big.str == nullptr`

    - when in SSO mode:
    ..- `big.str` must be null
    ..- `basic_api_string::data()` must return `small.str`
    ..- `small.str[small.capacity()]` must be zero (otherwise `big.str` may became non zero)
    ..- `small.str[small.len]` must be zero
    ..- `small.len` must not be greater than `small.capacity()`
    ..- if `small.len == 0` , then `big.len` must be zero too
        ( this facilitates the implementation of `basic_api_string::empty()` )

    - when not in SSO mode:
    ..- `big.str` must not be null
    ..- `basic_api_string::data()` must return `big.str`
    ..- `big.str[big.len]` must be zero
    ..- `big.mem_manager` is used to update the reference counters.
    ..-  `big.mem_manager` may be null, in this case `basic_api_string` does not manage
         the lifetime of the memory pointer by `big.str`. This is the situation when
         `basic_api_string` is created by `api_string_ref` function.
*/
template <typename CharT> union api_string_data
{
    constexpr static std::size_t small_capacity()
    {
        constexpr std::size_t s = (2 * sizeof(void*)) / sizeof(CharT);
        return s > 0 ? (s - 1) : 0;
    }

    struct
    {
        std::size_t len;
        speudo_std::abi::api_string_mem_base* mem_manager;
        const CharT* str;
    } big;

    struct
    {
        unsigned char len;
        CharT str[small_capacity() + 1];
    } small;
};


template <typename CharT>
constexpr void reset(speudo_std::abi::api_string_data<CharT>& data)
{
    if(sizeof(data.big) > sizeof(data.small))
    {
        data.big = {0};
    }
    else
    {
        data.small = {0};
    }
}

} // namespace abi

namespace _detail {

class basic_string_helper;

void api_string_init
    ( speudo_std::abi::api_string_data<char>& data
    , const char* str
    , std::size_t count);

void api_string_init
    ( speudo_std::abi::api_string_data<char16_t>& data
    , const char16_t* str
    , std::size_t count);

void api_string_init
    ( speudo_std::abi::api_string_data<char32_t>& data
    , const char32_t* str
    , std::size_t count);

void api_string_init
    ( speudo_std::abi::api_string_data<wchar_t>&  data
    , const wchar_t* str
    , std::size_t count);

std::size_t str_length(const char* str);
std::size_t str_length(const wchar_t* str);
std::size_t str_length(const char16_t* str);
std::size_t str_length(const char32_t* str);

int str_compare
    ( const char* lhs
    , std::size_t lhs_len
    , const char* rhs
    , std::size_t len_rhs);

int str_compare
    ( const wchar_t* lhs
    , std::size_t lhs_len
    , const wchar_t* rhs
    , std::size_t len_rhs);

int str_compare
    ( const char16_t* lhs
    , std::size_t lhs_len
    , const char16_t* rhs
    , std::size_t rhs_len);

int str_compare
    ( const char32_t* lhs
    , std::size_t lhs_len
    , const char32_t* rhs
    , std::size_t rhs_len);

void throw_std_out_of_range(const char*);

} // namespace _detail


struct api_string_ref_tag {};

template <typename CharT> class basic_api_string;

namespace _detail{
template <typename CharT>
inline basic_api_string<CharT> api_string_ref(const CharT* str, std::size_t len);
}

template <typename CharT> class basic_api_string
{
public:

    using value_type = CharT;
    using pointer = CharT*;
    using const_pointer = const CharT*;
    using reference = CharT&;
    using const_reference = const CharT&;
    using const_iterator = const CharT*;
    using interator      = const CharT*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr basic_api_string() noexcept
    {
        speudo_std::abi::reset(_data);
    }

    basic_api_string(const basic_api_string& other) noexcept
        : _data(other._data)
    {
        _adquire();
    }

    basic_api_string(basic_api_string&& other) noexcept
        : _data(other._data)
    {
        if (_is_managed())
        {
            speudo_std::abi::reset(other._data);
        }
    }

    basic_api_string(const CharT* str, size_type count)
    {
        speudo_std::_detail::api_string_init(_data, str, count);
    }

    basic_api_string(const CharT* str)
        : basic_api_string(str, speudo_std::_detail::str_length(str))
    {
    }

    constexpr basic_api_string(speudo_std::api_string_ref_tag, const CharT* str)
    {
        _data.big.len = speudo_std::_detail::str_length(str);
        _data.big.mem_manager = nullptr;
        _data.big.str = str;
    }

    ~basic_api_string()
    {
        _release();
    }

    // modifiers

    basic_api_string& operator=(const basic_api_string& other) noexcept
    {
        if(_data.big.str != other._data.big.str)
        {
            basic_api_string tmp{other};
            swap(tmp);
        }
        return *this;
    }

    basic_api_string& operator=(basic_api_string&& other) noexcept
    {
        if(_data.big.str != other._data.big.str)
        {
            basic_api_string tmp{static_cast<basic_api_string&&>(other)};
            swap(tmp);
        }
        return *this;
    }

    basic_api_string& operator=(const CharT* str)
    {
        basic_api_string tmp{str};
        swap(tmp);
        return *this;
    }

    void clear()
    {
        _release();
        speudo_std::abi::reset(_data);
    }

    void swap(basic_api_string& other) noexcept
    {
        _data_type tmp = other._data;
        other._data = _data;
        _data = tmp;
    }

    // capacity

    constexpr bool empty() const noexcept
    {
        return _data.big.len == 0;
    }

    constexpr size_type length() const noexcept
    {
        return _big() ? _data.big.len : _data.small.len;
    }

    constexpr size_type size() const noexcept
    {
        return length();
    }

    // element access

    const_pointer data() const noexcept
    {
        return _big() ? _data.big.str : _data.small.str;
    }
    const_pointer c_str() const noexcept
    {
        return data();
    }
    const_iterator cbegin() const noexcept
    {
        return const_iterator{data()};
    }
    const_iterator begin() const noexcept
    {
        return const_iterator{data()};
    }
    const_iterator cend() const noexcept
    {
        return const_iterator{_data_end()};
    }
    const_iterator end() const noexcept
    {
        return const_iterator{_data_end()};
    }
    constexpr const_reference operator[](size_type pos) const noexcept
    {
        return data()[pos];
    }
    constexpr const_reference at(size_type pos) const
    {
        if (pos >= length())
        {
            speudo_std::_detail::throw_std_out_of_range("basic_api_string::at() out of range");
        }
        return data()[pos];
    }
    constexpr const_reference front() const noexcept
    {
        return * data();
    }
    constexpr const_reference back() const noexcept
    {
        return * (_data_end() - 1);

    }

    // Comparison

    int compare(const basic_api_string& s) const
    {
        return speudo_std::_detail::str_compare
            ( data()
            , size()
            , s.data()
            , s.size() );
    }

    constexpr int compare
        ( size_type pos1
        , size_type count1
        , const basic_api_string& s) const
    {
        size_t max_count = size() - pos1;
        if (count1 > max_count)
        {
            count1 = max_count;
        }
        return speudo_std::_detail::str_compare
            ( &at(pos1)
            , count1
            , s.data()
            , s.size() );
    }

    // constexpr int compare
    //     ( size_type pos1
    //     , size_type count1
    //     , const basic_api_string& s
    //     , size_type pos2
    //     , size_type count2 ) const
    // {
    //     return speudo_std::_detail::str_compare
    //         ( &at(pos1)
    //         , std::min(count1, size() - pos1)
    //         , &s.at(pos2)
    //         , std::min(count2, s.size() - pos2) );
    // }

    constexpr int compare(const CharT* s) const
    {
        return speudo_std::_detail::str_compare
            ( data()
            , size()
            , s
            , speudo_std::_detail::str_length(s) );
    }

    // constexpr int compare
    //     ( size_type pos1
    //     , size_type count1
    //     , const CharT* s) const
    // {
    //     return speudo_std::_detail::str_compare
    //         ( &at(pos1)
    //         , std::min(count1, size() - pos1)
    //         , s
    //         , speudo_std::_detail::str_length(s) );
    // }

    // constexpr int compare
    //     ( size_type pos1
    //     , size_type count1
    //     , const CharT* s
    //     , size_type count2 ) const
    // {
    //     return speudo_std::_detail::str_compare
    //         ( &at(pos1)
    //         , std::min(count1, size() - pos1)
    //         , s
    //         , cout2 );
    // }

    // todo

    bool starts_with(const basic_api_string& x) const noexcept
    {
        size_type xlen = x.size();
        return x <= size() && 0 == str_compare(data(), xlen, x.data(), xlen);
    }
    bool starts_with(CharT x) const noexcept
    {
        return _data.big.len != 0 && *data() == x;
    }
    // bool starts_with(const CharT* x) const
    // {
    //     size_type xlen = str_length(x);
    //     return x <= size() && 0 == str_compare(data(), xlen, x, xlen);
    // }
    //   bool ends_with(const basic_api_string_view& x) const noexcept;
    //   bool ends_with(CharT x) const noexcept;
    //   bool ends_with(const CharT* x) const;

private:

    constexpr basic_api_string
        ( speudo_std::api_string_ref_tag
        , const CharT* str
        , std::size_t len )
    {
        _data.big.len = len;
        _data.big.mem_manager = nullptr;
        _data.big.str = str;
    }

    template <typename C>
    friend basic_api_string<C>  _detail::api_string_ref(const C* str, std::size_t);

    const_pointer _data_end() const
    {
        return _big()
            ? (_data.big.str + _data.big.len)
            : (_data.small.str + _data.small.len);
    }

    bool _is_managed()
    {
        return _big() && _data.big.mem_manager != nullptr;
    }

    void _adquire()
    {
        if(_is_managed())
        {
            _data.big.mem_manager->adquire();
        }
    }

    void _release()
    {
        if(_is_managed())
        {
            _data.big.mem_manager->release();
        }
    }

    constexpr bool _big() const noexcept
    {
        return _data.big.str != nullptr;
    }

    using _data_type = speudo_std::abi::api_string_data<CharT>;
    _data_type _data = _data_type{0};

    friend class speudo_std::_detail::basic_string_helper;

#if defined(API_STRING_TEST_MODE)
public:
    constexpr static std::size_t sso_capacity = _data_type::small_capacity();
#endif
};

using api_string    = basic_api_string<char>;
using api_u16string = basic_api_string<char16_t>;
using api_u32string = basic_api_string<char32_t>;
using api_wstring   = basic_api_string<wchar_t>;

template <typename CharT>
constexpr speudo_std::basic_api_string<CharT> api_string_ref(const CharT* str)
{
    return {speudo_std::api_string_ref_tag{}, str};
}

namespace _detail {

template <typename CharT>
inline basic_api_string<CharT> api_string_ref(const CharT* str, std::size_t len)
{
    return {speudo_std::api_string_ref_tag{}, str, len};
}

}

namespace string_literals {

inline api_string operator ""_as(const char* str, std::size_t len)
{
    return speudo_std::_detail::api_string_ref(str, len);
}
inline api_u16string operator ""_as(const char16_t* str, std::size_t len)
{
    return speudo_std::_detail::api_string_ref(str, len);
}
inline api_u32string operator ""_as(const char32_t* str, std::size_t len)
{
    return speudo_std::_detail::api_string_ref(str, len);
}
inline api_wstring operator ""_as(const wchar_t* str, std::size_t len)
{
    return speudo_std::_detail::api_string_ref(str, len);
}

} // namespace string_literals

template<class CharT>
bool operator == (const speudo_std::basic_api_string<CharT>& lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
}

template<class CharT>
bool operator != (const speudo_std::basic_api_string<CharT>& lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return lhs.size() != rhs.size() || lhs.compare(rhs) != 0;
}

template<class CharT>
bool operator < (const speudo_std::basic_api_string<CharT>& lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return lhs.compare(rhs) < 0;
}

template<class CharT>
bool operator <= (const speudo_std::basic_api_string<CharT>& lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return lhs.compare(rhs) <= 0;
}

template<class CharT>
bool operator > (const speudo_std::basic_api_string<CharT>& lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return lhs.compare(rhs) > 0;
}

template<class CharT>
bool operator >= (const speudo_std::basic_api_string<CharT>& lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return lhs.compare(rhs) >= 0;
}

template<class CharT>
bool operator == (const CharT* lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return speudo_std::api_string_ref(lhs) == rhs;
}

template<class CharT>
bool operator == (const speudo_std::basic_api_string<CharT>& lhs, const CharT* rhs)
{
    return lhs == speudo_std::api_string_ref(rhs);
}

template<class CharT>
bool operator != (const CharT* lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return speudo_std::api_string_ref(lhs) != rhs;
}

template<class CharT>
bool operator != (const speudo_std::basic_api_string<CharT>& lhs, const CharT* rhs)
{
    return lhs != speudo_std::api_string_ref(rhs);
}

template<class CharT>
bool operator < (const CharT* lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return speudo_std::api_string_ref(lhs) < rhs;
}

template<class CharT>
bool operator < (const speudo_std::basic_api_string<CharT>& lhs,  const CharT* rhs)
{
    return lhs < speudo_std::api_string_ref(rhs);
}

template<class CharT>
bool operator <= (const CharT* lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return speudo_std::api_string_ref(lhs) <= rhs;
}

template<class CharT>
bool operator <= (const speudo_std::basic_api_string<CharT>& lhs, const CharT* rhs)
{
    return lhs <= speudo_std::api_string_ref(rhs);
}

template<class CharT>
bool operator > (const CharT* lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return speudo_std::api_string_ref(lhs) > rhs;
}

template<class CharT>
bool operator > (const speudo_std::basic_api_string<CharT>& lhs, const CharT* rhs)
{
    return lhs > speudo_std::api_string_ref(rhs);
}

template<class CharT>
bool operator >= (const CharT* lhs, const speudo_std::basic_api_string<CharT>& rhs)
{
    return speudo_std::api_string_ref(lhs) >= rhs;
}

template<class CharT>
bool operator >= (const speudo_std::basic_api_string<CharT>& lhs, const CharT* rhs)
{
    return lhs >= speudo_std::api_string_ref(rhs);
}



}// namespace speudo_std

#endif  // API_STRING_HPP

