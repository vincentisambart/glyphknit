cmake_minimum_required(VERSION 3.0)

project(glyphknit)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++1y")

set(gtest-directory "${PROJECT_SOURCE_DIR}/../../src/googletest")

set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
add_definitions(-DU_USING_ICU_NAMESPACE=0 -DU_DISABLE_RENAMING=1)

set(warning-flags -Weverything -Wno-c++98-compat -Wno-c99-extensions -Wno-documentation -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic -Wno-padded -Wno-deprecated -Wno-weak-vtables -Wno-missing-prototypes -Wno-sign-conversion -Wno-old-style-cast)

include_directories("${PROJECT_SOURCE_DIR}/vendor/icu-53.1")
include_directories("${PROJECT_SOURCE_DIR}/include")

add_subdirectory(vendor)

add_library(glyphknit STATIC
  src/text_block.cc
  src/typesetter.cc
  src/mini_coretext_typesetter.cc
  src/script_iterator.cc
  src/split_runs.cc
  src/language.cc
  src/font.cc
)
find_library(APPLICATION_SERVICES_FRAMEWORK ApplicationServices)
target_link_libraries(glyphknit ${APPLICATION_SERVICES_FRAMEWORK} icucore harfbuzz freetype)
target_compile_options(glyphknit PRIVATE ${warning-flags})

add_library(gtest STATIC ${gtest-directory}/src/gtest-all.cc)
target_include_directories(gtest PRIVATE "${gtest-directory}")
target_include_directories(gtest PUBLIC "${gtest-directory}/include")

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
