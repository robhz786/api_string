#ifndef SPEUDO_STD_STRING_HPP
#define SPEUDO_STD_STRING_HPP

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/api_string_memory.hpp>
#include <string> // char_traits

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
        return s.m_data;
    }

    template < typename, typename, typename >
    friend class speudo_std::basic_string;
};
    
}


template <typename CharT, typename Traits, typename Allocator>
class basic_string
{
    using prop_alloc_on_copy_assignment
    = typename std::allocator_traits<Allocator>::propagate_on_container_copy_assignment;

    using prop_alloc_on_move_assigment
    = typename std::allocator_traits<Allocator>::propagate_on_container_copy_assignment;

    using prop_alloc_on_swap
    = typename std::allocator_traits<Allocator>::propagate_on_container_swap;

    using memory_creator = speudo_std::private_::api_string_mem<Allocator>;

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
        : m_allocator{a}
    {
        reset_data();
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
        , const Allocator& alloc = Allocator() )
        : basic_string(alloc)
    {
        append(&other.at[pos], other.size() - pos);
    }

    basic_string
        ( const basic_string& other
        , size_type pos
        , size_type count
        , const Allocator& alloc = Allocator() )
        : basic_string(alloc)
    {
        append(&other.ap(pos), std::min(count, other.size() - pos));
    }

    basic_string
        ( const CharT* s
        , size_type count
        , const Allocator& alloc = Allocator() )
        : basic_string(alloc)
    {
        append(s, count);
    }

