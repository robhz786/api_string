
[comment]: # (  Distributed under the Boost Software License, Version 1.0.  )
[comment]: # (     (See accompanying file LICENSE_1_0.txt or copy at        )
[comment]: # (        http://www.boost.org/LICENSE_1_0.txt                  )

# Introduction

Almost every API contain some functions that receive or return strings. Yet, the C++ standard does not provide any good solution to transfer strings across modules boundaries. While `std::basic_string` does not have a standard ABI, raw strings and `std::basic_string_view` are not able manage heap allocated memory.

The purpose of this project is to persuade the addition into the C++ Standard Library of a new string type with a standard ABI that could safely cross module boundaries carrying dynamically allocated memory. The proposed solution is to use a string type that can reference an immutable array of characters managed by reference counting, just like an `std::shared_ptr<char[]>`. 

The `basic_api_string` class template aims to prove the concept. Its notable characteristics are:

- Copy constructor is always fast and never throws.
- Header file is fast to compile.
- It supports small string optimisation.
- The user can choose to construct an `api_string` that only reference a string without managing its lifetime. After all, why to allocate and copy, if one can just reference a statically allocated string?
- Can safely cross modules boundaries, because:
  * It has a specified ABI
  * Memory allocated in one module is not deallocated in another.

Since `api_string` primary aims to be suitable to be used in APIs, compilation time is critical, especially because strings tend to be needed everywhere. Therefore you will note some limitations when comparing it to `std::string_view` and `std::string`. For example: no reverse iterator and no possibility to choose a traits class. However, the user can simply create an `std::string_view` from an `api_string` object, or use or use some other parts of `std`, to achieve the missing functionalities.

The second purpose of this project is to check whether it is possible to reimplement `std::string` so that it uses the same memory structure of `api_string`. The difference is that `std::string` would always keep unique ownership to safely provide mutable access to the individual characters. This way, after the user finishes the composition of the `std::string` content, it could be moved into an `api_string` without memory allocation and copy, leaving the original `std::string` object empty. After being moved, the content gets shared ownership, and lost mutable access.

---
# The headers
There are two public headers in this repository: `api_string.hpp` and `string.hpp`. everithing is inside the `speudo_std` namespace. **Note:** This is _not_ a header-only library. To build the library there is one sole source file to compile: `source/api_string.cpp`.

## The header `api_string.hpp` header

`api_string.hpp` defines `basic_api_string` class template, whose interface is similar to the one of `std::basic_string_view`. It has some some limitations in order to reduce compilation times:
- `CharT` must be `char`, `char16_t`, `char32_t` or `wchar_t`
- No reverse iterator. They would require the inclusion of `<iterator>` ( Even if we implement it, we would still need `std::andom_access_iterator_tag` ).
- Not possible to customize `Traits` class.
- There is no Allocator template parameter. However, the other header `string.hpp` provides ways to create in `api_string` object with an alternative allocator.

The `api_string_ref` function template creates a `basic_api_string` object that just references a string without managing its lifetime.


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
    constexpr basic_api_string() noexcept;
    basic_api_string(const basic_api_string& other) noexcept;
    basic_api_string(basic_api_string&& other) noexcept;
    basic_api_string(const CharT* str, size_type count);
    basic_api_string(const CharT* str);
    constexpr basic_api_string(api_string_dont_manage tag, const CharT* str);
    ~basic_api_string();

    // Modifiers
    basic_api_string& operator=(const basic_api_string& other) noexcept;
    basic_api_string& operator=(basic_api_string&& other) noexcept;
    basic_api_string& operator=(const CharT* str);
    constexpr void swap(basic_api_string& other) noexcept;
    void clear();

    // Capacity
    constexpr bool empty() const noexcept;
    constexpr size_type length() const noexcept;
    constexpr size_type size() const noexcept;

    // Element access
    const_pointer c_str() const noexcept;
    const_pointer data() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator begin() const noexcept;
    const_iterator cend() const noexcept;
    const_iterator end() const noexcept;
    constexpr const_reference operator[](size_type pos) const;
    constexpr const_reference at(size_type pos) const; // throws std::out_of_range
    constexpr const_reference front() const;
    constexpr const_reference back() const;

    // Comparison
    int compare(const basic_api_string& other) const;
    bool starts_with(const basic_api_string& x) const noexcept;  
    bool starts_with(CharT x) const noexcept;
    bool starts_with(const CharT* x) const;
    bool ends_with(const basic_api_string_view& x) const noexcept;
    bool ends_with(CharT x) const noexcept;
    bool ends_with(const CharT* x) const;
};

template <class CharT> basic_api_string<CharT> api_string_ref(const CharT* s);

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

using api_string    = basic_api_string<char>;
using api_u16string = basic_api_string<char16_t>;
using api_u32string = basic_api_string<char32_t>;
using api_wstring   = basic_api_string<wchar_t>;
}

```

## The `string.hpp` header

`string.hpp` defines `basic_string` class template, whose interface is basically the same of `std::basic_string`.

The constructor that takes `basic_api_string<CharT>&&` exist because it could use the following optimization: if the reference count of `s` is equal to one, then its content could just be moved.

When an `basic_string` object is converted to `basic_api_string`, the allocator of the `basic_string` ( or a rebinded copy ) is stored together with the reference counter so that it will be further  used for the destroction and deallocation.


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

# The ABI of `basic_api_string`

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
  - `big.str` must be null
  - `basic_api_string<CharT>::data()` must return `small.str`
  - `small.str[sso_capacity()]` must be zero (otherwise `big.str` may became non zero)
  - `small.str[small.len]` must be zero
  - `small.len` must not be greater than `sso_capacity()`
  - if `small.len == 0` , then `big.len` must be zero too ( this facilitates the implementation of `basic_api_string<CharT>::empty()` )

- when not in SSO mode:
  - `big.str` must not be null
  - `basic_api_string<CharT>::data()` must return `big.str`
  - `big.str[big.len]` must be zero
  - `big.mem_manager` is used to update the reference counters.
  -  `big.mem_manager` may be null, in this case `basic_api_string` does not manage the lifetime of the memory pointer by `big.str`. This is the case when `basic_api_string` is created by `api_string_ref` function.


### The `api_string_mem_base` class

The `api_string_mem_base` class contains the function to manage the reference count

- `api_string_mem_base::abi_version` shall be equall to zero.

```c++
struct api_string_mem_base;

struct api_string_func_table {
    typedef void        (*func_void) (api_string_mem_base*);
    typedef bool        (*func_bool) (api_string_mem_base*);
    typedef std::size_t (*func_size) (api_string_mem_base*);

    unsigned long abi_version; // reserved for possible future use
    func_void adquire;
    func_void release;
    func_bool unique;
    func_size bytes_capacity;
};

struct api_string_mem_base {
    const api_string_func_table* const func_table;

    void adquire() { func_table->adquire(this);  }
    void release() { func_table->release(this);  }
    bool unique()  { return func_table->unique(this); }
    std::size_t bytes_capacity() { return func_table->bytes_capacity(this); }
};
```
For example the `basic_api_string<CharT>::clear()` function could be implemented like this:

```c++
template <typename CharT>
void basic_api_string<CharT>::clear() {
    if (big.str != nullptr && big.mem_manager != nullptr) {
        big.mem_manager->release();
    }
    std::memset(this, 0, sizeof(basic_api_string));
}
```
