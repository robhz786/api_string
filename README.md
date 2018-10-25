
[comment]: # (  Distributed under the Boost Software License, Version 1.0.  )
[comment]: # (     (See accompanying file LICENSE_1_0.txt or copy at        )
[comment]: # (        http://www.boost.org/LICENSE_1_0.txt                  )

# Introduction

Almost every API contains some functions that receive or return strings. Yet, the C++ standard does not provide any good solution to transfer strings across modules boundaries. While raw strings and `std::basic_string_view` are not able to manage dynamic allocated memory, `std::basic_string` does not have a standard ABI and its header is expensive to compile

The purpose of this project is to persuade the addition into the C++ Standard Library of a new string type with a standard ABI that could safely cross module boundaries carrying dynamically allocated memory. The proposed solution uses reference counting to manage an immutable array of characters. Kind of `std::shared_ptr<const char[]>`.

The `basic_api_string` class template aims to prove the concept. Its main characteristics are:

Pros:

- Its header file is fast to compile.
- It is able manage memory allocation/deallocation (using reference counting).
- Its copy constructor is always fast and never throws.
- It supports small string optimisation.
- `c_str()` and `data()` member functions that aways return a null-terminated raw string, *i.e.* they never return `nullptr`.
- The user can create an `basic_api_string` object from a string literal without any memory allocating.
- Can safely cross modules boundaries, because:
  * It has a specified ABI
  * It ensures that memory is deallocated in same module it has been allocated.

Cons:

- No write access to individual characters ( like `string_view` ).
- No support for char traits, allocators, nor reverse iterators ( to keep the header cheap to compile ).
- No support for user defined character type. It must be `char`, `char16_t`, `char32_t` or `wchar_t`.


The second purpose of this project is to check whether it is possible to reimplement `std::string` so that it uses the same memory structure of `api_string`. The difference is that `std::string` would always keep unique ownership to safely provide mutable access to the individual characters. This way, after the user finishes the composition of the `std::string` content, it could be moved into an `api_string` without memory allocation nor copy, leaving the original `std::string` object empty. After being moved to an `api_string`, the content gets shared ownership, and lost mutable access.

---
# The headers
There are two public headers in this repository: `api_string.hpp` and `string.hpp`. everything is inside the `speudo_std` namespace. **Note:** This is _not_ a header-only library. To build the library there is one sole source file to compile: `source/api_string.cpp`.

## The header `api_string.hpp` header