    basic_string
        ( const CharT* s
        , const Allocator& alloc = Allocator()
        )
        : basic_string(s, Traits::length(s), alloc)
    {
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
        : basic_string(other, prop_alloc_on_copy_assignment{})
    {
    }

    basic_string(basic_string&& other, const Allocator& alloc)
        : m_allocator(alloc)
        , m_data(other.m_data)
    {
         other.reset_data();
    }

    basic_string(basic_string&& other)
        : basic_string(std::move(other), prop_alloc_on_move_assigment{})
    {
    }

private:

    basic_string(const basic_string& other, std::false_type)
        : basic_string(other.data(), other.size())
    {
    }

    basic_string(const basic_string& other, std::true_type)
        : basic_string(other.data(), other.size(), other.m_allocator)
    {
    }

    basic_string(basic_string&& other, std::false_type)
        : m_data(other.m_data)
    {
        other.reset_data();
    }

    basic_string(basic_string&& other, std::true_type)
        : m_allocator(std::move(other.m_allocator))
        , m_data(other.m_data)
    {
        other.reset_data();
    }

public:

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


    ~basic_string()
    {
        if(big())
        {
            m_data.big.mem_manager->release();
        }
    }

    //
    // Assignment
    //
    basic_string& operator=(const basic_string& str)
    {
        return assign(str);
    }
    basic_string& operator=(basic_string&& str)
    {
        return assign(std::move(str));
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

    //basic_string& operator=(std::initializer_list<CharT> ilist);

    //template<class T>
    //basic_string& operator=(const T& t);

    basic_string& assign(size_type count, CharT ch)
    {
        clear();
        return append(count, ch);
    }
    basic_string& assign(const basic_string& str)
    {
        clear();
        return append(str);
    }
    basic_string& assign
        ( const basic_string& str
        , size_type pos
        , size_type count = npos )
    {
        clear();
        append(str, pos, count);
    }
    basic_string& assign(basic_string&& str)
    {
        swap(str);
        return *this;
    }
    basic_string& assign(const CharT* s, size_type count)
    {
        clear();
        return append(s, count);
    }
    basic_string& assign(const CharT* s)
    {
        clear();
        return append(s);
    }

    // template< class InputIt >
    // basic_string& assign(InputIt first, InputIt last);

    // basic_string& assign(std::initializer_list<CharT> ilist);

    // template < class T >
    // basic_string& assign(const T& t);

    // template < class T >
    // basic_string& assign(const T& t, size_type pos, size_type count = npos);


    allocator_type get_allocator() const
    {
        return m_allocator;
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
        return big() ? m_data.big.str : m_data.small.str;
    }
    pointer data()
    {
        return big() ? m_data.big.str : m_data.small.str;
    }
    const_pointer c_str() const
    {
        return data();
    }
    // operator std::basic_string_view<CharT, Traits>() const noexcept;

    operator speudo_std::basic_api_string<CharT>() const &
    {
        return basic_string{*this}.move_to_api_string();
    }

    operator speudo_std::basic_api_string<CharT>() &&
    {
        return std::move(*this).move_to_api_string();
    }

    operator speudo_std::basic_api_string<CharT>() const &&
    {
        return basic_string{*this}.move_to_api_string();
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
        return big()
            ? (m_data.big.str + m_data.big.len)
            : (m_data.small.str + m_data.small.len);
    }
    const_iterator end() const
    {
        return big()
            ? (m_data.big.str + m_data.big.len)
            : (m_data.small.str + m_data.small.len);
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
        return big() ? m_data.big.len : m_data.small.len;
    }
    size_type size() const
    {
        return big() ? m_data.big.len : m_data.small.len;
    }
    size_type capacity() const
    {
        return big() ? m_data.big.capacity : m_data.small.capacity();
    }
    bool empty() const
    {
        return length() == 0;
    }
    void reserve(size_type new_cap)
    {
        if (new_cap > capacity()) {
            replace_memory(data(), length(), new_cap);
        }
    }
    size_type max_size() const
    {
        return memory_creator::max_bytes_size(m_allocator) / sizeof(CharT) - 1;
    }
    void shrink_to_fit();

    //
    // Operations
    //

    void clear();

    // basic_string& insert( size_type index, size_type count, CharT ch );

    // basic_string& insert( size_type index, const CharT* s );

    // basic_string& insert( size_type index, const CharT* s, size_type count );

    // basic_string& insert( size_type index, const basic_string& str );

    // basic_string& insert
    //     ( size_type index
    //     , const basic_string& str
    //     , size_type index_str
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

    basic_string& append( const basic_string& str )
    {
        return append(str.data(), str.length());
    }
    basic_string& append
        ( const basic_string& str
        , size_type pos
        , size_type count = npos )
    {
        return append(&str.at(pos), std::min(count, str.length() - pos));
    }
    basic_string& append( const CharT* s, size_type count );

    basic_string& append( const CharT* s )
    {
        return append(s, Traits::length(s));
    }

    // template< class InputIt >
    // basic_string& append( InputIt first, InputIt last );

    // basic_string& append( std::initializer_list<CharT> ilist );

    // template< class T >
    // basic_string& append( const T& t );

    // template< class T >
    // basic_string& append
    //     ( const T& t
    //     , size_type pos
    //     , size_type count = npos );

    basic_string& operator+=( const basic_string& str )
    {
        return append(str);
    }
    basic_string& operator+=( CharT ch )
    {
        return push_back(ch);
    }
    basic_string& operator+=( const CharT* s )
    {
        return append(s);
    }
    // basic_string& operator+=( std::initializer_list<CharT> ilist )
    // {
    //     return append(ilist)
    // }
    // basic_string& operator+=( std::basic_string_view<CharT, Traits> sv)
    // {
    //     return append(&sv.front(), sv.size());
    // }
    int compare( const basic_string& str ) const
    {
        return do_compare(data(), size(), str.data(), str.size());
    }
    int compare( size_type pos1, size_type count1, const basic_string& str ) const
    {
        return do_compare
            ( &at(pos1)
            , std::min(count1, size() - pos1)
            , str.data()
            , str.size());              
    }
    // int compare
    //     ( size_type pos1
    //     , size_type count1
    //     , const basic_string& str
    //     , size_type pos2
    //     , size_type count2 = npos ) const;

    int compare( const CharT* s ) const
    {
        return do_compare(data(), size(), s, Traits::length(s));
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
    //     , const basic_string& str );

    // basic_string& replace
    //     ( const_iterator first
    //     , const_iterator last
    //     , const basic_string& str );

    // basic_string& replace
    //     ( size_type pos
    //     , size_type count
    //     , const basic_string& str
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

    static constexpr bool is_swap_noexcept
        = std::allocator_traits<Allocator>::propagate_on_container_swap::value;
//     || std::allocator_traits<Allocator>::is_always_equal::value;

    void swap(basic_string& other) noexcept(is_swap_noexcept);

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

    speudo_std::basic_api_string<CharT> move_to_api_string() &&;
    
    static int do_compare
        ( const CharT* s1
        , size_type len1
        , const CharT* s2
        , size_type len2
        );

    constexpr static size_type min_capacity_diff =
        sizeof(speudo_std::private_::api_string_mem<Allocator>)
        / sizeof(CharT);

    bool big() const
    {
        return m_data.big.str != nullptr;
    }
    bool small() const
    {
        return m_data.big.str != nullptr;
    }
    bool ok_to_shrink() const
    {
        return big() && m_data.big.capacity > 2 * m_data.big.len;
    }

    size_t grow_cap_if_necessary_for(size_t len_growth);

    void replace_memory(const CharT* new_str, size_type new_len, size_type new_cap);


    constexpr void reset_data()
    {
        if(sizeof(m_data.big) > sizeof(m_data.small))
        {
            m_data.big = {0};
        }
        else
        {
            m_data.small = {0};
        }
    }

    Allocator m_allocator;

    union data_type
    {
        struct
        {
            size_type len;
            size_type capacity;
            speudo_std::abi::api_string_mem_base* mem_manager;
            CharT* str;
        } big;

        struct
        {   // for small string optimizatiom
            constexpr static size_type capacity()
            {
                constexpr size_type c = (2 * sizeof(size_type) + sizeof(void*)) / sizeof(CharT);
                return c == 0 ? 0 : c - 1;
            }
            unsigned char len;
            CharT str[capacity() + 1];
        } small;
    };

    data_type m_data;
};


// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( const CharT* lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( CharT lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , const CharT* rhs );

// template<class CharT, class Traits, class Alloc>
// basic_string<CharT,Traits,Alloc> operator+
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , CharT rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( basic_string<CharT,Traits,Alloc>&& lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , basic_string<CharT,Traits,Alloc>&& rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( basic_string<CharT,Traits,Alloc>&& lhs
//     , basic_string<CharT,Traits,Alloc>&& rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     (const CharT* lhs
//     , basic_string<CharT,Traits,Alloc>&& rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( CharT lhs
//     , basic_string<CharT,Traits,Alloc>&& rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( basic_string<CharT,Traits,Alloc>&& lhs
//     , const CharT* rhs );

// template< class CharT, class Traits, class Alloc >
// basic_string<CharT,Traits,Alloc> operator+
//     ( basic_string<CharT,Traits,Alloc>&& lhs
//     , CharT rhs );


// template< class CharT, class Traits, class Alloc >
// bool operator==
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator!=
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator<
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator<=
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator>
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator>=
//     ( const basic_string<CharT,Traits,Alloc>& lhs
//     , const basic_string<CharT,Traits,Alloc>& rhs );


// template< class CharT, class Traits, class Alloc >
// bool operator==( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator==( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator!=( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator!=( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator<( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator<( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator<=( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator<=( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator>( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator>( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator>=( const CharT* lhs, const basic_string<CharT,Traits,Alloc>& rhs );

// template< class CharT, class Traits, class Alloc >
// bool operator>=( const basic_string<CharT,Traits,Alloc>& lhs, const CharT* rhs );

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;



} // namespace speudo_std


#include <private/string.tcc>

#endif
