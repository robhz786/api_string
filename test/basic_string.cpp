#include <gtest/gtest.h>
#include <string.hpp>
#include "custom_allocator.hpp"

template <typename StringType, typename CharT = typename StringType::value_type>
void test_empty(const StringType& str)
{
    EXPECT_TRUE(str.empty());
    EXPECT_EQ(str.size(), 0);
    EXPECT_EQ(str.length(), 0);
    EXPECT_EQ(str.c_str()[0], CharT{});
    EXPECT_EQ(str.begin(), str.end());
}

template <typename StringType, typename CharT>
void test_equal
    ( const StringType& str
    , const CharT* cstr
    , std::size_t len )
{
    EXPECT_EQ(str.size(), len);
    EXPECT_EQ(str.length(), len);
    EXPECT_EQ(str.c_str()[len], CharT{});
    EXPECT_EQ(str.begin() + len, str.end());
}


template <typename CharT>
class basic_fixture: public ::testing::Test
{
public:

    basic_fixture()
    {
        speudo_std::api_string_test::reset();
        fill(m_small_buff, small_raw_string_len());
        fill(m_big_buff, big_raw_string_len());
        fill(m_bigger_buff, even_bigger_raw_string_len());
    }

    using char_type = CharT;

    using string_type = speudo_std::basic_string
        < CharT
        , std::char_traits<CharT>
        , custom_allocator<CharT> >;

    constexpr static std::size_t sso_capacity()
    {
        return string_type::sso_capacity;
    }

    constexpr static std::size_t small_raw_string_len()
    {
        return sso_capacity();
    }
    constexpr static std::size_t big_raw_string_len()
    {
        return sso_capacity() + 1;
    }
    constexpr static std::size_t even_bigger_raw_string_len()
    {
        return 5 * sso_capacity();
    }

    const CharT* small_raw_string() const
    {
        return m_small_buff;
    }

    const CharT* big_raw_string() const
    {
        return m_big_buff;
    }

    const CharT* even_bigger_raw_string() const
    {
        return m_bigger_buff;
    }

    static void fill(CharT* str, std::size_t len)
    {
        CharT c = ' ';
        for (int i = 0; i < len; ++i)
        {
            str[i] = c;
            c = (c == 'z') ? ' ' : c + 1;
        }
        str[len] = 0;
    }

private:

    CharT m_small_buff[small_raw_string_len() + 1];
    CharT m_big_buff[big_raw_string_len() + 1];
    CharT m_bigger_buff[even_bigger_raw_string_len() + 1];
};

using all_char_types = ::testing::Types<char, wchar_t, char16_t, char32_t>;

TYPED_TEST_CASE(basic_fixture, all_char_types);

TYPED_TEST(basic_fixture, default_constructor)
{
    using char_type = typename TestFixture::char_type;
    //using string_type = typename TestFixture::string_type;

    speudo_std::basic_string<char_type> str;

    test_empty(str);

    EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 0);
    EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 0);
}

TYPED_TEST(basic_fixture, custom_allocator)
{
    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    allocator_log log;
    custom_allocator<char_type> a(log);

    // create an empty string
    {
        str_type str(a);
        EXPECT_EQ(str.get_allocator(), a);

        EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 0);
        EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 0);
        EXPECT_EQ(log.allocations_count(), 0);
        EXPECT_EQ(log.deallocations_count(), 0);

        // resize it
        str.append(this->big_raw_string());

        EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 1);
        EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 0);
        EXPECT_EQ(log.allocations_count(), 1);
        EXPECT_EQ(log.deallocations_count(), 0);
    }

    EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 1);
    EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 1);
    EXPECT_EQ(log.allocations_count(), 1);
    EXPECT_EQ(log.deallocations_count(), 1);
}

TYPED_TEST(basic_fixture, construct_from_char)
{
    // basic_string::basic_string
    //     ( size_type count
    //     , CharT ch
    //     , const Allocator& alloc = Allocator() )

    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    allocator_log log;
    custom_allocator<char_type> a(log);

    {
        char_type ch('x');

        str_type empty_str(0, ch, a);
        EXPECT_EQ(empty_str.get_allocator(), a);
        test_empty(empty_str);

        str_type small_str(this->sso_capacity(), ch, a);
        EXPECT_EQ(small_str.size(), this->sso_capacity());
        EXPECT_EQ(std::count(small_str.begin(), small_str.end(), ch), small_str.size());

        str_type big_str(this->sso_capacity() + 1, ch, a);
        EXPECT_EQ(big_str.size(), this->sso_capacity() + 1);
        EXPECT_EQ(std::count(big_str.begin(), big_str.end(), ch), big_str.size());

        EXPECT_EQ(log.allocations_count(), 1);
        EXPECT_EQ(log.deallocations_count(), 0);
    }

    EXPECT_EQ(log.deallocations_count(), 1);
}

