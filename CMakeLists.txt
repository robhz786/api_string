cmake_minimum_required (VERSION 3.0)
enable_testing()

project(api_string LANGUAGES CXX VERSION 0.1)

add_library(api_string STATIC source/api_string.cpp)
target_include_directories(api_string PUBLIC include)

option(API_STRING_TEST "Generate tests" ON)

if (API_STRING_TEST)

  add_subdirectory(googletest)

  add_library(api_string_test_mode STATIC source/api_string.cpp)
  target_include_directories(api_string_test_mode PUBLIC include)
  target_compile_definitions(api_string_test_mode PUBLIC API_STRING_TEST_MODE)

  add_executable(test_basic_api_string test/basic_api_string.cpp)
  add_executable(test_basic_string     test/basic_string.cpp)
  target_link_libraries(test_basic_api_string gtest api_string_test_mode)
  target_link_libraries(test_basic_string     gtest api_string_test_mode)
  
  add_test(test_basic_api_string test_basic_api_string)
  add_test(test_basic_string     test_basic_string)
  
endif (API_STRING_TEST)