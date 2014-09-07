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
#include "autorelease.hh"
#include "utf.hh"

#include <ApplicationServices/ApplicationServices.h>

namespace glyphknit {

static
bool IsCodepointToIgnoreForComparison(UChar32 c) {
  switch (c) { // line/paragraph separators should be ignored because Glyphknit does not output glyphs for those
    case 0x000B: // LINE/VERTICAL TABULATION (VT) \v
    case 0x000C: // FORM FEED (FF) \f
    case 0x2028: // LINE SEPARATOR (LS)
    case 0x000A: // LINE FEED (LF) \n
    case 0x000D: // CARRIAGE RETURN (CR) \r
    case 0x0085: // NEXT LINE (NEL)
    case 0x2029: // PARAGRAPH SEPARATOR (PS)
      return true;
    default:
      return false;
  }
}


template <typename Callable>
void MiniCoreTextTypesetter::Typeset(TextBlock &text_block, const size_t width, Callable process_frame) {
  auto string = MakeAutoReleasedCFRef(CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, text_block.text_content(), text_block.text_length(), kCFAllocatorNull));
  auto attributed_string = MakeAutoReleasedCFRef(CFAttributedStringCreateMutable(kCFAllocatorDefault, 0));
  CFAttributedStringReplaceString(attributed_string.get(), CFRangeMake(0, 0), string.get());
  CFAttributedStringBeginEditing(attributed_string.get());
  for (auto &run : text_block.attributes_runs()) {
    auto ct_font = run.attributes.font_descriptor.CreateNativeFont(run.attributes.font_size);
    CFAttributedStringSetAttribute(attributed_string.get(), CFRangeMake(run.start, run.end-run.start), kCTFontAttributeName, ct_font.get());
    const auto &language = run.attributes.language;
    if (!language.is_undefined()) {
      char language_name[8];
      if (language.language_code == MakeTag('z','h')) {
        if (language.opentype_tag == MakeTag('Z','H','H') || language.opentype_tag == MakeTag('Z','H','T')) {
          std::strcpy(language_name, "zh-Hant");
        }
        else {
          std::strcpy(language_name, "zh-Hans");
        }
      }
      else if (((language.language_code >> 8) & 0xFF) == kEmptyTagCharacter) {  // 2 characters language code
        language_name[0] = (language.language_code >> 24) & 0xFF;
        language_name[1] = (language.language_code >> 16) & 0xFF;
        language_name[2] = 0;
      }
      else {  // 3 characters language code
        language_name[0] = (language.language_code >> 24) & 0xFF;
        language_name[1] = (language.language_code >> 16) & 0xFF;
        language_name[2] = (language.language_code >>  8) & 0xFF;
        language_name[3] = 0;
      }
      auto native_language_name = MakeAutoReleasedCFRef(CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, language_name, kCFStringEncodingUTF8, kCFAllocatorNull));
      CFAttributedStringSetAttribute(attributed_string.get(), CFRangeMake(run.start, run.end-run.start), kCTLanguageAttributeName, native_language_name.get());
    }
  }
  CFAttributedStringEndEditing(attributed_string.get());

  auto framesetter = MakeAutoReleasedCFRef(CTFramesetterCreateWithAttributedString(attributed_string.get()));

  CFRange fit_range;
  auto suggested_frame_size = CTFramesetterSuggestFrameSizeWithConstraints(framesetter.get(), CFRangeMake(0, 0), nullptr, CGSizeMake(width, CGFLOAT_MAX), &fit_range);

  auto path = MakeAutoReleasedCFRef(CGPathCreateMutable());
  auto path_bounds = CGRectMake(0, 0, suggested_frame_size.width, suggested_frame_size.height);
  CGPathAddRect(path.get(), nullptr, path_bounds);

  auto frame = MakeAutoReleasedCFRef(CTFramesetterCreateFrame(framesetter.get(), CFRangeMake(0, 0), path.get(), nullptr));

  process_frame(frame.get());
}

void MiniCoreTextTypesetter::DrawToContext(TextBlock &text_block, const size_t width, CGContextRef context) {
  CGContextSetTextMatrix(context, CGAffineTransformIdentity);
  Typeset(text_block, width, [context](auto frame) {
    CTFrameDraw(frame, context);
  });
}

TypesetLines MiniCoreTextTypesetter::PositionGlyphs(TextBlock &text_block, const size_t width) {
  TypesetLines typeset_lines;
  Typeset(text_block, width, [&typeset_lines, &text_block](auto frame) {
    auto ct_lines = CTFrameGetLines(frame);
    auto lines_count = CFArrayGetCount(ct_lines);
    typeset_lines.resize(lines_count);
    auto text_content = text_block.text_content();
    auto text_length = text_block.text_length();
    for (ssize_t line_index = 0; line_index < lines_count; ++line_index) {
      auto ct_line = static_cast<CTLineRef>(CFArrayGetValueAtIndex(ct_lines, line_index));

      auto &generated_line = typeset_lines[line_index];

      auto runs = CTLineGetGlyphRuns(ct_line);
      auto runs_count = CFArrayGetCount(runs);
      for (ssize_t run_index = 0; run_index < runs_count; ++run_index) {
        auto run = static_cast<CTRunRef>(CFArrayGetValueAtIndex(runs, run_index));
        auto run_glyphs_count = CTRunGetGlyphCount(run);
        auto glyphs = CTRunGetGlyphsPtr(run);
        auto positions = CTRunGetPositionsPtr(run);
        auto offsets = CTRunGetStringIndicesPtr(run);

        auto attributes = CTRunGetAttributes(run);
        CTFontRef font = static_cast<CTFontRef>(CFDictionaryGetValue(attributes, kCTFontAttributeName));

        TypesetRun generated_run{};
        generated_run.font_size = float(CTFontGetSize(font));
        generated_run.font_descriptor = FontManager::CreateDescriptorFromNativeFont(font);
        for (ssize_t glyph_index = 0; glyph_index < run_glyphs_count; ++glyph_index) {
          auto cp = GetCodepoint(text_content, text_length, offsets[glyph_index]);
          if (IsCodepointToIgnoreForComparison(cp)) {
            continue;
          }
          generated_run.glyphs.push_back(TypesetRun::Glyph{
            .id = glyphs[glyph_index],
            .position = positions[glyph_index],
            .offset = offsets[glyph_index],
          });
        }
        if (generated_run.glyphs.size() > 0) {
          generated_line.runs.push_back(std::move(generated_run));
        }
      }
    }
  });
  return typeset_lines;
}

}