```c++
namespace speudo_std {

template <typename CharT> class basic_api_string {
public:
    // Types
    using value_type      = CharT;
    using pointer         = const CharT*;
    using const_pointer   = const CharT*;
    using reference       = CharT&;
    using const_reference = const CharT&;
    using const_iterator  = const CharT*;
    using interator       = const CharT*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;    

    // Construction and Destruction
    basic_api_string() noexcept;
    basic_api_string(const basic_api_string& other) noexcept;
    basic_api_string(basic_api_string&& other) noexcept;
    basic_api_string(const CharT* str, size_type count)
        [[expects: str != nullpr]];
    basic_api_string(const CharT* str)
        [[expects: str != nullpr]];

    ~basic_api_string();

    // Modifiers
    basic_api_string& operator=(const basic_api_string& other) noexcept;
    basic_api_string& operator=(basic_api_string&& other) noexcept;
    basic_api_string& operator=(const CharT* str)
        [[expects: str != nullpr]];
    void swap(basic_api_string& other) noexcept;
    void clear();

    // Capacity
    bool empty() const noexcept;
    size_type length() const noexcept;
    size_type size() const noexcept;

    // Element access
    const_pointer c_str() const noexcept
        [[ensures str: str != nullptr && str[length()] == CharT()]]
    const_pointer data() const noexcept;
        [[ensures str: str != nullptr && str[length()] == CharT()]]
    const_iterator cbegin() const noexcept;
    const_iterator begin() const noexcept;
    const_iterator cend() const noexcept;
    const_iterator end() const noexcept;
    const_reference operator[](size_type pos) const
        [[expects: pos <= length()]];
    const_reference at(size_type pos) const; // throws std::out_of_range
    const_reference front() const;
        [[expects: ! empty()]];
    const_reference back() const
        [[expects: ! empty()]];

    // Comparison
    int compare( const basic_api_string& other) const;
    int compare( size_type pos1
               , size_type count1
               , const basic_api_string& s ) const;  // throws std::out_of_rang
    int compare( size_type pos1
               , size_type count1
               , const basic_api_string& s
               , size_type pos2
               , size_type count2) const;  // throws std::out_of_rang
    int compare(const CharT* str) const
    int compare( size_type pos1
               , size_type count1
               , const CharT* s) const;
    int compare( size_type pos1
               , size_type count1
               , const CharT* s
               , size_type count2) const;
    bool starts_with(const basic_api_string& x) const;
    bool starts_with(CharT x) const;
    bool starts_with(const CharT* x) const;
    bool ends_with(const basic_api_string_view& x) const;
    bool ends_with(CharT x) const;
    bool ends_with(const CharT* x) const;
};

template <class CharT> bool operator==(const basic_api_string<CharT>&, const basic_api_string<CharT>&);
template <class CharT> bool operator!=(const basic_api_string<CharT>&, const basic_api_string<CharT>&);
template <class CharT> bool operator< (const basic_api_string<CharT>&, const basic_api_string<CharT>&);
template <class CharT> bool operator<=(const basic_api_string<CharT>&, const basic_api_string<CharT>&);
template <class CharT> bool operator> (const basic_api_string<CharT>&, const basic_api_string<CharT>&);
template <class CharT> bool operator>=(const basic_api_string<CharT>&, const basic_api_string<CharT>&);

template <class CharT> bool operator==(const CharT*, const basic_api_string<CharT>&);
template <class CharT> bool operator!=(const CharT*, const basic_api_string<CharT>&);
template <class CharT> bool operator< (const CharT*, const basic_api_string<CharT>&);
template <class CharT> bool operator<=(const CharT*, const basic_api_string<CharT>&);
template <class CharT> bool operator> (const CharT*, const basic_api_string<CharT>&);
template <class CharT> bool operator>=(const CharT*, const basic_api_string<CharT>&);

template <class CharT> bool operator==(const basic_api_string<CharT>&, const CharT*);
template <class CharT> bool operator!=(const basic_api_string<CharT>&, const CharT*);
template <class CharT> bool operator< (const basic_api_string<CharT>&, const CharT*);
template <class CharT> bool operator<=(const basic_api_string<CharT>&, const CharT*);
template <class CharT> bool operator> (const basic_api_string<CharT>&, const CharT*);
template <class CharT> bool operator>=(const basic_api_string<CharT>&, const CharT*);


template <class CharT>
basic_api_string<CharT> api_string_ref(const CharT* s);

template <class CharT>
basic_api_string<CharT> api_string_ref(const CharT* s, std::size_t len)
    [[expects: s[len] == CharT{}]];

namespace string_literals {

basic_api_string<char>     operator "" _as(const char* str, size_t len) noexcept;
basic_api_string<char16_t> operator "" _as(const char16_t* str, size_t len) noexcept;
basic_api_string<char32_t> operator "" _as(const char32_t* str, size_t len) noexcept;
basic_api_string<wchar_t>  operator "" _as(const wchar_t* str, size_t len) noexcept;

} // namespace string_literals


using api_string    = basic_api_string<char>;
using api_u16string = basic_api_string<char16_t>;
using api_u32string = basic_api_string<char32_t>;
using api_wstring   = basic_api_string<wchar_t>;

}

```

The `operator "" _as` functions as well as the `api_string_ref` function templates create a `basic_api_string` object that just references a string without managing its lifetime.


## The `string.hpp` header

`string.hpp` defines `basic_string` class template, whose interface is basically the same of `std::basic_string`.

The constructor that takes `basic_api_string<CharT>&&` exist because it could use the following optimization: if the reference count of `s` is equal to one, then its content could just be moved.

When a `basic_string` object is converted to `basic_api_string`, the allocator of the `basic_string` ( or a rebound copy ) is stored together with the reference counter so that it is further used for the destruction and deallocation.


