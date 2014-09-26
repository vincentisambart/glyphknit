cmake_minimum_required(VERSION 3.0)

project(glyphknit)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++1y")

add_subdirectory(vendor)

set(warning-flags -Weverything -Wno-c++98-compat -Wno-c99-extensions -Wno-documentation -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic -Wno-padded -Wno-deprecated -Wno-weak-vtables -Wno-missing-prototypes -Wno-sign-conversion -Wno-old-style-cast)

add_library(glyphknit STATIC
  src/text_block.cc
  src/typesetter.cc
  src/mini_coretext_typesetter.cc
  src/script_iterator.cc
  src/split_runs.cc
  src/language.cc
  src/font.cc
)
target_include_directories(glyphknit PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_options(glyphknit PRIVATE ${warning-flags})
find_library(APPLICATION_SERVICES_FRAMEWORK ApplicationServices)
target_link_libraries(glyphknit ${APPLICATION_SERVICES_FRAMEWORK} icu harfbuzz freetype)

add_executable(glyphknit-test
  test/test-main.cc
  test/test-typeset.cc
  test/test-script_iterator.cc
  test/test-language.cc
  test/test-text_block.cc
  test/test-font.cc
  test/test-split_runs.cc
)
target_compile_options(glyphknit-test PRIVATE ${warning-flags})
target_link_libraries(glyphknit-test glyphknit gtest)
