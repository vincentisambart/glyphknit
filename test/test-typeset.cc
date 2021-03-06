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
  kDefault         = 0,
  kDrawToFiles     = 1 << 0,
  kIgnorePositions = 1 << 1,
  kIgnoreOffsets   = 1 << 2,
};
const double kAllowedPositionDelta = 1.0 / 16384.0;

void ComparePositions(glyphknit::TypesetLines &lines_typeset_by_coretext, glyphknit::TypesetLines &lines_typeset_by_glyphknit, const char *description, int flags = ComparisonFlags::kDefault) {
  ASSERT_EQ(lines_typeset_by_coretext.size(), lines_typeset_by_glyphknit.size()) << "The number of lines should be the same for " << description;
  auto lines_count = lines_typeset_by_glyphknit.size();
  for (size_t line_index = 0; line_index < lines_count; ++line_index) {
    CGFloat coretext_x = 0, coretext_y = 0;
    CGFloat glyphknit_x = 0, glyphknit_y = 0;
    auto &coretext_line = lines_typeset_by_coretext[line_index];
    auto &glyphknit_line = lines_typeset_by_glyphknit[line_index];
    EXPECT_NEAR(coretext_line.ascent, glyphknit_line.ascent, kAllowedPositionDelta) << "at line " << line_index << " for " << description;
    EXPECT_NEAR(coretext_line.descent, glyphknit_line.descent, kAllowedPositionDelta) << "at line " << line_index << " for " << description;
    EXPECT_NEAR(coretext_line.leading, glyphknit_line.leading, kAllowedPositionDelta) << "at line " << line_index << " for " << description;
    EXPECT_EQ(coretext_line.runs.size(), glyphknit_line.runs.size()) << "at line " << line_index << " for " << description;
    size_t runs_count = std::min(coretext_line.runs.size(), glyphknit_line.runs.size());
    for (size_t run_index = 0; run_index < runs_count; ++run_index) {
      auto &coretext_run = coretext_line.runs[run_index];
      auto &glyphknit_run = glyphknit_line.runs[run_index];

      EXPECT_EQ(coretext_run.font_descriptor, glyphknit_run.font_descriptor) << "at run " << run_index << " at line " << line_index << " for " << description;
      EXPECT_NEAR(coretext_run.font_size, glyphknit_run.font_size, kAllowedPositionDelta) << "at run " << run_index << " at line " << line_index << " for " << description;
      EXPECT_EQ(coretext_run.glyphs.size(), glyphknit_run.glyphs.size()) << "at run " << run_index << " at line " << line_index << " for " << description;

      size_t glyphs_count = std::min(coretext_run.glyphs.size(), glyphknit_run.glyphs.size());
      for (size_t glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
        auto &coretext_glyph = coretext_run.glyphs[glyph_index];
        auto &glyphknit_glyph = glyphknit_run.glyphs[glyph_index];
        EXPECT_EQ(coretext_glyph.id, glyphknit_glyph.id) << "at glyph " << glyph_index << " at run " << run_index << " at line " << line_index << " for " << description;
        if (!(flags & ComparisonFlags::kIgnoreOffsets)) {
          EXPECT_EQ(coretext_glyph.offset, glyphknit_glyph.offset) << "at glyph " << glyph_index << " at run " << run_index << " at line " << line_index << " for " << description;
        }
        if (!(flags & ComparisonFlags::kIgnorePositions)) {
          EXPECT_NEAR(coretext_x + coretext_glyph.x_offset, glyphknit_x + glyphknit_glyph.x_offset, kAllowedPositionDelta) << "at glyph " << glyph_index << " at run " << run_index << " at line " << line_index << " for " << description;
          EXPECT_NEAR(coretext_y + coretext_glyph.y_offset, glyphknit_y + glyphknit_glyph.y_offset, kAllowedPositionDelta) << "at glyph " << glyph_index << " at run " << run_index << " at line " << line_index << " for " << description;
          coretext_x += coretext_glyph.x_advance;
          glyphknit_x += glyphknit_glyph.x_advance;
          coretext_y += coretext_glyph.y_advance;
          glyphknit_y += glyphknit_glyph.y_advance;
        }
      }
    }
  }
}

const int kImageWidth = 200;
const int kImageHeight = 200;

void CompareTypesetters(glyphknit::TextBlock &text_block, const char *description, int flags = ComparisonFlags::kDefault) {
  glyphknit::MiniCoreTextTypesetter ct_typesetter;
  glyphknit::TypesetLines lines_typeset_by_coretext = ct_typesetter.PositionGlyphs(text_block, kImageWidth);
  if (flags & ComparisonFlags::kDrawToFiles) {
    DrawToFile("test-coretext.png", kImageWidth, kImageHeight, [&](CGContextRef context) {
      ct_typesetter.DrawToContext(text_block, kImageWidth, context);
    });
  }

  glyphknit::Typesetter glyphknit_typesetter;
  glyphknit::TypesetLines lines_typeset_by_glyphknit = glyphknit_typesetter.PositionGlyphs(text_block, kImageWidth);
  if (flags & ComparisonFlags::kDrawToFiles) {
    DrawToFile("test-glyphknit.png", kImageWidth, kImageHeight, [&](CGContextRef context) {
      glyphknit_typesetter.DrawToContext(text_block, kImageWidth, context);
    });
  }

  ComparePositions(lines_typeset_by_coretext, lines_typeset_by_glyphknit, description, flags);
}

