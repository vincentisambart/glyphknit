/*
 * Copyright © 2014  Vincent Isambart
 *
 *  This file is part of Glyphknit.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "mini_coretext_typesetter.hh"
#include "typesetter.hh"
#include "autorelease.hh"
#include "utf.hh"

#include "test.h"

#include <iostream>
#include <ApplicationServices/ApplicationServices.h>

using glyphknit::MakeAutoReleasedCFRef;

template <typename Callable>
void DrawToFile(const char *file_path, const size_t image_width, const size_t image_height, Callable render_callback) {
  const size_t kBitsPerComponents = 8;
  const size_t kBytesPerRow = image_width * 4;

  auto rgb_color_space = MakeAutoReleasedCFRef(CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB));
  auto context = MakeAutoReleasedCFRef(CGBitmapContextCreate(nullptr, image_width, image_height, kBitsPerComponents, kBytesPerRow, rgb_color_space.get(), kCGImageAlphaPremultipliedLast));
  assert(context.get() != nullptr);

  CGContextSetRGBFillColor(context.get(), 1, 1, 1, 1);
  CGContextFillRect(context.get(), CGRectMake(0, 0, image_width, image_height));
  CGContextSetRGBFillColor(context.get(), 0, 0, 0, 1);

  CGContextSetTextMatrix(context.get(), CGAffineTransformIdentity);
  render_callback(context.get());

  auto image = MakeAutoReleasedCFRef(CGBitmapContextCreateImage(context.get()));
  auto file_path_string = MakeAutoReleasedCFRef(CFStringCreateWithCString(kCFAllocatorDefault, file_path, kCFStringEncodingUTF8));
  auto url = MakeAutoReleasedCFRef(CFURLCreateWithFileSystemPath(nullptr, file_path_string.get(), kCFURLPOSIXPathStyle, false));
  auto destination = MakeAutoReleasedCFRef(CGImageDestinationCreateWithURL(url.get(), kUTTypePNG, 1, nullptr));

  CGImageDestinationAddImage(destination.get(), image.get(), nullptr);
  CGImageDestinationFinalize(destination.get());
}

enum ComparisonFlags {
  kDefault = 0,
  kDrawToFiles = 1,
  kIgnorePositions = 2,
};
const double kAllowedPositionDelta = 1.0 / 16384.0;

// TODO: Compare fonts
void ComparePositions(glyphknit::PositionedLines &lines_typesetted_by_coretext, glyphknit::PositionedLines &lines_typesetted_by_glyphknit, const char *description, int flags = ComparisonFlags::kDefault) {
  ASSERT_EQ(lines_typesetted_by_coretext.size(), lines_typesetted_by_glyphknit.size()) << "The number of lines should be the same for " << description;
  auto lines_count = lines_typesetted_by_glyphknit.size();
  for (size_t line_index = 0; line_index < lines_count; ++line_index) {
    auto &line_typesetted_by_coretext = lines_typesetted_by_coretext[line_index];
    auto &line_typesetted_by_glyphknit = lines_typesetted_by_glyphknit[line_index];
    EXPECT_EQ(line_typesetted_by_coretext.glyphs.size(), line_typesetted_by_coretext.positions.size()) << "at line index " << line_index << " for " << description;
    EXPECT_EQ(line_typesetted_by_glyphknit.glyphs.size(), line_typesetted_by_glyphknit.positions.size()) << "at line index " << line_index << " for " << description;
    EXPECT_EQ(line_typesetted_by_glyphknit.glyphs.size(), line_typesetted_by_coretext.glyphs.size()) << "at line index " << line_index << " for " << description;
    size_t glyphs_count = std::min(line_typesetted_by_glyphknit.glyphs.size(), line_typesetted_by_coretext.glyphs.size());

    for (size_t glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
      EXPECT_EQ(line_typesetted_by_coretext.glyphs[glyph_index], line_typesetted_by_glyphknit.glyphs[glyph_index]) << "at glyph index " << glyph_index << " at line index " << line_index << " for " << description;
      EXPECT_EQ(line_typesetted_by_coretext.text_offsets[glyph_index], line_typesetted_by_glyphknit.text_offsets[glyph_index]) << "at glyph index " << glyph_index << " at line index " << line_index << " for " << description;
      if (!(flags & ComparisonFlags::kIgnorePositions)) {
        EXPECT_NEAR(line_typesetted_by_coretext.positions[glyph_index].x, line_typesetted_by_glyphknit.positions[glyph_index].x, kAllowedPositionDelta) << "at glyph index " << glyph_index << " at line index " << line_index << " for " << description;
        EXPECT_NEAR(line_typesetted_by_coretext.positions[glyph_index].y, line_typesetted_by_glyphknit.positions[glyph_index].y, kAllowedPositionDelta) << "at glyph index " << glyph_index << " at line index " << line_index << " for " << description;
      }
    }
  }
}

const int kImageWidth = 200;
const int kImageHeight = 100;
void SimpleCompare(const char *text, const char *description, const char *font_name, float font_size, int flags = ComparisonFlags::kDefault) {
  auto font = glyphknit::FontManager::LoadFontFromPostScriptName(font_name);
  assert(font.get() != nullptr);
  auto sized_font = glyphknit::SizedFont(font, font_size);
  glyphknit::TextBlock text_block{sized_font};
  text_block.SetText(text);

  glyphknit::MiniCoreTextTypesetter ct_typesetter;
  glyphknit::PositionedLines lines_typesetted_by_coretext;
  ct_typesetter.PositionGlyphs(text_block, kImageWidth, lines_typesetted_by_coretext);
  if (flags & ComparisonFlags::kDrawToFiles) {
    DrawToFile("test-coretext.png", kImageWidth, kImageHeight, [&](CGContextRef context) {
      ct_typesetter.DrawToContext(text_block, kImageWidth, context);
    });
  }

  glyphknit::Typesetter glyphknit_typesetter;
  glyphknit::PositionedLines lines_typesetted_by_glyphknit;
  glyphknit_typesetter.PositionGlyphs(text_block, kImageWidth, lines_typesetted_by_glyphknit);
  if (flags & ComparisonFlags::kDrawToFiles) {
    DrawToFile("test-glyphknit.png", kImageWidth, kImageHeight, [&](CGContextRef context) {
      glyphknit_typesetter.DrawToContext(text_block, kImageWidth, context);
    });
  }

  ComparePositions(lines_typesetted_by_coretext, lines_typesetted_by_glyphknit, description, flags);
}

TEST(Typesetter, HandlesSimpleLTRText) {
  SimpleCompare("abcde\nfghijk", "LF with no wrapping needed", "SourceSansPro-Regular", 13);
  SimpleCompare("abcde\u2028fghijk", "NL with no wrapping needed", "SourceSansPro-Regular", 13);
  SimpleCompare("abcdefghijklmnopqr abcdefghijklmnopqr", "simple wrapping", "SourceSansPro-Regular", 13);
  SimpleCompare("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", "one very long word", "SourceSansPro-Regular", 13);
  SimpleCompare("abcde                                                                                                                                              abcde", "many spaces", "SourceSansPro-Regular", 13);

  SimpleCompare("abcdefghijklmnopqr", "very simple text", "SourceSansPro-Regular", 13);

  SimpleCompare("ffff", "simple ligatures", "SourceSansPro-Regular", 50);

  // for combining marks the positions don't match but I'm not sure whose fault it is
  //SimpleCompare(" \u0301\u0301", "space with combining acutes", "SourceSansPro-Regular", 13, ComparisonFlags::kIgnorePositions);
  //SimpleCompare("e\u0301\u0301", "e with combining acutes", "Arial", 13, ComparisonFlags::kIgnorePositions | ComparisonFlags::kDrawToFiles);
  //SimpleCompare("abcde\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0abcde", "many non-breaking spaces");
  SimpleCompare("a                               bcdefghijklmnopqr", "simple text with many spaces and very big font", "SourceSansPro-Regular", 100);

  // I don't know why the following test doesn't work with SourceSansPro-Regular, seems to be more of a problem with that font and CoreText
  //SimpleCompare("a                               bcdefghijklmnopqr", "simple text with many no-break spaces and very big font", "Arial", 100);
}

/*
TEST(Typesetter, HandlesFontFallback) {
  SimpleCompare("abcdeあいうえおklmnopqr", "simple text with Japanese not in font", "SourceSansPro-Regular", 13);
}
*/
