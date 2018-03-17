cmake_minimum_required (VERSION 3.0)

enable_testing()

set(CMAKE_CXX_STANDARD 14)

project(api_string LANGUAGES CXX VERSION 0.1)

add_library(api_string STATIC source/api_string.cpp)
target_include_directories(api_string PUBLIC include)

option(API_STRING_TEST "Generate tests" OFF)

if (API_STRING_TEST)

  add_library(api_string_test_mode STATIC source/api_string.cpp)
  target_include_directories(api_string_test_mode PUBLIC include)
  target_compile_definitions(api_string_test_mode PUBLIC API_STRING_TEST_MODE)
  
  add_subdirectory(test)
  
endif (API_STRING_TEST)