#ifndef SPEUDO_STD_STRING_HPP
#define SPEUDO_STD_STRING_HPP

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/api_string_memory.hpp>
#include <string_view> // char_traits

namespace speudo_std {

template
    < typename CharT
    , typename Traits = std::char_traits<CharT>
    , typename Allocator = std::allocator<CharT> >
class basic_string;

namespace private_ {

class basic_string_helper
{
    template <typename CharT>
    static speudo_std::abi::api_string_data<CharT>&
    get_data(speudo_std::basic_api_string<CharT>& s)
    {
        return s._data;
    }

    template < typename, typename, typename >
    friend class speudo_std::basic_string;
};

}


template <typename CharT, typename Traits, typename Allocator>
class basic_string
{
    using _alloc_traits = std::allocator_traits<Allocator>;

    using _prop_alloc_on_copy_assignment
    = typename _alloc_traits::propagate_on_container_copy_assignment;

    using _prop_alloc_on_move_assignment
    = typename _alloc_traits::propagate_on_container_move_assignment;


    using _memory_creator = speudo_std::private_::api_string_mem<Allocator>;

public:

    using value_type     = CharT;
    using traits_type    = Traits;
    using allocator_type = Allocator;

    using size_type       = typename std::allocator_traits<Allocator>::size_type;
    using difference_type = typename std::allocator_traits<Allocator>::difference_type;
    using pointer         = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer   = typename std::allocator_traits<Allocator>::const_pointer;
    using reference       = CharT&;
    using const_reference = const CharT&;

    using iterator               = CharT*;
    using const_iterator         = const CharT*;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static const size_type npos = -1;

    basic_string() noexcept(noexcept(Allocator{}))
        : basic_string(Allocator{})
    {
    }

    explicit basic_string(const Allocator& a) noexcept
        : _allocator{a}
    {
        _reset_data();
    }

    basic_string
        ( size_type count
        , CharT ch
        , const Allocator& alloc = Allocator() )
        : basic_string(alloc)
    {
        append(count, ch);
    }

    basic_string
        ( const basic_string& other
        , size_type pos
        , const Allocator& alloc = Allocator() );

    basic_string
        ( const basic_string& other
        , size_type pos
        , size_type count
        , const Allocator& alloc = Allocator() );

    basic_string
        ( const CharT* s
        , size_type count
        , const Allocator& alloc = Allocator() )
        : basic_string(alloc)
    {
        assign(s, count);
    }

    basic_string
        ( const CharT* s
        , const Allocator& alloc = Allocator() )
        : basic_string(s, Traits::length(s), alloc)
    {
        assign(s);
    }

    // template< class InputIt >
    // basic_string
    //     ( InputIt first
    //     , InputIt last,
    //     , const Allocator& alloc = Allocator()
    //     );

    basic_string(const basic_string& other, const Allocator& alloc)
        : basic_string(other.data(), other.size(), alloc)
    {
    }

    basic_string(const basic_string& other)
        : _allocator(_alloc_traits::select_on_container_copy_construction(other._allocator))
    {
        _reset_data();
        append(&other[0], other.size());
    }

    basic_string(basic_string&& other, const Allocator& alloc)
        noexcept(std::is_nothrow_copy_constructible<Allocator>::value)
        : _allocator(alloc)
        , _data(other._data)
    {
         other._reset_data();
    }

    basic_string(basic_string&& other)
        noexcept(std::is_nothrow_move_constructible<Allocator>::value)
        : _allocator(std::move(other._allocator))
        , _data(other._data)
    {
        other._reset_data();
    }

    // basic_string
    //     ( std::initializer_list<CharT> init
    //     , const Allocator& alloc = Allocator()
    //     );

    // template <class T>
    // explicit basic_string(const T& t, const Allocator& alloc = Allocator());

    // template <class T>
    // basic_string
    //     ( const T& t
    //     , size_type pos, size_type n
    //     , const Allocator& alloc = Allocator()
    //     );

    basic_string
        ( const basic_api_string<CharT>& other
        , const Allocator& alloc = Allocator{} )
        : basic_string(other.data(), other.size(), alloc)
    {
    }

    basic_string
        ( basic_api_string<CharT>&& other
        , const Allocator& alloc = Allocator{} );


    ~basic_string()
    {
        if(_big())
        {
            _data.big.mem_manager->release();
        }
    }