TYPED_TEST(basic_fixture, construct_from_pos)
{
    // basic_string::basic_string
    //     ( const basic_string& other
    //     , size_type pos
    //     , const Allocator& alloc = Allocator() )

    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    allocator_log log;
    custom_allocator<char_type> a(log);

    {
        str_type input(this->even_bigger_raw_string());

        str_type empty_str(input, input.size(), a);
        EXPECT_EQ(empty_str.get_allocator(), a);
        test_empty(empty_str);

        std::size_t pos = input.size() - this->sso_capacity();
        str_type small_str(input, pos, a);
        EXPECT_EQ(small_str.size(), this->sso_capacity());
        EXPECT_EQ(small_str, input.c_str() + pos);

        pos = 1;
        str_type big_str(input, pos, a);
        EXPECT_EQ(big_str.size(), input.size() - pos);
        EXPECT_EQ(big_str, input.c_str() + pos);

        EXPECT_EQ(log.deallocations_count(), 0);
    }
    EXPECT_EQ(log.allocations_count(), 1);
    EXPECT_EQ(log.deallocations_count(), 1);
}

TYPED_TEST(basic_fixture, construct_from_slice)
{
    // basic_string::basic_string
    //     ( const basic_string& other
    //     , size_type pos
    //     , size_type count
    //     , const Allocator& alloc = Allocator() )

    using char_type = typename TestFixture::char_type;
    using traits = std::char_traits<char_type>;
    using str_type = speudo_std::basic_string< char_type
                                             , traits
                                             , custom_allocator<char_type> >;
    allocator_log log;
    custom_allocator<char_type> a(log);
    {
        str_type input(this->even_bigger_raw_string());

        const std::size_t pos = 1;
        str_type empty_str(input, pos, 0, a);
        EXPECT_EQ(empty_str.get_allocator(), a);
        test_empty(empty_str);

        str_type small_str(input, pos, this->sso_capacity(), a);
        ASSERT_EQ(small_str.size(), this->sso_capacity());
        EXPECT_EQ(traits::compare(&small_str[0], &input[pos], small_str.size()), 0);

        EXPECT_EQ(log.allocations_count(), 0);
        str_type big_str(input, pos, this->sso_capacity() + 1, a);
        ASSERT_EQ(big_str.size(), this->sso_capacity() + 1);
        EXPECT_EQ(traits::compare(&big_str[0], &input[pos], big_str.size()), 0);


        EXPECT_EQ(log.deallocations_count(), 0);
    }
    EXPECT_EQ(log.allocations_count(), 1);
    EXPECT_EQ(log.deallocations_count(), 1);
}


TYPED_TEST(basic_fixture, construct_from_truncated_raw_string)
{
    // basic_string::basic_string
    //     ( const CharT* s
    //     , size_type count
    //     , const Allocator& alloc = Allocator() )

    using char_type = typename TestFixture::char_type;
    using traits = std::char_traits<char_type>;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    allocator_log log;
    custom_allocator<char_type> a(log);

    const char_type* input = this->even_bigger_raw_string();

    {
        str_type empty_str(input, 0, a);
        EXPECT_EQ(empty_str.get_allocator(), a);
        test_empty(empty_str);

        str_type small_str(input, this->sso_capacity(), a);
        ASSERT_EQ(small_str.size(), this->sso_capacity());
        EXPECT_EQ(traits::compare(&small_str[0], input, small_str.size()), 0);

        EXPECT_EQ(log.allocations_count(), 0);
        str_type big_str(input, this->sso_capacity() + 1, a);
        ASSERT_EQ(big_str.size(), this->sso_capacity() + 1);
        EXPECT_EQ(traits::compare(&big_str[0], input, small_str.size()), 0);

        EXPECT_EQ(log.allocations_count(), 1);
    }
    EXPECT_EQ(log.deallocations_count(), 1);
    EXPECT_EQ(log.destructions_count(), 1);
}

