set(freetype-version 2.5.3)
set(freetype-source-directory "freetype-${freetype-version}")
add_subdirectory(${freetype-source-directory})
target_include_directories(freetype INTERFACE ${freetype-source-directory}/include)

set(harfbuzz-version 0.9.35)
set(harfbuzz-source-directory "harfbuzz-${harfbuzz-version}/src")
add_library(harfbuzz STATIC
  ${harfbuzz-source-directory}/hb-blob.cc
  ${harfbuzz-source-directory}/hb-buffer-serialize.cc
  ${harfbuzz-source-directory}/hb-buffer.cc
  ${harfbuzz-source-directory}/hb-common.cc
  ${harfbuzz-source-directory}/hb-fallback-shape.cc
  ${harfbuzz-source-directory}/hb-face.cc
  ${harfbuzz-source-directory}/hb-font.cc
  ${harfbuzz-source-directory}/hb-ot-tag.cc
  ${harfbuzz-source-directory}/hb-set.cc
  ${harfbuzz-source-directory}/hb-shape.cc
  ${harfbuzz-source-directory}/hb-shape-plan.cc
  ${harfbuzz-source-directory}/hb-shaper.cc
  ${harfbuzz-source-directory}/hb-ft.cc
  ${harfbuzz-source-directory}/hb-unicode.cc
  ${harfbuzz-source-directory}/hb-warning.cc
  ${harfbuzz-source-directory}/hb-ot-layout.cc
  ${harfbuzz-source-directory}/hb-ot-map.cc
  ${harfbuzz-source-directory}/hb-ot-shape.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-arabic.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-default.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-hangul.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-hebrew.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-indic.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-indic-table.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-myanmar.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-sea.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-thai.cc
  ${harfbuzz-source-directory}/hb-ot-shape-complex-tibetan.cc
  ${harfbuzz-source-directory}/hb-ot-shape-normalize.cc
  ${harfbuzz-source-directory}/hb-ot-shape-fallback.cc
  ${harfbuzz-source-directory}/hb-icu.cc
)
target_compile_definitions(harfbuzz PRIVATE -DHAVE_OT -DHAVE_FREETYPE -DHAVE_ICU -DHAVE_ICU_BUILTIN)
target_include_directories(harfbuzz INTERFACE ${harfbuzz-source-directory})
target_link_libraries(harfbuzz icucore freetype)