```c++
namespace speudo_std {

template
    < typename CharT
    , typename Traits = std::char_traits<CharT>
    , typename Allocator = std::allocator<CharT> >
class basic_string {
public:
    basic_string(const basic_api_string<Chart>& s,  const Allocator& = Allocator());
    basic_string(basic_api_string<Chart>&& s, const Allocator& = Allocator());

    operator basic_api_string<CharT> () && ;
    operator basic_api_string<CharT> () const & ;
    
    // ... the rest is just like std::basic_string ...
};

}
```
An example:
```c++
    speudo_std::string str = "---- blah blah blah blah ----";
    speudo_std::api_string astr = std::move(str);

    assert(str.empty());
    assert(astr == "---- blah blah blah blah ----");
```


---

## The ABI of `basic_api_string`

`basic_api_string` has no virtual functions. Its has the following internal data structure:

```c++
union {

    constexpr static std::size_t sso_capacity()
    {
        constexpr std::size_t c = (2 * sizeof(void*)) / sizeof(CharT);
        return c > 0 ? (c - 1) : c;
    }

    struct {
        std::size_t len;
        abi::api_string_mem_base* mem_manager; // see below
        const CharT* str;
    } big;

    struct { // (for small string optimization)
        unsigned char len;
        CharT str[sso_capacity() + 1];
    } small;
};
```

The `small` object is used in SSO (small string optimization) mode. The `big` object is used otherwise. We are in SSO mode, if, and only if, `big.str == nullptr`

- when in SSO mode:
  - `big.str` must be null.
  - `basic_api_string<CharT>::data()` must return `small.str`.
  - `small.len` must not be greater than `sso_capacity()`.
  - `small.str[small.len]` must be zero. 
  - `small.str[sso_capacity()]` must be zero. Note that changing the value of `small.str[sso_capacity()]` corrupts `big.str`.
  - if `small.len == 0` , then `big.len` must be zero too ( this facilitates the implementation of `basic_api_string<CharT>::empty()` )

- when not in SSO mode:
  - `big.str` must not be null.
  - `basic_api_string<CharT>::data()` must return `big.str`.
  - `big.str[big.len]` must be zero.
  - If `big.mem_manager != nullptr` then the memory pointed by `big.str` is managed by reference counting, and `big.mem_manager` is used to update the counters.
  - If `big.mem_manager == nullptr` then the memory pointer by `big.str` is not managed by `basic_api_string`. This is the case when `basic_api_string` is created by `api_string_ref` function.


### The `api_string_mem_base` class

```c++
struct api_string_mem_base;

struct api_string_func_table
{
    typedef std::size_t (*func_size)(api_string_mem_base*);
    typedef void        (*func_void)(api_string_mem_base*);
    typedef bool        (*func_bool)(api_string_mem_base*);
    typedef std::byte*  (*func_ptr) (api_string_mem_base*);

    unsigned long abi_version = 0;
    func_size acquire = nullptr;
    func_void release = nullptr;
    func_bool unique  = nullptr;
    func_ptr  begin   = nullptr;
    func_ptr  end     = nullptr;
};

struct api_string_mem_base
{
    const api_string_func_table* const func_table;

    std::size_t acquire() { return func_table->acquire(this); }
    void release()        { func_table->release(this); }
    bool unique()         { return func_table->unique(this); }
    std::byte* begin()    { return func_table->begin(this); }
    std::byte* end()      { return func_table->end(this); }
};

```
* `acquire()` increments the reference counter, and returns the previous value
* `release()` decrements the reference counter and, if it becames zero, deallocates the memory.
* `unique()` tells whether the reretence countes is equal to one.
* `begin()` and `end()` return the memory region that contains the string. 
* `api_string_mem_base::abi_version` shall be equall to zero.

For example, the `basic_api_string<CharT>::clear()` function could be implemented like this:

```c++
template <typename CharT>
void basic_api_string<CharT>::clear() {
    if (big.str != nullptr && big.mem_manager != nullptr) {
        big.mem_manager->release();
    }
    std::memset(this, 0, sizeof(basic_api_string<CharT>));
}
```