TYPED_TEST(basic_fixture, construct_from_raw_string)
{
    // basic_string::basic_string
    //     ( const CharT* s
    //     , const Allocator& alloc = Allocator() )

    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    allocator_log log;
    custom_allocator<char_type> a(log);
    {
        char_type ch0 = 0;
        str_type empty_str(&ch0);
        test_empty(empty_str);

        str_type small_str(this->small_raw_string(), a);
        ASSERT_EQ(small_str, this->small_raw_string());

        EXPECT_EQ(log.allocations_count(), 0);
        str_type big_str(this->big_raw_string(), a);
        ASSERT_EQ(big_str, this->big_raw_string());
        EXPECT_EQ(log.allocations_count(), 1);
    }
    EXPECT_EQ(log.deallocations_count(), 1);
}

TYPED_TEST(basic_fixture, construct_copy)
{
    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    allocator_log log;
    custom_allocator<char_type> a(log);


    str_type small_str_1(this->small_raw_string(), a);
    str_type small_str_2(small_str_1);
    EXPECT_EQ(small_str_2.get_allocator(), a);
    EXPECT_EQ(small_str_1, small_str_2);
    EXPECT_EQ(log.allocations_count(), 0);


    str_type big_str_1(this->big_raw_string(), a);
    str_type big_str_2(big_str_1);
    EXPECT_EQ(big_str_2.get_allocator(), a);
    EXPECT_EQ(big_str_1, big_str_2);
    EXPECT_EQ(log.allocations_count(), 2);
}

TYPED_TEST(basic_fixture, construct_copy_with_allocator)
{
    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    allocator_log log1;
    custom_allocator<char_type> a1(log1);

    allocator_log log2;
    custom_allocator<char_type> a2(log2);

    EXPECT_NE(a1, a2);

    str_type small_str_1(this->small_raw_string(), a1);
    str_type small_str_2(small_str_1, a2);
    EXPECT_EQ(small_str_2.get_allocator(), a2);
    EXPECT_EQ(log2.allocations_count(), 0);
    EXPECT_EQ(small_str_1, small_str_2);

    str_type big_str_1(this->big_raw_string(), a1);
    str_type big_str_2(big_str_1, a2);
    EXPECT_EQ(big_str_2.get_allocator(), a2);
    EXPECT_EQ(log1.allocations_count(), 1);
    EXPECT_EQ(log2.allocations_count(), 1);
    EXPECT_EQ(big_str_1, big_str_2);
}

TYPED_TEST(basic_fixture, construct_move)
{
    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    allocator_log log;
    custom_allocator<char_type> a(log);

    str_type small_str_1(this->small_raw_string(), a);
    str_type small_str_2(std::move(small_str_1));
    EXPECT_EQ(small_str_2.get_allocator(), a);
    EXPECT_EQ(small_str_2, this->small_raw_string());
    EXPECT_EQ(log.allocations_count(), 0);

    str_type big_str_1(this->big_raw_string(), a);
    str_type big_str_2(std::move(big_str_1));
    EXPECT_EQ(big_str_2, this->big_raw_string());
    EXPECT_EQ(big_str_2.get_allocator(), a);
    EXPECT_EQ(log.allocations_count(), 1);
    test_empty(big_str_1);
}


TYPED_TEST(basic_fixture, construct_move_with_allocator)
{
    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    allocator_log log1;
    custom_allocator<char_type> a1(log1);

    allocator_log log2;
    custom_allocator<char_type> a2(log2);

    EXPECT_NE(a1, a2);

    str_type small_str_1(this->small_raw_string(), a1);
    str_type small_str_2(std::move(small_str_1), a2);
    EXPECT_EQ(small_str_2.get_allocator(), a2);
    EXPECT_EQ(log2.allocations_count(), 0);
    EXPECT_EQ(small_str_2, this->small_raw_string());

    str_type big_str_1(this->big_raw_string(), a1);
    str_type big_str_2(std::move(big_str_1), a2);
    EXPECT_EQ(log1.allocations_count(), 1);
    EXPECT_EQ(log2.allocations_count(), 0);
    EXPECT_EQ(big_str_2, this->big_raw_string());
    test_empty(big_str_1);
}


