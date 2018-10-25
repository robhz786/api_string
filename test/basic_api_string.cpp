#include <gtest/gtest.h>
#include <api_string.hpp>


template <typename CharT>
void test_empty(const speudo_std::basic_api_string<CharT>& api_str)
{
    EXPECT_TRUE(api_str.empty());
    EXPECT_EQ(api_str.size(), 0);
    EXPECT_EQ(api_str.length(), 0);
    EXPECT_EQ(api_str.c_str()[0], CharT{});
    EXPECT_EQ(api_str.begin(), api_str.end());
}

template <typename CharT>
void test_equal
    ( const speudo_std::basic_api_string<CharT>& api_str
    , const CharT* cstr
    , std::size_t len
    )
{
    EXPECT_EQ(api_str.size(), len);
    EXPECT_EQ(api_str.length(), len);
    EXPECT_EQ(api_str.c_str()[len], CharT{});
    EXPECT_EQ(api_str.begin() + len, api_str.end());
}

template <typename CharT>
class basic_fixture: public ::testing::Test
{
public:

    basic_fixture()
    {
        speudo_std::api_string_test::reset();
        fill(m_small_buff, small_string_len());
        fill(m_big_buff, big_string_len());
    }

    using char_type = CharT;
    using data_type = speudo_std::abi::api_string_data<CharT>;

    constexpr static std::size_t small_string_len()
    {
        return data_type::small_capacity();
    }
    constexpr static std::size_t big_string_len()
    {
        return data_type::small_capacity() + 1;
    }

    const CharT* small_string() const
    {
        return m_small_buff;
    }

    const CharT* big_string() const
    {
        return m_big_buff;
    }


private:
    CharT m_small_buff[small_string_len() + 1];
    CharT m_big_buff[big_string_len() + 1];

    static void fill(CharT* str, std::size_t len)
    {
        CharT c = ' ';
        for (int i = 0; i < len; ++i)
        {
            str[i] = c;
            ++c;
        }
        str[len] = 0;
    }


};

using all_char_types = ::testing::Types<char, wchar_t, char16_t, char32_t>;

TYPED_TEST_CASE(basic_fixture, all_char_types);

TYPED_TEST(basic_fixture,  default_constructor)
{
    using char_type = typename TestFixture::char_type;
    speudo_std::basic_api_string<char_type> s;

    EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 0);
    EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 0);
    test_empty(s);
}

TYPED_TEST(basic_fixture, construct_from_empty_string)
{
    using char_type = typename TestFixture::char_type;
    char_type c = 0;
    speudo_std::basic_api_string<char_type> s{&c};

    EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 0);
    EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 0);

    test_empty(s);
    test_equal(s, &c, 0);

    using data_type = typename TestFixture::data_type;
    static_assert(sizeof(s) == sizeof(data_type));
    EXPECT_EQ(reinterpret_cast<data_type&>(s).big.len, 0);
}

TYPED_TEST(basic_fixture,  small_string)
{
    using char_type = typename TestFixture::char_type;

    // constructor
    speudo_std::basic_api_string<char_type> s{this->small_string()};
    test_equal(s, this->small_string(), this->small_string_len());

    // copy constructor
    auto s2 = s;
    test_equal(s2, this->small_string(), this->small_string_len());

    // clear
    s.clear();
    test_empty(s);

    // assignment
    s = this->small_string();
    test_equal(s, this->small_string(), this->small_string_len());

    s = s2;
    test_equal(s, this->small_string(), this->small_string_len());

    // no allocation nor deallocation
    EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 0);
    EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 0);

}

TYPED_TEST(basic_fixture,  big_string)
{
    using char_type = typename TestFixture::char_type;

    // constructor
    speudo_std::basic_api_string<char_type> s{this->big_string()};
    EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 1);
    EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 0);
    test_equal(s, this->big_string(), this->big_string_len());

    // move constructor
    auto s2(std::move(s));
    test_empty(s);
    test_equal(s2, this->big_string(), this->big_string_len());

    // move assignment
    s = std::move(s2);
    test_empty(s2);
    test_equal(s, this->big_string(), this->big_string_len());

    // copy assignment
    s2 = s;
    EXPECT_EQ(s, s2);
    test_equal(s, this->big_string(), this->big_string_len());
    test_equal(s2, this->big_string(), this->big_string_len());

    // copy constructor
    auto s3(s2);
    EXPECT_EQ(s2, s3);
    test_equal(s2, this->big_string(), this->big_string_len());
    test_equal(s3, this->big_string(), this->big_string_len());

    // clear()
    s.clear();
    s3.clear();
    EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 0);
    s2.clear();
    EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 1);
    EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 1);
}