    //
    // Assignment
    //
    basic_string& operator=(const basic_string& other)
    {
        return assign(other);
    }
    basic_string& operator=(basic_string&& other)
    {
        return assign(std::move(other));
    }
    basic_string& operator=(const CharT* s)
    {
        return assign(s);
    }
    basic_string& operator=(CharT ch)
    {
        clear();
        push_back(ch);
    }
    basic_string& operator=(std::initializer_list<CharT> ilist)
    {
        return assign(ilist.begin(), ilist.size());
    }
    //template<class T>
    //basic_string& operator=(const T& t);

    basic_string& assign(size_type count, CharT ch)
    {
        clear();
        return append(count, ch);
    }
    basic_string& assign(const basic_string& other)
    {
        _copy_allocator(other, _prop_alloc_on_copy_assignment{});
        return assign(&other[0], other.size());
    }
    basic_string& assign
        ( const basic_string& other
        , size_type pos
        , size_type count = npos );

    basic_string& assign(basic_string&& other);

private:

    void _copy_allocator(const basic_string& other, std::true_type)
    {
        _allocator = other._allocator;
    }
    void _copy_allocator(const basic_string& other, std::false_type)
    {
    }
    void _move_allocator(basic_string& other, std::true_type)
    {
        _allocator = std::move(other._allocator);
    }
    void _move_allocator(basic_string& other, std::false_type)
    {
    }

public:

    basic_string& assign(const CharT* s, size_type count);

    basic_string& assign(const CharT* s)
    {
        return assign(s, Traits::length(s));
    }

    // template< class InputIt >
    // basic_string& assign(InputIt first, InputIt last);

    basic_string& assign(std::initializer_list<CharT> ilist)
    {
        assign(ilist.begin(), ilist.size());
    }

    // template < class T >
    // basic_string& assign(const T& t);

    // template < class T >
    // basic_string& assign(const T& t, size_type pos, size_type count = npos);


    allocator_type get_allocator() const
    {
        return _allocator;
    }

    //
    // Element access
    //
    CharT& operator[](size_type pos)
    {
        return data()[pos];
    }
    const CharT& operator[](size_type pos) const
    {
        return data()[pos];
    }
    CharT& at(size_type pos);

    const CharT& at(size_type pos) const;

    CharT& front()
    {
        return *data();
    }
    const CharT& front() const
    {
        return *data();
    }
    CharT& back()
    {
        return *(end() - 1);
    }
    const CharT& back() const
    {
        return *(end() - 1);
    }
    const_pointer data() const
    {
        return _big() ? _data.big.str : _data.small.str;
    }
    pointer data()
    {
        return _big() ? _data.big.str : _data.small.str;
    }
    const_pointer c_str() const
    {
        return data();
    }
    // operator std::basic_string_view<CharT, Traits>() const noexcept;

    operator speudo_std::basic_api_string<CharT>() const &
    {
        return basic_string{*this}._move_to_api_string();
    }

    operator speudo_std::basic_api_string<CharT>() &&
    {
        return std::move(*this)._move_to_api_string();
    }

    operator speudo_std::basic_api_string<CharT>() const &&
    {
        return basic_string{*this}._move_to_api_string();
    }

    //
    // Iterators
    //