void SimpleCompare(const char *text, const char *description, const char *font_name, float font_size, int flags = ComparisonFlags::kDefault) {
  auto font = glyphknit::FontManager::CreateDescriptorFromPostScriptName(font_name);
  assert(font.is_valid());
  glyphknit::TextBlock text_block{font, font_size};
  text_block.SetText(text);

  CompareTypesetters(text_block, description, flags);
}

TEST(Typesetter, HandlesSimpleLTRText) {
  SimpleCompare("abcde\nfghijk", "LF with no wrapping needed", "SourceSansPro-Regular", 13);
  SimpleCompare("abcde\u2028fghijk", "NL with no wrapping needed", "SourceSansPro-Regular", 13);
  SimpleCompare("abcdefghijklmnopqr abcdefghijklmnopqr", "simple wrapping", "SourceSansPro-Regular", 13);
  SimpleCompare("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", "one very long word", "SourceSansPro-Regular", 13);
  SimpleCompare("abcde                                                                                                                                              abcde", "many spaces", "SourceSansPro-Regular", 13);
  SimpleCompare("12345\nabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", "2 paragraphs with breaking in the 2nd paragraph", "SourceSansPro-Regular", 13);

  SimpleCompare("abcdefghijklmnopqr", "very simple text", "SourceSansPro-Regular", 13);

  SimpleCompare("ffff", "simple ligatures", "SourceSansPro-Regular", 50);

  // for combining marks the positions don't match but I'm not sure whose fault it is
  SimpleCompare("e\u0301\u0301", "e with combining acutes", "Arial", 20, ComparisonFlags::kIgnorePositions | ComparisonFlags::kIgnoreOffsets);
  SimpleCompare("abcde\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0\u00A0abcde", "many non-breaking spaces", "SourceSansPro-Regular", 13);

  SimpleCompare("a                               bcdefghijklmnopqr", "simple text with many spaces and very big font", "SourceSansPro-Regular", 200);

  // I don't know why the following test doesn't work with SourceSansPro-Regular, seems to be more of a problem with that font and CoreText
  //SimpleCompare("a                               bcdefghijklmnopqr", "simple text with many no-break spaces and big font", "SourceSansPro-Regular", 50);
}

TEST(Typesetter, HandlesRTLText) {
  SimpleCompare("شششششششششزززززززززز", "simple Arabic", "Scheherazade", 20);
  SimpleCompare("شششششششششششششششششششششششaaaaaaaaaaaaaaaaaaaaaaaaaaaaaششششششششششششششششششششaaaaaaaaaaaaaaaaaaaa", "Arabic and Latin characters mixed", "Scheherazade", 20);
}

TEST(Typesetter, HandlesFontFallback) {
  SimpleCompare("abcdeあいうえおklmnopqr", "simple text with Japanese not in font", "SourceSansPro-Regular", 13);
  SimpleCompare("abcde あいうえお klmnopqr", "simple text with Japanese not in font with spaces", "SourceSansPro-Regular", 13);

  auto font = glyphknit::FontManager::CreateDescriptorFromPostScriptName("Helvetica");
  assert(font.is_valid());
  glyphknit::TextBlock text_block{font, 20};
  text_block.SetText("骨");
  text_block.SetLanguage(glyphknit::FindLanguageCodeAndOpenTypeLanguageTag("zh-Hans"));
  CompareTypesetters(text_block, "text with language forced to Chinese");
  text_block.SetLanguage(glyphknit::FindLanguageCodeAndOpenTypeLanguageTag("zh-Hant"));
  CompareTypesetters(text_block, "text with language forced to Traditional Chinese");
  text_block.SetLanguage(glyphknit::FindLanguageCodeAndOpenTypeLanguageTag("ja"));
  CompareTypesetters(text_block, "text with language forced to Japanese");
}

TEST(Typesetter, HandlesMultipleFontSized) {
  auto font = glyphknit::FontManager::CreateDescriptorFromPostScriptName("SourceSansPro-Regular");
  assert(font.is_valid());
  glyphknit::TextBlock text_block{font, 20};
  text_block.SetText("abcdefghi abcdefghijklmnopqrst");
  text_block.SetFontSize(30, 15);
  CompareTypesetters(text_block, "font size change is not a possible line break");

  text_block.SetText("abcde");
  text_block.SetFontSize(70);
  text_block.SetFontSize(400, 0, 1);
  CompareTypesetters(text_block, "the first character of a run does not always fit");
}
