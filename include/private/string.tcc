#ifndef SPEUDO_STD_PRIVATE_STRING_TCC
#define SPEUDO_STD_PRIVATE_STRING_TCC

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


namespace speudo_std {

// template <typename CharT, typename Traits, typename Allocator>
// basic_string<CharT, Traits, Allocator>::basic_string
//     ( basic_string::size_type count
//     , CharT ch
//     , const Allocator& alloc
//     )
//     : m_allocator(alloc)
// {
//     reset_data();
//     if (count <= m_data.small.capacity())
//     {
//         Traits::assign(m_data.small.str, count, ch);
//         Traits::assign(m_data.small.str[count], CharT{});
//         m_data.small.len = count;
//     }
//     else
//     {
//         reserve(count * 2);
//         Traits::assign(m_data.big.str, count, ch);
//         Traits::assign(m_data.big.str[count], CharT{});
//         m_data.big.len = count;
//     }
// }


// template <typename CharT, typename Traits, typename Allocator>
// basic_string<CharT, Traits, Allocator>::basic_string
//     ( const CharT* s
//     , basic_string::size_type count
//     , const Allocator& alloc
//     )
//     : m_allocator(alloc)
// {
//     reset_data();
//     if (count <= m_data.small.capacity())
//     {
//         m_data.small.len = static_cast<decltype(m_data.small.len)>(count);
//         Traits::copy(m_data.small.str, s, count);
//         Traits::assign(m_data.small.str[count], CharT{});
//     }
//     else
//     {
//         replace_memory(s, count, 2 * count);
//     }
// }

//
// Element access
//
template <typename CharT, typename Traits, typename Allocator>
CharT& basic_string<CharT, Traits, Allocator>::at(size_type pos)
{
    if (pos >= size())
    {
        throw std::out_of_range("speudo_std::at() out of range");
    }
    return data()[pos];
}

template <typename CharT, typename Traits, typename Allocator>
const CharT& basic_string<CharT, Traits, Allocator>::at(size_type pos) const
{
    if (pos >= size())
    {
        throw std::out_of_range("speudo_std::at() out of range");
    }
    return data()[pos];
}



//
// Operations
//

template <typename CharT, typename Traits, typename Allocator>
void basic_string<CharT, Traits, Allocator>::resize(size_type count, CharT ch)
{
    if(count < size())
    {
        if(big())
        {
            m_data.big.len = count;
            Traits::assign(m_data.big.str[count], CharT{});
        }
        else
        {
            m_data.small.len = count;
            Traits::assign(m_data.small.str[count], CharT{});
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
    if(big())
    {
        if (m_data.big.len <= m_data.small.capacity())
        {
            basic_string tmp{m_data.big.str, m_data.big.len};
            swap(tmp);
        }
        else if(m_data.big.capcity - m_data.big.len > min_capacity_diff)
        {
            replace_memory(m_data.big.str, m_data.big.len, m_data.big.len);
        }
    }
}

template <typename CharT, typename Traits, typename Allocator>
inline void basic_string<CharT, Traits, Allocator>::clear()
{
    if(big())
    {
        m_data.big.len = 0;
        m_data.big.str[0] = 0;
    }
    else
    {
        reset_data();
    }
}

namespace private_{

template <typename T>
inline void swap_if(T& a, T&b, std::false_type)
{
}

template <typename T>
inline void swap_if(T& a, T&b, std::true_type)
{
    std::swap(a, b);
}

}

template <typename CharT, typename Traits, typename Allocator>
inline void basic_string<CharT, Traits, Allocator>::swap(basic_string& other)
    noexcept(basic_string::is_swap_noexcept)
{
    data_type tmp = other.m_data;
    other.m_data = m_data;
    m_data = tmp;

    speudo_std::private_::swap_if
        ( m_allocator
        , other.m_allocator
        , prop_alloc_on_swap{}
        );
}

//
// Operations
//

template <typename CharT, typename Traits, typename Allocator>
speudo_std::basic_api_string<CharT>
basic_string<CharT, Traits, Allocator>::move_to_api_string() &&
{
    speudo_std::basic_api_string<CharT> dest;
    auto& d = speudo_std::private_::basic_string_helper::get_data(dest);
    if (big())
    {
        d.big.len = m_data.big.len;
        d.big.mem_manager = m_data.big.mem_manager;
        d.big.str = m_data.big.str;
        reset_data();
    }
    else if (m_data.small.len <= d.small.capacity())
    {
        d.small.len = m_data.small.len;
        Traits::copy(d.small.str, m_data.small.str, m_data.small.len);
        Traits::assign(d.small.str[m_data.small.len], CharT{});
    }
    else
    {
        auto m = memory_creator::create(m_allocator, (m_data.small.len + 1) * sizeof(CharT));
        CharT * str = reinterpret_cast<CharT*>(m.pool);
        
        d.big.len = m_data.small.len;
        d.big.mem_manager = m.manager;
        d.big.str = str;
        Traits::copy(str, m_data.small.str, m_data.small.len);
        Traits::assign(str[m_data.small.len], CharT{});
    }
    return dest;
}



template <typename CharT, typename Traits, typename Allocator>
CharT basic_string<CharT, Traits, Allocator>::pop_back()
{
    if(big())
    {
        if (m_data.big.len > 0)
        {
            CharT value = m_data.big.str[m_data.big.len - 1];
            -- m_data.big.len;
            return value;
        }
    }
    else if (m_data.small.len > 0)
    {
        CharT value = m_data.small.str[m_data.small.len - 1];
        -- m_data.small.len;
        return value;
    }

    return 0;
}


template <typename CharT, typename Traits, typename Allocator>
void basic_string<CharT, Traits, Allocator>::push_back(CharT ch)
{
    grow_cap_if_necessary_for(1);
    if(big())
    {
        Traits::assign(m_data.big.str[m_data.big.len + 1], CharT());
        Traits::assign(m_data.big.str[m_data.big.len], ch);
        ++ m_data.big.len;
    }
    else
    {
        Traits::assign(m_data.small.str[m_data.small.len + 1], CharT());
        Traits::assign(m_data.small.str[m_data.small.len], ch);
        ++ m_data.small.len;
    }

}

template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>&
basic_string<CharT, Traits, Allocator>::append( size_type count, CharT ch )
{
    grow_cap_if_necessary_for(count);
    if(big())
    {
        Traits::assign(m_data.big.str[m_data.big.len + count], CharT());
        Traits::assign(m_data.big.str[m_data.big.len], count, ch);
        m_data.big.len += count;
    }
    else
    {
        Traits::assign(m_data.small.str[m_data.small.len + count], CharT());
        Traits::assign(m_data.small.str[m_data.small.len], count, ch);
        m_data.small.len += count;
    }
    return *this;
}

template <typename CharT, typename Traits, typename Allocator>
basic_string<CharT, Traits, Allocator>&
basic_string<CharT, Traits, Allocator>::append( const CharT* s, size_type count )
{
    grow_cap_if_necessary_for(count);
    if(big())
    {
        Traits::assign(m_data.big.str[m_data.big.len + count], CharT());
        Traits::copy(m_data.big.str + m_data.big.len, s, count);
        m_data.big.len += count;
    }
    else
    {
        Traits::assign(m_data.small.str[m_data.small.len + count], CharT());
        Traits::copy(m_data.small.str + m_data.small.len, s, count);
        m_data.small.len += count;
    }
    return *this;
}






//
// private functions
//

template <typename CharT, typename Traits, typename Allocator>
int basic_string<CharT, Traits, Allocator>::do_compare
    ( const CharT* s1
    , size_type len1
    , const CharT* s2
    , size_type len2
    )
{
    int cmp = Traits::compare(s1, s2, std::min(len1, len2));
    return cmp != 0 ? cmp : (len1 == len2 ? 0 : (len1 < len2 ? -1 : +1));
}


template <typename CharT, typename Traits, typename Allocator>
size_t basic_string<CharT, Traits, Allocator>::grow_cap_if_necessary_for(size_t len_growth)
{
    if (length() + len_growth > capacity())
    {
        size_type new_cap = 2 * (length() + len_growth);
        replace_memory(data(), length(), new_cap);
    }
}

template <typename CharT, typename Traits, typename Allocator>
void basic_string<CharT, Traits, Allocator>::replace_memory
    ( const CharT* new_str
    , basic_string::size_type new_len
    , basic_string::size_type new_cap
    )
{
    assert(new_cap >= new_len);

    auto m = memory_creator::create(m_allocator, (new_cap + 1) * sizeof(CharT));

    basic_string tmp;
    tmp.m_data.big.len = new_len;
    tmp.m_data.big.capacity = m.bytes_capacity / sizeof(CharT) - 1;
    tmp.m_data.big.mem_manager = m.manager;
    tmp.m_data.big.str = reinterpret_cast<CharT*>(m.pool);
    Traits::copy(tmp.m_data.big.str, new_str, new_len);
    Traits::assign(tmp.m_data.big.str[new_len], CharT{});

    swap(tmp);
}

} // namespace speudo_std

#endif