    iterator begin()
    {
        return data();
    }
    const_iterator begin() const
    {
        return data();
    }
    const_iterator cbegin() const
    {
        return data();
    }
    iterator end()
    {
        return _big()
            ? (_data.big.str + _data.big.len)
            : (_data.small.str + _data.small.len);
    }
    const_iterator end() const
    {
        return _big()
            ? (_data.big.str + _data.big.len)
            : (_data.small.str + _data.small.len);
    }
    const_iterator cend() const
    {
        return end();
    }
    reverse_iterator rbegin()
    {
        return std::make_reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const
    {
        return std::make_reverse_iterator(end());
    }
    const_reverse_iterator crbegin() const
    {
        return std::make_reverse_iterator(cend());
    }
    reverse_iterator rend()
    {
        return std::make_reverse_iterator(begin());
    }
    const_reverse_iterator rend() const
    {
        return std::make_reverse_iterator(begin());
    }
    const_reverse_iterator crend() const
    {
        return std::make_reverse_iterator(cbegin());
    }

    //
    // Capacity
    //
    size_type length() const
    {
        return _big() ? _data.big.len : _data.small.len;
    }
    size_type size() const
    {
        return _big() ? _data.big.len : _data.small.len;
    }
    size_type capacity() const
    {
        return _big() ? _data.big.capacity : _data.small_capacity();
    }
    bool empty() const
    {
        return length() == 0;
    }
    void reserve(size_type new_cap)
    {
        if (new_cap > capacity()) {
            _replace_memory(data(), length(), new_cap);
        }
    }
    size_type max_size() const
    {
        return _memory_creator::max_bytes_size(_allocator) / sizeof(CharT) - 1;
    }
    void shrink_to_fit();

    //
    // Operations
    //

    void clear();

    // basic_string& insert( size_type index, size_type count, CharT ch );

    // basic_string& insert( size_type index, const CharT* s );

    // basic_string& insert( size_type index, const CharT* s, size_type count );

    // basic_string& insert( size_type index, const basic_string& other );

    // basic_string& insert
    //     ( size_type index
    //     , const basic_string& other
    //     , size_type index_other
    //     , size_type count = npos);

    // iterator insert( const_iterator pos, CharT ch );

    // iterator insert( const_iterator pos, size_type count, CharT ch );

    // template< class InputIt >
    // iterator insert( const_iterator pos, InputIt first, InputIt last );

    // iterator insert( const_iterator pos, std::initializer_list<CharT> ilist );

    // template < class T >
    // basic_string& insert( size_type pos, const T& t );

    // template < class T >
    // basic_string& insert
    //     ( size_type index
    //     , const T& t
    //     , size_type index_str
    //     , size_type count = npos);

    // basic_string& erase( size_type index = 0, size_type count = npos );

    // iterator erase( const_iterator position );

    // iterator erase( const_iterator first, const_iterator last );

    void push_back(CharT ch);

    value_type pop_back();

    basic_string& append( size_type count, CharT ch );

    basic_string& append( const basic_string& other )
    {
        return append(other.data(), other.length());
    }
    basic_string& append
        ( const basic_string& other
        , size_type pos
        , size_type count = npos )
    {
        return append(&other.at(pos), std::min(count, other.length() - pos));
    }
    basic_string& append( const CharT* s, size_type count );

    basic_string& append( const CharT* s )
    {
        return append(s, Traits::length(s));
    }

    // template< class InputIt >
    // basic_string& append( InputIt first, InputIt last );

    basic_string& append( std::initializer_list<CharT> ilist )
    {
        return append(ilist.begin(), ilist.size());
    }

    // template< class T >
    // basic_string& append( const T& t );

    // template< class T >
    // basic_string& append
    //     ( const T& t
    //     , size_type pos
    //     , size_type count = npos );

    basic_string& operator+=( const basic_string& other )
    {
        return append(other);
    }
    basic_string& operator+=( CharT ch )
    {
        return push_back(ch);
    }
    basic_string& operator+=( const CharT* s )
    {
        return append(s);
    }
    basic_string& operator+=( std::initializer_list<CharT> ilist )
    {
        return append(ilist);
    }
    // basic_string& operator+=( std::basic_string_view<CharT, Traits> sv)
    // {
    //     return append(&sv.front(), sv.size());
    // }
    int compare( const basic_string& other ) const
    {
        return _compare(data(), size(), other.data(), other.size());
    }
    int compare( size_type pos1, size_type count1, const basic_string& other ) const
    {
        return _compare
            ( &at(pos1)
            , std::min(count1, size() - pos1)
            , other.data()
            , other.size());
    }
    // int compare
    //     ( size_type pos1
    //     , size_type count1
    //     , const basic_string& other
    //     , size_type pos2
    //     , size_type count2 = npos ) const;

    int compare( const CharT* s ) const
    {
        return _compare(data(), size(), s, Traits::length(s));
    }

    // int compare( size_type pos1, size_type count1,
    //              const CharT* s ) const;
    // int compare
    //     ( size_type pos1
    //     , size_type count1
    //     , const CharT* s
    //     , size_type count2 ) const;

    // template < class T >
    // int compare( const T& t ) const;

    // template < class T >
    // int compare( size_type pos1, size_type count1, const T& t ) const;

    // template < class T >
    // int compare
    //     ( size_type pos1
    //     , size_type count1
    //     , const T& t
    //     , size_type pos2
    //     , size_type count2 = npos) const;

    // bool starts_with(std::basic_string_view<CharT, Traits> x) const noexcept;

    // bool starts_with(CharT x) const noexcept;

    // bool starts_with(const CharT* x) const;

    // bool ends_with(std::basic_string_view<CharT, Traits> x) const noexcept;

    // bool ends_with(CharT x) const noexcept;

    // bool ends_with(const CharT* x) const;

    // basic_string& replace
    //     ( size_type pos
    //     , size_type count
    //     , const basic_string& other );

    // basic_string& replace
    //     ( const_iterator first
    //     , const_iterator last
    //     , const basic_string& other );

    // basic_string& replace
    //     ( size_type pos
    //     , size_type count
    //     , const basic_string& other
    //     , size_type pos2
    //     , size_type count2 = npos );

    // template< class InputIt >
    // basic_string& replace
    //     ( const_iterator first
    //     , const_iterator last
    //     , InputIt first2
    //     , InputIt last2 );

    // basic_string& replace
    //     ( size_type pos
    //     , size_type count
    //     , const CharT* cstr
    //     , size_type count2 );

    // basic_string& replace
    //     ( const_iterator first
    //     , const_iterator last
    //     , const CharT* cstr
    //     , size_type count2 );

    // basic_string& replace
    //     ( size_type pos
    //     , size_type count
    //     , const CharT* cstr );

    // basic_string& replace
    //     ( const_iterator first
    //     , const_iterator last
    //     , const CharT* cstr );

    // basic_string& replace
    //     ( size_type pos
    //     , size_type count
    //     , size_type count2
    //     , CharT ch );

    // basic_string& replace
    //     ( const_iterator first
    //     , const_iterator last
    //     , size_type count2
    //     , CharT ch );

    // basic_string& replace
    //     ( const_iterator first
    //     , const_iterator last
    //     , std::initializer_list<CharT> ilist );

    // template < class T >
    // basic_string& replace
    //     ( size_type pos
    //     , size_type count
    //     , const T& t );

    // template < class T >
    // basic_string& replace
    //     ( const_iterator first
    //     , const_iterator last
    //     , const T& t );

    // template < class T >
    // basic_string& replace
    //     ( size_type pos
    //     , size_type count
    //     , const T& t
    //     , size_type pos2
    //     , size_type count2 = npos );


    // basic_string substr( size_type pos = 0, size_type count = npos ) const;

    // size_type copy( CharT* dest, size_type count, size_type pos = 0) const;

    void resize( size_type count )
    {
        resize(count, CharT{});
    }

    void resize( size_type count, CharT ch );

private:

    using _prop_alloc_on_swap
        = typename _alloc_traits::propagate_on_container_swap;

    constexpr static bool _is_swap_noexcept
        = _prop_alloc_on_swap::value
       || _alloc_traits::is_always_equal::value;

    void _swap_allocators(basic_string& other, std::false_type) noexcept
    {
    }

    void _swap_allocators(basic_string& other, std::true_type) noexcept(_is_swap_noexcept)
    {
        std::swap(_allocator, other._allocator);
    }

public:

    void swap(basic_string& other) noexcept(_is_swap_noexcept)
    {
        data_type tmp = other._data;
        other._data = _data;
        _data = tmp;
        _swap_allocators(other, _prop_alloc_on_swap{});
    }

    //
    // Search
    //
    // size_type find( const basic_string& str, size_type pos = 0 ) const

    //     size_type find( const CharT* s, size_type pos, size_type count ) const;

    // size_type find( const CharT* s, size_type pos = 0 ) const;

    // size_type find( CharT ch, size_type pos = 0 ) const;

    // template < class T >
    // size_type find( const T& t, size_type pos = 0 ) const;

    // size_type find_first_of( const basic_string& str, size_type pos = 0 ) const;

    // size_type find_first_of( const CharT* s, size_type pos, size_type count ) const;

    // size_type find_first_of( const CharT* s, size_type pos = 0 ) const;

    // size_type find_first_of( CharT ch, size_type pos = 0 ) const;

    // template < class T >
    // size_type find_first_of( const T& t, size_type pos = 0 ) const;

    // size_type find_first_not_of( const basic_string& str, size_type pos = 0 ) const;

    // size_type find_first_not_of( const CharT* s, size_type pos, size_type count ) const;

    // size_type find_first_not_of( const CharT* s, size_type pos = 0 ) const;

    // size_type find_first_not_of( CharT ch, size_type pos = 0 ) const;

    // template < class T >
    // size_type find_first_not_of( const T& t, size_type pos = 0 ) const;

    // size_type find_last_of( const basic_string& str, size_type pos = npos ) const;

    // size_type find_last_of( const CharT* s, size_type pos, size_type count ) const;

    // size_type find_last_of( const CharT* s, size_type pos = npos ) const;

    // size_type find_last_of( CharT ch, size_type pos = npos ) const;

    // template < class T >
    // size_type find_last_of( const T& t, size_type pos = npos ) const;

    // size_type find_last_not_of( const basic_string& str, size_type pos = npos ) const;

    // size_type find_last_not_of( const CharT* s, size_type pos, size_type count ) const;

    // size_type find_last_not_of( const CharT* s, size_type pos = npos ) const;

    // size_type find_last_not_of( CharT ch, size_type pos = npos ) const;

    // template < class T >
    // size_type find_last_not_of( const T& t, size_type pos = npos ) const;

private:

    speudo_std::basic_api_string<CharT> _move_to_api_string() &&;

    static int _compare
        ( const CharT* s1
        , size_type len1
        , const CharT* s2
        , size_type len2 )
    {
        int cmp = Traits::compare(s1, s2, std::min(len1, len2));
        return cmp != 0 ? cmp : (len1 == len2 ? 0 : (len1 < len2 ? -1 : +1));
    }

    bool _big() const
    {
        return _data.big.str != nullptr;
    }
    bool _small() const
    {
        return _data.big.str == nullptr;
    }

    void _grow_cap_if_necessary_for(size_t len_growth);

    void _replace_memory(const CharT* new_str, size_type new_len, size_type new_cap);

    // const Allocator&& _allocator_ref() const &&
    // {
    //     return static_cast<const Allocator&&>(_allocator);
    // }
    // const Allocator& _allocator_ref() const &
    // {
    //     return _allocator;
    // }
    // Allocator& _allocator_ref()&
    // {
    //     return _allocator;
    // }
    // Allocator&& _allocator_ref() &&
    // {
    //     return static_cast<Allocator&&>(_allocator);
    // }

    constexpr void _reset_data()
    {
        if(sizeof(_data.big) > sizeof(_data.small))
        {
            _data.big = {0};
        }
        else
        {
            _data.small = {0};
        }
    }

    Allocator _allocator;

    union data_type
    {
        constexpr static std::size_t small_capacity()
        {
            constexpr std::size_t s = (3 * sizeof(void*)) / sizeof(CharT);
            return s > 0 ? (s - 1) : 0;
        }

        struct
        {
            size_type len;
            size_type capacity;
            speudo_std::abi::api_string_mem_base* mem_manager;
            CharT* str;
        } big;

        struct
        {   // for small string optimizatiom
            unsigned char len;
            CharT str[small_capacity() + 1];
        } small;
    };

    data_type _data;

#if defined(API_STRING_TEST_MODE)
public:
    constexpr static std::size_t sso_capacity = data_type::small_capacity();
#endif
};


template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>::basic_string
    ( const basic_string& other
    , size_type pos
    , const Allocator& alloc )
    : basic_string(alloc)
{
    if(pos < other.size())
    {
        assign(&other[pos], other.size() - pos);
    }
    else if(pos > other.size())
    {
        private_::throw_std_out_of_range("basic_string::basic_string: pos > other.size()");
    }
}

template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>::basic_string
    ( const basic_string& other
    , size_type pos
    , size_type count
    , const Allocator& alloc )
    : basic_string(alloc)
{
    if(pos < other.size())
    {
        assign(&other[pos], count);
    }
    else if(pos > other.size())
    {
        private_::throw_std_out_of_range("basic_string::basic_string: pos > other.size()");
    }
}

template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>::basic_string
    ( basic_api_string<CharT>&& other
    , const Allocator& alloc )
    : basic_string(alloc)
{
    auto& other_data = reinterpret_cast<abi::api_string_data<CharT>&>(other);

    if( other_data.big.str != nullptr
     && other_data.big.mem_manager != nullptr
     && other_data.big.mem_manager->unique() )
    {
        _data.big.len = other_data.big.len;
        _data.big.mem_manager = other_data.big.mem_manager;
        _data.big.str = const_cast<CharT*>(other_data.big.str);
        _data.big.capacity = other_data.big.mem_manager->bytes_capacity() / sizeof(CharT) - 1;
        other_data = speudo_std::abi::api_string_data<CharT> {};
    }
    else if( ! other.empty())
    {
        assign(&other[0], other.size());
    }
}

template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>&
basic_string<CharT, Traits, Allocator>::assign
    ( const basic_string& other
    , size_type pos
    , size_type count )
{
    if (pos < other.size() && count != 0)
    {
        assign(&other[pos], std::min(other.size() - pos, count));
    }
    else if(pos > other.size())
    {
        private_::throw_std_out_of_range("basic_string::basic_string: pos > other.size()");
    }
    return *this;
}


template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>&
basic_string<CharT, Traits, Allocator>::assign(basic_string&& other)
{
    _move_allocator(other, _prop_alloc_on_move_assignment{});
    if(other._big())
    {
        if(_big())
        {
            _data.big.mem_manager->release();
        }
        _data = other._data;
        other._reset_data();
    }
    else if (_small())
    {
        _data = other._data;
    }
    else
    {
        _data.big.len = other._data.small.len;
        Traits::copy(_data.big.str, other._data.small.str, _data.big.len);
    }
    return *this;
}


template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>&
basic_string<CharT, Traits, Allocator>::assign(const CharT* s, size_type count)
{
    if(count > capacity())
    {
        _replace_memory(s, count, 2*count);
    }
    else if(_small())
    {
        _data.small.len = count;
        Traits::copy(_data.small.str, s, count);
        Traits::assign(_data.small.str[count], CharT{});
    }
    else
    {
        _data.big.len = count;
        Traits::copy(_data.big.str, s, count);
        Traits::assign(_data.big.str[count], CharT{});
    }
    return *this;
}

template <typename CharT, typename Traits, typename Allocator>
CharT& basic_string<CharT, Traits, Allocator>::at(size_type pos)
{
    if (pos >= size())
    {
        private_::throw_std_out_of_range("basic_string::at() out of range");
    }
    return data()[pos];
}

template <typename CharT, typename Traits, typename Allocator>
const CharT& basic_string<CharT, Traits, Allocator>::at(size_type pos) const
{
    if (pos >= size())
    {
        private_::throw_std_out_of_range("basic_string::at() out of range");
    }
    return data()[pos];
}

template <typename CharT, typename Traits, typename Allocator>
void basic_string<CharT, Traits, Allocator>::resize(size_type count, CharT ch)
{
    if(count < size())
    {
        if(_big())
        {
            _data.big.len = count;
            Traits::assign(_data.big.str[count], CharT{});
        }
        else
        {
            _data.small.len = count;
            Traits::assign(_data.small.str[count], CharT{});
        }
    }
    else if (count > size())
    {
        append(count - size(), ch);
    }
}

template <typename CharT, typename Traits, typename Allocator>
void basic_string<CharT, Traits, Allocator>::shrink_to_fit()
{
    if(_big())
    {
        constexpr size_type min_capacity_diff =
            sizeof(speudo_std::private_::api_string_mem<Allocator>)
            / sizeof(CharT);

        if (_data.big.len <= _data.small_capacity())
        {
            basic_string tmp{_data.big.str, _data.big.len};
            swap(tmp);
        }
        else if(_data.big.capcity - _data.big.len > min_capacity_diff)
        {
            _replace_memory(_data.big.str, _data.big.len, _data.big.len);
        }
    }
}

template <typename CharT, typename Traits, typename Allocator>
inline void basic_string<CharT, Traits, Allocator>::clear()
{
    _data.big.len = 0;
    if(_big())
    {
        _data.big.str[0] = 0;
    }
    else
    {
        _data.small.str[0] = 0;
    }
}

//
// Operations
//

template <typename CharT, typename Traits, typename Allocator>
speudo_std::basic_api_string<CharT>
basic_string<CharT, Traits, Allocator>::_move_to_api_string() &&
{
    speudo_std::basic_api_string<CharT> dest;
    auto& d = speudo_std::private_::basic_string_helper::get_data(dest);
    if (_big())
    {
        d.big.len = _data.big.len;
        d.big.mem_manager = _data.big.mem_manager;
        d.big.str = _data.big.str;
        _reset_data();
    }
    else if (_data.small.len <= d.small_capacity())
    {
        d.small.len = _data.small.len;
        Traits::copy(d.small.str, _data.small.str, _data.small.len);
        Traits::assign(d.small.str[_data.small.len], CharT{});
    }
    else
    {
        auto size = (_data.small.len + 1) * sizeof(CharT);
        auto m = _memory_creator::create(_allocator, size);
        CharT * str = reinterpret_cast<CharT*>(m.pool);

        d.big.len = _data.small.len;
        d.big.mem_manager = m.manager;
        d.big.str = str;
        Traits::copy(str, _data.small.str, _data.small.len);
        Traits::assign(str[_data.small.len], CharT{});
    }
    return dest;
}

template <typename CharT, typename Traits, typename Allocator>
CharT basic_string<CharT, Traits, Allocator>::pop_back()
{
    if(_big())
    {
        if (_data.big.len > 0)
        {
            CharT value = _data.big.str[_data.big.len - 1];
            -- _data.big.len;
            return value;
        }
    }
    else if (_data.small.len > 0)
    {
        CharT value = _data.small.str[_data.small.len - 1];
        -- _data.small.len;
        return value;
    }

    return 0;
}

template <typename CharT, typename Traits, typename Allocator>
void basic_string<CharT, Traits, Allocator>::push_back(CharT ch)
{
    _grow_cap_if_necessary_for(1);
    if(_big())
    {
        Traits::assign(_data.big.str[_data.big.len + 1], CharT());
        Traits::assign(_data.big.str[_data.big.len], ch);
        ++ _data.big.len;
    }
    else
    {
        Traits::assign(_data.small.str[_data.small.len + 1], CharT());
        Traits::assign(_data.small.str[_data.small.len], ch);
        ++ _data.small.len;
    }

}

template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>&
basic_string<CharT, Traits, Allocator>::append(size_type count, CharT ch)
{
    _grow_cap_if_necessary_for(count);
    if(_big())
    {
        Traits::assign(_data.big.str, count, ch);
        Traits::assign(_data.big.str[_data.big.len + count], CharT());
        _data.big.len += count;
    }
    else
    {
        Traits::assign(_data.small.str, count, ch);
        Traits::assign(_data.small.str[_data.small.len + count], CharT());
        _data.small.len += count;
    }
    return *this;
}

template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>&
basic_string<CharT, Traits, Allocator>::append(const CharT* s, size_type count)
{
    _grow_cap_if_necessary_for(count);
    if(_big())
    {
        Traits::assign(_data.big.str[_data.big.len + count], CharT());
        Traits::copy(_data.big.str + _data.big.len, s, count);
        _data.big.len += count;
    }
    else
    {
        Traits::assign(_data.small.str[_data.small.len + count], CharT());
        Traits::copy(_data.small.str + _data.small.len, s, count);
        _data.small.len += count;
    }
    return *this;
}

template <typename CharT, typename Traits, typename Allocator>
void basic_string<CharT, Traits, Allocator>::_grow_cap_if_necessary_for(size_t len_growth)
{
    if (length() + len_growth > capacity())
    {
        size_type new_cap = 2 * (length() + len_growth);
        _replace_memory(data(), length(), new_cap);
    }
}

template <typename CharT, typename Traits, typename Allocator>
void basic_string<CharT, Traits, Allocator>::_replace_memory
    ( const CharT* new_str
    , basic_string::size_type new_len
    , basic_string::size_type new_cap )
{
    assert(new_cap >= new_len);

    auto m = _memory_creator::create(_allocator, (new_cap + 1) * sizeof(CharT));

    data_type new_data;
    new_data.big.len = new_len;
    new_data.big.capacity = m.bytes_capacity / sizeof(CharT) - 1;
    new_data.big.mem_manager = m.manager;
    new_data.big.str = reinterpret_cast<CharT*>(m.pool);
    Traits::copy(new_data.big.str, new_str, new_len);
    Traits::assign(new_data.big.str[new_len], CharT{});

    data_type old_data = _data;
    _data = new_data;
    if (old_data.big.str != nullptr)
    {
        old_data.big.mem_manager->release();
    }
}



template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( const basic_string<CharT,Traits,Alloc>& lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    basic_string<CharT,Traits,Alloc> str;
    str.reserve(lhs.size() + rhs.size());
    str.append(lhs);
    str.append(rhs);
    return std::move(str);
}

template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( const CharT* lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    basic_string<CharT,Traits,Alloc> str;
    std::size_t lhs_len = Traits::length(lhs);
    str.reserve(lhs_len + rhs.size());
    str.append(lhs, lhs_len);
    str.append(rhs, rhs.length());
    return std::move(str);
}

template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( CharT lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    basic_string<CharT,Traits,Alloc> str;
    str.reserve(1 + rhs.size());
    str.push_back(lhs);
    str.append(rhs);
    return std::move(str);
}

template<class CharT, class Traits, class Alloc>
basic_string<CharT,Traits,Alloc> operator+
    ( const basic_string<CharT,Traits,Alloc>& lhs
    , CharT rhs )
{
    basic_string<CharT,Traits,Alloc> str;
    str.reserve(lhs.size() + 1);
    str = lhs;
    str.push_back(rhs);
    return std::move(str);
}

template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( basic_string<CharT,Traits,Alloc>&& lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    lhs.reserve(lhs.size() + rhs.size());
    std::move(lhs.apppend(rhs));
}

template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( const basic_string<CharT,Traits,Alloc>& lhs
    , basic_string<CharT,Traits,Alloc>&& rhs )
{
    if( ! lhs.empty())
    {
        if(rhs.empty())
        {
            if(rhs.capacity() >= lhs.size())
            {
                return std::move(rhs.append(lhs));
            }
            return lhs;
        }
        std::size_t rhs_size = rhs.size();
        rhs.resize(rhs_size + lhs.size());
        Traits::move(&rhs[0] + lhs.size(), &rhs[0], rhs_size);
        Traits::copy(rhs.data(), lhs.data(), lhs.size());
    }
    return std::move(rhs);
}

template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( basic_string<CharT,Traits,Alloc>&& lhs
    , basic_string<CharT,Traits,Alloc>&& rhs )
{
    std::size_t len = lhs.length() + rhs.length();
    if(lhs.capacity() >= len || rhs.capacity() < len)
    {
        return std::move(lhs.append(rhs));
    }
    rhs.resize(len);
    Traits::move(&rhs[0] + lhs.length(), &rhs[0], rhs.length());
    Traits::copy(rhs.data(), lhs.data(), lhs.length());
    return std::move(rhs);
}

template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( const CharT* lhs
    , basic_string<CharT,Traits,Alloc>&& rhs )
{
    std::size_t lhs_len = Traits::length(lhs);
    if (lhs_len > 0)
    {
        if (rhs.empty())
        {
            rhs = lhs;
        }
        else
        {
            rhs.resize(rhs.length() + lhs_len);
            Traits::move(&rhs[0] + lhs_len, &rhs[0], rhs.length());
            Traits::copy(rhs.data(), lhs, lhs_len);
        }
    }
    return std::move(rhs);
}

template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( CharT lhs
    , basic_string<CharT,Traits,Alloc>&& rhs )
{
    rhs.resize(rhs.size() + 1);
    Traits::move(&rhs[0] + 1, &rhs[0], rhs.length());
    Traits::assign(rhs[0], lhs);
    return std::move(rhs);
}

template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( basic_string<CharT,Traits,Alloc>&& lhs
    , const CharT* rhs )
{
    return std::move(lhs.append(rhs));
}

template< class CharT, class Traits, class Alloc >
basic_string<CharT,Traits,Alloc> operator+
    ( basic_string<CharT,Traits,Alloc>&& lhs
    , CharT rhs )
{
    return std::move(lhs.push_back(rhs));
}


template< class CharT, class Traits, class Alloc >
inline bool operator==
    ( const basic_string<CharT,Traits,Alloc>& lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    return lhs.compare(rhs) == 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator!=
    ( const basic_string<CharT,Traits,Alloc>& lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    return lhs.compare(rhs) != 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator<
    ( const basic_string<CharT,Traits,Alloc>& lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    return lhs.compare(rhs) < 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator<=
    ( const basic_string<CharT,Traits,Alloc>& lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    return lhs.compare(rhs) <= 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator>
    ( const basic_string<CharT,Traits,Alloc>& lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    return lhs.compare(rhs) > 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator>=
    ( const basic_string<CharT,Traits,Alloc>& lhs
    , const basic_string<CharT,Traits,Alloc>& rhs )
{
    return lhs.compare(rhs) >= 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator==( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs )
{
    return lhs.compare(rhs) == 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator==( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs )
{
    return rhs.compare(lhs) == 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator!=( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs )
{
    return lhs.compare(rhs) != 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator!=( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs )
{
    return rhs.compare(lhs) != 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator<( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs )
{
    return lhs.compare(rhs) < 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator<( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs )
{
    return rhs.compare(lhs) > 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator<=( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs )
{
    return lhs.compare(rhs) <= 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator<=( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs )
{
    return rhs.compare(lhs) >= 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator>( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs )
{
    return lhs.compare(rhs) > 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator>( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs )
{
    return rhs.compare(lhs) < 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator>=( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs )
{
    return lhs.compare(rhs) >= 0;
}

template< class CharT, class Traits, class Alloc >
inline bool operator>=( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs )
{
    return rhs.compare(lhs) <= 0;
}


using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;


} // namespace speudo_std

#endif
