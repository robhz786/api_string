#ifndef SPEUDO_STD_DETAIL_API_STRING_MEMORY_HPP
#define SPEUDO_STD_DETAIL_API_STRING_MEMORY_HPP

// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <api_string.hpp>
#include <atomic>
#include <memory>

namespace speudo_std {

namespace api_string_test {

#if defined(API_STRING_TEST_MODE)
void report_allocation();
void report_deallocation();
#else
inline void report_allocation(){}
inline void report_deallocation(){}
#endif

} // namespace api_string_test

namespace _detail {

constexpr std::size_t api_string_mem_alignment
    = alignof(speudo_std::abi::api_string_mem_base) > alignof(char32_t)
    ? alignof(speudo_std::abi::api_string_mem_base)
    : alignof(char32_t);

template <typename Allocator>
class alignas(speudo_std::_detail::api_string_mem_alignment) api_string_mem
    : public speudo_std::abi::api_string_mem_base
    , private Allocator
{
    using rebinded_allocator_type
        = typename std::allocator_traits<Allocator>
        :: template rebind_alloc<api_string_mem>;

    using rebinded_allocator_traits
        = std::allocator_traits<rebinded_allocator_type>;

    using size_type = typename Allocator::size_type;
    using allocator_type = Allocator;

public:

    api_string_mem(Allocator a, std::byte* end)
        : speudo_std::abi::api_string_mem_base{get_table()}
        , Allocator(a)
        , _end(end)
    {
    }

    struct memory
    {
        speudo_std::abi::api_string_mem_base* manager;
        std::byte* pool;
        size_type pool_size;
    };

    static memory create(const Allocator& a, size_type bytes_capacity)
    {
        size_type array_size
            = (bytes_capacity + 2 * sizeof(api_string_mem) - 1)
            / sizeof(api_string_mem);

        size_type appended_array_size
            = (bytes_capacity + sizeof(api_string_mem) - 1)
            / sizeof(api_string_mem);

        bytes_capacity = appended_array_size * sizeof(api_string_mem);

        rebinded_allocator_type r_allocator(a);
        api_string_mem* self
            = rebinded_allocator_traits::allocate(r_allocator, array_size);
        std::byte* end = reinterpret_cast<std::byte*>(self + array_size);
        rebinded_allocator_traits::construct(r_allocator, self, a, end);

        speudo_std::api_string_test::report_allocation();

        return { self
               , reinterpret_cast<std::byte*>(self + 1)
               , (array_size - 1) * sizeof(api_string_mem)};
    }

    static size_type max_bytes_size(const Allocator& a)
    {
        rebinded_allocator_type rb(a);
        return
            (rebinded_allocator_traits::max_size(a) - 1)
            * sizeof(typename rebinded_allocator_traits::value_type);
    }

private:

    std::atomic<std::size_t> _refcount{1};
    std::byte* _end;

    Allocator& get_allocator()
    {
        return *this;
    }

    static std::size_t acquire(api_string_mem_base* mem_base)
    {
        auto* self = static_cast<api_string_mem*>(mem_base);
        return self->_refcount.fetch_add(1, std::memory_order_relaxed);
    }

    static void release(api_string_mem_base* mem_base)
    {
        auto* self = static_cast<api_string_mem*>(mem_base);
        if (self->_refcount.fetch_sub(1, std::memory_order_release) == 1) {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete_self(self);
        }
    }

    static bool unique(api_string_mem_base* mem_base)
    {
        auto* self = static_cast<api_string_mem*>(mem_base);
        return self->_refcount.load() == 1;
    }

    static std::byte* begin(api_string_mem_base* mem_base)
    {
        return reinterpret_cast<std::byte*>(mem_base + 1);
    }

    static std::byte* end(api_string_mem_base* mem_base)
    {
        auto* self = static_cast<api_string_mem*>(mem_base);
        return reinterpret_cast<std::byte*>(self->_end);
    }

    static const speudo_std::abi::api_string_func_table* get_table()
    {
        static const speudo_std::abi::api_string_func_table table =
            {0, acquire, release, unique, begin, end};
        return & table;
    }

    static void delete_self(api_string_mem_base* mem_base)
    {
        auto* self = static_cast<api_string_mem*>(mem_base);
        rebinded_allocator_type r_allocator{self->get_allocator()};
        size_type count = reinterpret_cast<api_string_mem*>(self->_end) - self;
        rebinded_allocator_traits::destroy(r_allocator, self);
        rebinded_allocator_traits::deallocate(r_allocator, self, count);

        speudo_std::api_string_test::report_deallocation();
    }
};

extern template class api_string_mem<std::allocator<char>>;
extern template class api_string_mem<std::allocator<char16_t>>;
extern template class api_string_mem<std::allocator<char32_t>>;
extern template class api_string_mem<std::allocator<wchar_t>>;


template
    < typename CharT
    , typename Traits = std::char_traits<CharT>
    , typename Allocator = std::allocator<CharT> >
void api_string_init_impl
    ( speudo_std::abi::api_string_data<CharT>& data
    , const CharT* src
    , std::size_t count
    , const Allocator a = Allocator{} )
{
    speudo_std::abi::reset(data);

    if(count > data.small_capacity())
    {
        auto mem = api_string_mem<Allocator>::create(a, sizeof(CharT) * (count + 1));

        CharT* str = reinterpret_cast<CharT*>(mem.pool);
        Traits::copy(str, src, count);
        Traits::assign(str[count], CharT{});

        data.big.len = count;
        data.big.str = str;
        data.big.mem_manager = mem.manager;
    }
    else if(count > 0)
    {
        // small string optimization
        data.small.len = static_cast<decltype(data.small.len)>(count);
        Traits::copy(data.small.str, src, count);
        Traits::assign(data.small.str[count], CharT{});
    }
}


} // namespace _detail
} // namespace speudo_std

#endif
