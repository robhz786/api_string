#include <memory>


class allocator_log
{
public:

    allocator_log() = default;
    allocator_log(const allocator_log&) = default;

    void report_construction() { ++ m_construction_count; }
    void report_destruction() { ++ m_destruction_count; }
    void report_allocation() { ++ m_allocation_count; }
    void report_deallocation() { ++ m_deallocation_count; }
    unsigned constructions_count(){ return m_construction_count; }
    unsigned destructions_count(){ return m_destruction_count; }
    unsigned allocations_count(){ return m_allocation_count; }
    unsigned deallocations_count(){ return m_deallocation_count; }

private:

    unsigned m_construction_count = 0;
    unsigned m_destruction_count = 0;
    unsigned m_allocation_count = 0;
    unsigned m_deallocation_count = 0;
};


template< typename T
        , typename PropOnCopyAssig = std::true_type
        , typename PropOnMoveAssig = std::true_type
        , typename PropOnSwap = std::true_type >
class custom_allocator
{
public:

    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type= std::ptrdiff_t;

    using propagate_on_container_copy_assignment = PropOnCopyAssig;
    using propagate_on_container_move_assignment = PropOnMoveAssig;
    using propagate_on_container_swap = PropOnSwap;
    using is_always_equal = std::false_type;

    template< class U >
    struct rebind
    {
        typedef
            custom_allocator<U, PropOnCopyAssig, PropOnMoveAssig, PropOnSwap>
            other;
    };

    constexpr custom_allocator() = default;
    constexpr custom_allocator(const custom_allocator&) = default;

    template <typename U, typename PCP, typename PMV, typename PSW>
    constexpr custom_allocator(const custom_allocator<U, PCP, PMV, PSW>& other)
        : m_log(other.m_log)
    {
    }

    constexpr custom_allocator(allocator_log& log)
        : m_log(&log)
    {
    }


    constexpr static custom_allocator select_on_container_copy_construction
        ( const custom_allocator& rhs )
    {
        return rhs;
    }

    T* allocate( std::size_t n )
    {
        if(m_log != nullptr)
        {
            m_log->report_allocation();
        }
        std::allocator<T> a;
        return a.allocate(n);
    }

    void deallocate( T* p, std::size_t n )
    {
        if(m_log != nullptr)
        {
            m_log->report_deallocation();
        }
        std::allocator<T> a;
        return a.deallocate(p, n);
    }
		
		
    template< class U, class... Args >
    void construct( U* p, Args&&... args )
    {
        if(m_log != nullptr)
        {
            m_log->report_construction();
        }
        ::new((void *)p) U(std::forward<Args>(args)...);
    }

    template< class U >
    void destroy( U* p )
    {
        if(m_log != nullptr)
        {
            m_log->report_destruction();
        }
        p->~U();
    }

    unsigned constructions_count() const
    {
        return m_log ? m_log->constructions_count() : 0;
    }
    unsigned destructions_count() const
    {
        return m_log ? m_log->destructions_count() : 0;
    }
    unsigned allocations_count() const
    {
        return m_log ? m_log->allocations_count() : 0;
    }
    unsigned deallocations_count() const
    {
        return m_log ? m_log->deallocations_count() : 0;
    }
    template <class T2, typename PCP, typename PMV, typename PSW>
    bool operator==(const custom_allocator<T2, PCP, PMV, PSW>& rhs) const
    {
        return m_log == rhs.m_log;
    }

    template <class T2, typename PCP, typename PMV, typename PSW>
    bool operator!=(const custom_allocator<T2, PCP, PMV, PSW>& rhs) const
    {
        return m_log != rhs.m_log;
    }

private:

    template <class, class, class, class>
    friend class custom_allocator;

    allocator_log* m_log = nullptr;
};