TYPED_TEST(basic_fixture,  string_reference)
{
    // constructor
    auto s = speudo_std::api_string_ref(this->big_string());
    test_equal(s, this->big_string(), this->big_string_len());

    // move constructor
    auto s2(std::move(s));
    test_equal(s2, this->big_string(), this->big_string_len());

    // move assignment
    s = std::move(s2);
    test_equal(s, this->big_string(), this->big_string_len());

    // copy assignment
    s2 = s;
    EXPECT_EQ(s, s2);
    test_equal(s, this->big_string(), this->big_string_len());
    test_equal(s2, this->big_string(), this->big_string_len());

    // copy constructor
    auto s3(s2);
    EXPECT_EQ(s2, s3);
    test_equal(s2, this->big_string(), this->big_string_len());
    test_equal(s3, this->big_string(), this->big_string_len());

    // clear()
    s.clear();
    s3.clear();
    s2.clear();

    EXPECT_EQ(speudo_std::api_string_test::allocations_count(), 0);
    EXPECT_EQ(speudo_std::api_string_test::deallocations_count(), 0);

}

// TYPED_TEST(basic_fixture,  remove_prefix)
// {
//      using char_type = typename TestFixture::char_type;
//      using api_str_type = speudo_std::basic_api_string<char_type>;

//      {
//          api_str_type s{this->small_string()};
//          s.remove_prefix(0);
//          test_equal(s, this->small_string(), this->small_string_len());
//      }
//      {
//          api_str_type s{this->big_string()};
//          s.remove_prefix(0);
//          test_equal(s, this->big_string(), this->big_string_len());
//      }
//      {
//          auto s= speudo_std::api_string_ref(this->big_string());
//          s.remove_prefix(0);
//          test_equal(s, this->big_string(), this->big_string_len());
//      }

//      {
//          api_str_type s{this->small_string()};
//          s.remove_prefix(this->small_string_len());
//          test_empty(s);
//          s.remove_prefix(1);
//          test_empty(s);
//      }
//      {
//          api_str_type s{this->big_string()};
//          s.remove_prefix(this->big_string_len());
//          test_empty(s);
//          s.remove_prefix(1);
//          test_empty(s);
//      }
//      {
//          auto s = speudo_std::api_string_ref(this->big_string());
//          s.remove_prefix(this->big_string_len() + 1);
//          test_empty(s);
//          s.remove_prefix(1);
//          test_empty(s);
//      }
     
//      {
//          api_str_type s{this->small_string()};
//          s.remove_prefix(this->small_string_len() + 1);
//          test_empty(s);
//      }
//      {
//          api_str_type s{this->big_string()};
//          s.remove_prefix(this->big_string_len() + 1);
//          test_empty(s);
//      }
//      {
//          auto s = speudo_std::api_string_ref(this->big_string());
//          s.remove_prefix(this->big_string_len() + 1);
//          test_empty(s);
//      }
     
//      for(auto n = 0; n <= this->small_string_len(); ++n)
//      {
//          api_str_type s{this->small_string()};
//          s.remove_prefix(n);
//          EXPECT_EQ(s.length(), this->small_string_len() - n);
//          EXPECT_EQ(s, this->small_string() + n);
//      }
//      {
//          api_str_type s{this->small_string()};
//          for(auto n = 1; n <= this->small_string_len(); ++n)
//          {
//              s.remove_prefix(1);
//              EXPECT_EQ(s.length(), this->small_string_len() - n);
//              EXPECT_EQ(s, this->small_string() + n);
//          }
//      }
//      {
//          api_str_type as1{this->big_string()};
//          speudo_std::basic_string<char_type> s1(std::move(as1));
         
//          api_str_type as2{this->big_string()};
//          as2.remove_prefix(1)
//          speudo_std::basic_string<char_type> s2(std::move(as1));

//          EXPECT_NE(s1.capacity(), s2.capacity());
//      }     
// }

// remove_prefix
// destructor
// swap
// at, front, back
// compare
// starts_with



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