TYPED_TEST(basic_fixture, assign_copy_with_allocator)
{
    using char_type = typename TestFixture::char_type;
    using allocator_type = custom_allocator<char_type>;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , allocator_type >;

    {   //from small to small
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type small_str_1(this->small_raw_string(), a1);
        str_type small_str_2(a2);
        small_str_2 = small_str_1;
        EXPECT_EQ(small_str_2.get_allocator(), a1);
        EXPECT_EQ(small_str_1, small_str_2);
        EXPECT_EQ(log1.allocations_count(), 0);
    }
    {   // from small to big
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type small_str(this->small_raw_string(), a1);
        str_type big_str(this->big_raw_string(), a2);
        big_str = small_str;
        EXPECT_EQ(big_str.get_allocator(), a1);
        EXPECT_EQ(big_str, small_str);
        EXPECT_EQ(log1.allocations_count(), 0);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
    {   // from big to small
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type big_str(this->big_raw_string(), a1);
        str_type small_str(this->small_raw_string(), a2);
        small_str = big_str;
        EXPECT_EQ(small_str.get_allocator(), a1);
        EXPECT_EQ(big_str, small_str);
        EXPECT_EQ(log1.allocations_count(), 2);
        EXPECT_EQ(log2.allocations_count(), 0);
    }
    {   // from big to bigger
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type big_str_1(this->big_raw_string(), a1);
        str_type big_str_2(this->even_bigger_raw_string() ,a2);
        big_str_2 = big_str_1;
        EXPECT_EQ(big_str_2.get_allocator(), a1);
        EXPECT_EQ(big_str_2, big_str_1);
        EXPECT_GE(log1.allocations_count(), 1u);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
    {   // from big to less big
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type big_str_1(this->even_bigger_raw_string(), a1);
        str_type big_str_2(this->big_raw_string() ,a2);
        big_str_2 = big_str_1;
        EXPECT_EQ(big_str_2.get_allocator(), a1);
        EXPECT_EQ(big_str_2, big_str_1);
        EXPECT_GE(log1.allocations_count(), 1u);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
}


TYPED_TEST(basic_fixture, assign_copy_without_allocator)
{
    using char_type = typename TestFixture::char_type;
    using allocator_no_copy_on_assign = custom_allocator<char_type, std::false_type>;
    using str_no_alloc_copy_on_assign = speudo_std::basic_string
        < char_type
        , std::char_traits<char_type>
        , allocator_no_copy_on_assign >;

    {   //from small to small
        allocator_log log1;
        allocator_log log2;
        allocator_no_copy_on_assign a1(log1);
        allocator_no_copy_on_assign a2(log2);

        str_no_alloc_copy_on_assign small_str_1(this->small_raw_string(), a1);
        str_no_alloc_copy_on_assign small_str_2(a2);
        small_str_2 = small_str_1;
        EXPECT_EQ(small_str_2.get_allocator(), a2);
        EXPECT_EQ(small_str_1, small_str_2);
        EXPECT_EQ(log2.allocations_count(), 0);
    }

    {   // from small to big
        allocator_log log1;
        allocator_log log2;
        allocator_no_copy_on_assign a1(log1);
        allocator_no_copy_on_assign a2(log2);

        str_no_alloc_copy_on_assign small_str(this->small_raw_string(), a1);
        str_no_alloc_copy_on_assign big_str(this->big_raw_string(), a2);
        big_str = small_str;
        EXPECT_EQ(big_str.get_allocator(), a2);
        EXPECT_EQ(big_str, small_str);
        EXPECT_EQ(log1.allocations_count(), 0);
        EXPECT_EQ(log2.allocations_count(), 1);
    }

    {   // from big to small
        allocator_log log1;
        allocator_log log2;
        allocator_no_copy_on_assign a1(log1);
        allocator_no_copy_on_assign a2(log2);

        str_no_alloc_copy_on_assign big_str(this->big_raw_string(), a1);
        str_no_alloc_copy_on_assign small_str(this->small_raw_string(), a2);
        small_str = big_str;
        EXPECT_EQ(small_str.get_allocator(), a2);
        EXPECT_EQ(big_str, small_str);
        EXPECT_EQ(log1.allocations_count(), 1);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
    {   // from big to bigger
        allocator_log log1;
        allocator_log log2;
        allocator_no_copy_on_assign a1(log1);
        allocator_no_copy_on_assign a2(log2);

        str_no_alloc_copy_on_assign big_str_1(this->big_raw_string(), a1);
        str_no_alloc_copy_on_assign big_str_2(this->even_bigger_raw_string() ,a2);
        big_str_2 = big_str_1;
        EXPECT_EQ(big_str_2.get_allocator(), a2);
        EXPECT_EQ(big_str_2, big_str_1);
        EXPECT_EQ(log1.allocations_count(), 1);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
    {   // from big to less big
        allocator_log log1;
        allocator_log log2;
        allocator_no_copy_on_assign a1(log1);
        allocator_no_copy_on_assign a2(log2);

        str_no_alloc_copy_on_assign big_str_1(this->even_bigger_raw_string(), a1);
        str_no_alloc_copy_on_assign big_str_2(this->big_raw_string() ,a2);
        big_str_2 = big_str_1;
        EXPECT_EQ(big_str_2.get_allocator(), a2);
        EXPECT_EQ(big_str_2, big_str_1);
        EXPECT_EQ(log1.allocations_count(), 1);
    }
}

TYPED_TEST(basic_fixture, assign_move_with_allocator)
{
    using char_type = typename TestFixture::char_type;
    using allocator_type = custom_allocator<char_type>;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , allocator_type >;
     {   //from small to small
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->small_raw_string(), a1);
        str_type str_2(a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a1);
        EXPECT_EQ(str_2, this->small_raw_string());
        EXPECT_EQ(log1.allocations_count(), 0);
    }
    {   // from small to big
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->small_raw_string(), a1);
        str_type str_2(this->big_raw_string(), a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a1);
        EXPECT_EQ(str_2, this->small_raw_string());
        EXPECT_EQ(log1.allocations_count(), 0);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
    {   // from big to small
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->big_raw_string(), a1);
        str_type str_2(this->small_raw_string(), a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a1);
        EXPECT_EQ(str_2, this->big_raw_string());
        EXPECT_EQ(log1.allocations_count(), 1);
        EXPECT_EQ(log2.allocations_count(), 0);
    }
    {   // from big to bigger
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->big_raw_string(), a1);
        str_type str_2(this->even_bigger_raw_string() ,a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a1);
        EXPECT_EQ(str_2, this->big_raw_string());
        EXPECT_EQ(log1.allocations_count(), 1);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
    {   // from big to less big
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->even_bigger_raw_string(), a1);
        str_type str_2(this->big_raw_string() ,a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a1);
        EXPECT_EQ(str_2, this->even_bigger_raw_string());
        EXPECT_EQ(log1.allocations_count(), 1);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
}

TYPED_TEST(basic_fixture, assign_move_without_allocator)
{
    using char_type = typename TestFixture::char_type;
    using allocator_type = custom_allocator< char_type
                                           , std::true_type
                                           , std::false_type >;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , allocator_type >;

     {   //from small to small
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->small_raw_string(), a1);
        str_type str_2(a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a2);
        EXPECT_EQ(str_2, this->small_raw_string());
    }
    {   // from small to big
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->small_raw_string(), a1);
        str_type str_2(this->big_raw_string(), a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a2);
        EXPECT_EQ(str_2, this->small_raw_string());
        EXPECT_EQ(log1.allocations_count(), 0);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
    {   // from big to small
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->big_raw_string(), a1);
        str_type str_2(this->small_raw_string(), a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a2);
        EXPECT_EQ(str_2, this->big_raw_string());
        EXPECT_EQ(log1.allocations_count(), 1);
        EXPECT_EQ(log2.allocations_count(), 0);
    }
    {   // from big to bigger
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->big_raw_string(), a1);
        str_type str_2(this->even_bigger_raw_string(), a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a2);
        EXPECT_EQ(str_2, this->big_raw_string());
        EXPECT_EQ(log1.allocations_count(), 1);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
    {   // from big to less big
        allocator_log log1;
        allocator_log log2;
        allocator_type a1(log1);
        allocator_type a2(log2);

        str_type str_1(this->even_bigger_raw_string(), a1);
        str_type str_2(this->big_raw_string(), a2);
        str_2 = std::move(str_1);
        EXPECT_EQ(str_2.get_allocator(), a2);
        EXPECT_EQ(str_2, this->even_bigger_raw_string());
        EXPECT_EQ(log1.allocations_count(), 1);
        EXPECT_EQ(log2.allocations_count(), 1);
    }
}


TYPED_TEST(basic_fixture, move_to_api_string)
{
    // operator basic_api_string () &&

    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    using api_str_type = speudo_std::basic_api_string<char_type>;

    {
        allocator_log log;
        custom_allocator<char_type> a(log);

        str_type str(this->big_raw_string(), a);
        api_str_type api_str(std::move(str));
        test_empty(str);
        EXPECT_EQ(api_str, this->big_raw_string());
        EXPECT_EQ(log.allocations_count(), 1);
        EXPECT_EQ(log.deallocations_count(), 0);

        api_str.clear();
        EXPECT_EQ(log.deallocations_count(), 1);
    }
    {
        allocator_log log;
        custom_allocator<char_type> a(log);

        str_type str(this->small_raw_string(), a);
        api_str_type api_str(std::move(str));
        EXPECT_EQ(api_str, this->small_raw_string());
    }
    {
        allocator_log log;
        custom_allocator<char_type> a(log);
        constexpr std::size_t len = std::min(str_type::sso_capacity, api_str_type::sso_capacity);
        char_type raw_string[len + 1];
        this->fill(raw_string, len);

        str_type str(raw_string, a);
        api_str_type api_str(std::move(str));
        EXPECT_EQ(api_str, raw_string);
        EXPECT_EQ(log.allocations_count(), 0);
    }
}

TYPED_TEST(basic_fixture, convert_to_api_string)
{
    // operator basic_api_string () const &

    using char_type = typename TestFixture::char_type;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;
    using api_str_type = speudo_std::basic_api_string<char_type>;

    {
        allocator_log log;
        custom_allocator<char_type> a(log);

        str_type str(this->big_raw_string(), a);
        api_str_type api_str(str);
        EXPECT_EQ(api_str, str.c_str());
        EXPECT_EQ(api_str, this->big_raw_string());
        EXPECT_EQ(log.allocations_count(), 2);
    }
    {
        allocator_log log;
        custom_allocator<char_type> a(log);

        str_type str(this->small_raw_string(), a);
        api_str_type api_str(str);
        EXPECT_EQ(api_str, str.c_str());
        EXPECT_EQ(api_str, this->small_raw_string());
    }
    {
        allocator_log log;
        custom_allocator<char_type> a(log);
        constexpr std::size_t len = std::min(str_type::sso_capacity, api_str_type::sso_capacity);
        char_type raw_string[len + 1];
        this->fill(raw_string, len);

        str_type str(raw_string, a);
        api_str_type api_str(str);
        EXPECT_EQ(api_str, str.c_str());
        EXPECT_EQ(api_str, raw_string);
        EXPECT_EQ(log.allocations_count(), 0);
    }
}


TYPED_TEST(basic_fixture, construct_from_moveable_api_string)
{
    // basic_string
    //     ( basic_api_string<CharT>&& other
    //     , const Allocator& alloc = Allocator{} );

    using char_type = typename TestFixture::char_type;
    using api_string_type = speudo_std::basic_api_string<char_type>;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;

    {
        allocator_log log;
        custom_allocator<char_type> a(log);
        api_string_type astr = this->big_raw_string();
        str_type str(std::move(astr), a);
        EXPECT_EQ(log.allocations_count(), 0);
    }
    {
        allocator_log log;
        custom_allocator<char_type> a(log);
        api_string_type astr_1 = this->big_raw_string();
        api_string_type astr_2 = astr_1;
        str_type str(std::move(astr_2), a);
        EXPECT_EQ(log.allocations_count(), 1);
    }
    {
        allocator_log log;
        custom_allocator<char_type> a(log);
        char_type small_raw_string[api_string_type::sso_capacity + 1];
        this->fill(small_raw_string, api_string_type::sso_capacity);
        api_string_type astr_1 = small_raw_string;
        api_string_type astr_2 = astr_1;
        str_type str(std::move(astr_2), a);
        EXPECT_EQ(log.allocations_count(), 0);
    }
    {
        allocator_log log;
        custom_allocator<char_type> a(log);

        api_string_type astr = speudo_std::api_string_ref(this->big_raw_string());
        str_type str(std::move(astr), a);
        EXPECT_EQ(log.allocations_count(), 1);
    }
}


TYPED_TEST(basic_fixture, construct_from_const_api_string)
{
    // basic_string
    //     ( const basic_api_string<CharT>& other
    //     , const Allocator& alloc = Allocator{} )

    using char_type = typename TestFixture::char_type;
    using api_string_type = speudo_std::basic_api_string<char_type>;
    using str_type = speudo_std::basic_string< char_type
                                             , std::char_traits<char_type>
                                             , custom_allocator<char_type> >;

    {
        allocator_log log;
        custom_allocator<char_type> a(log);
        api_string_type astr = this->big_raw_string();
        str_type str(astr, a);
        EXPECT_EQ(log.allocations_count(), 1);
    }
    {
        allocator_log log;
        custom_allocator<char_type> a(log);
        char_type small_raw_string[api_string_type::sso_capacity + 1];
        this->fill(small_raw_string, api_string_type::sso_capacity);
        api_string_type astr_1 = small_raw_string;
        str_type str(astr_1, a);
        EXPECT_EQ(log.allocations_count(), 0);
    }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
