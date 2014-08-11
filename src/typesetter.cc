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

#include "typesetter.hh"
#include "at_scope_exit.hh"
#include "autorelease.hh"
#include "utf.hh"

#include <cassert>
#include <cmath>
#include <unistd.h>
#include <iostream>  // for debugging

#include <unicode/brkiter.h>

#include <CoreText/CoreText.h>

// TODO: The state should be in a lighter Typesetter object, not a separate struct

namespace glyphknit {

// The Unicode specification (section 5.8) splits paragaph and line breaking characters.
// The character concerned are CR (U+000D), LF (U+000A), NEL (U+0085), VT (U+000B), FF (U+000C), LS (U+2028), PS (U+2029).
// "CR, LF, CRLF, and NEL should be treated the same" (they are called "NLF" below)
// "In word processing, interpret any NLF the same as PS." (we'll be using the behavior for word processing apps)
// "Always interpret PS as paragraph separator and LS as line separator."
// The spec says that "FF does not interrupt a paragraph" meaning it's not a paragraph separator.
// The specification also mentions that VT is used as a line separator in Microsoft Word.

static bool IsLineSeparator(UChar32 c) {
  switch (c) {
    case 0x000B: // LINE/VERTICAL TABULATION (VT) \v
    case 0x000C: // FORM FEED (FF) \f
    case 0x2028: // LINE SEPARATOR (LS)
      return true;
    default:
      return false;
  }
}

// be careful as CR+LF should be handled as a single separator
static bool IsParagraphSeparator(UChar32 c) {
  switch (c) {
    case 0x000A: // LINE FEED (LF) \n
    case 0x000D: // CARRIAGE RETURN (CR) \r
    case 0x0085: // NEXT LINE (NEL)
    case 0x2029: // PARAGRAPH SEPARATOR (PS)
      return true;
    default:
      return false;
  }
}

static ssize_t CountGraphemeClusters(UBreakIterator *grapheme_cluster_iterator, ssize_t start_offset, ssize_t end_offset) {
  ssize_t grapheme_clusters_count = 0;
  auto offset = start_offset;
  while (offset < end_offset) {
    offset = ubrk_following(grapheme_cluster_iterator, (int32_t)offset);
    ++grapheme_clusters_count;
  }
  return grapheme_clusters_count;
}

struct TypesettingState {
  hb_buffer_t *hb_buffer;
  std::shared_ptr<FontFace> font_face;
  float font_size;
  ssize_t  paragraph_start_offset;
  const uint16_t *paragraph_text;
  ssize_t paragraph_length;
  TypesetLines &typeset_lines;
  ssize_t width;
  ssize_t current_x_position;
  UBreakIterator *line_break_iterator;
  UBreakIterator *grapheme_cluster_iterator;
};

void Typesetter::Shape(TypesettingState &state, ssize_t start_offset, ssize_t end_offset) {
  hb_buffer_clear_contents(state.hb_buffer);
  hb_buffer_add_utf16(state.hb_buffer, state.paragraph_text, (int)state.paragraph_length, (unsigned int)start_offset, (int)(end_offset-start_offset));
  hb_buffer_set_direction(state.hb_buffer, HB_DIRECTION_LTR);
  hb_shape(state.font_face->GetHBFont(), state.hb_buffer, nullptr, 0);
}

ssize_t Typesetter::CountGlyphsThatFit(TypesettingState &state, ssize_t width) {
  auto glyphs_count = hb_buffer_get_length(state.hb_buffer);
  auto glyph_positions = hb_buffer_get_glyph_positions(state.hb_buffer, nullptr);
  auto glyph_infos = hb_buffer_get_glyph_infos(state.hb_buffer, nullptr);

  ssize_t glyphs_fitting_count = 0;

  ssize_t x_position = 0;
  for (ssize_t glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
    assert(glyph_index == 0 || glyph_infos[glyph_index-1].cluster <= glyph_infos[glyph_index].cluster);  // if it's not the case we need to reorder the clusters just after shaping

    if (glyph_index > 0) {  // the first glyph always fits
      uint32_t current_glyph_cluster = glyph_infos[glyph_index].cluster;
      bool at_start_of_cluster = (glyph_index == 0 || glyph_infos[glyph_index-1].cluster != current_glyph_cluster);
      bool at_end_of_cluster = (glyph_index == glyphs_count - 1 || glyph_infos[glyph_index+1].cluster != current_glyph_cluster);
      // u_isWhitespace does not include no-break spaces because they must be handled as non-spacing characters at the end of a line
      bool width_ignored_if_end_of_line = (at_start_of_cluster && at_end_of_cluster && u_isWhitespace(GetCodepoint(state.paragraph_text, state.paragraph_length, current_glyph_cluster)));

      if (!width_ignored_if_end_of_line && x_position + glyph_positions[glyph_index].x_advance > width) {
        return glyphs_fitting_count;
      }
    }
    ++glyphs_fitting_count;
    x_position += glyph_positions[glyph_index].x_advance;
  }
  return glyphs_fitting_count;
}

ssize_t Typesetter::FindTextOffsetAfterGlyphCluster(const TypesettingState &state, ssize_t glyph_index) {
  auto glyphs_count = hb_buffer_get_length(state.hb_buffer);
  auto glyph_infos = hb_buffer_get_glyph_infos(state.hb_buffer, nullptr);

  ssize_t glyph_cluster = glyph_infos[glyph_index].cluster;
  for (ssize_t compared_glyph_index = glyph_index+1; compared_glyph_index < glyphs_count; ++compared_glyph_index) {
    if (glyph_cluster != glyph_infos[compared_glyph_index].cluster) {
      return glyph_infos[compared_glyph_index].cluster;
    }
  }
  return state.paragraph_length;
}

void Typesetter::StartNewLine(TypesettingState &state) {
  state.typeset_lines.emplace_back();
  state.current_x_position = 0;
}

#if 0

/*
  Run types:
  - script
  - language (based on script)
  - grapheme cluster
  - forced line breaks boundaries
  - possible line break boundaries
  - font face, font size
  - bidi
  TODO:
  - infered properties (like language, font face) should be saved as attributes (but the original value must also be kept)
    pb: script (for common/inherited scripts) and so language can change depending on the previous or next characters
*/

/*
  In fact after the first shaping 2 cases: it fully fits or not.
  If it fits, push it on the line (note that later on backtracking is still possible though so you need to get the last possible line break).
  If it doesn't fit, find up to where it fits and try reshaping (we might end up not using any character of the run on the current line, especially if it's not the first run on the line)

  If some glyphs are 0 (glyph not found in glyph), stop the run just before. If it's the first glyphs of the run, try other fonts
*/

void Typesetter::NewTypesetParagraph(const uint16_t *paragraph_text, ssize_t paragraph_length) {
  ScriptIterator script_iterator{paragraph_text, paragraph_length};
  // TODO: be careful as the LineIterator skips over some characters (line ends), something other iterators won't do
  LineIterator line_iterator{paragraph_text, paragraph_length};

  auto script_run = script_iterator.FindNextRun();
  auto line_run = line_iterator.FindNextRun();

  ssize_t run_start = 0;
  auto run_end = std::min(script_run.start, line_run.start);  // TODO: language_run, font_face_run, font_size_run, bidi_run

  // TODO: be careful for the case where the line breaking point in inside a ligature

  Shape(paragraph_text, run_start, run_end);
  for (each glyph) {
    if (too large) {
      if (glyph only for one grapheme cluster) {
        if (firstCharacterOfLine) {

        }
        else {
          GoBackToSavePoint();
          StartNextLine();
        }
      }
      else {
        retry with smaller shape_until_offset if first character or glyph includes line break
      }
    }
    PushGlyph();

    if (possible_line_break) {
      MakeSavePoint();
    }
  }
  if (font_changed || script_changed || end_of_bidi_run) {
    MakeSavePoint();
  }
}
#endif

void Typesetter::TypesetLine(TypesettingState &state, ssize_t line_start_offset, ssize_t line_end_offset) {
  if (line_start_offset >= line_end_offset) {
    return;
  }
  ssize_t current_start_offset = line_start_offset, current_end_offset = line_end_offset;

retry:
  Shape(state, current_start_offset, current_end_offset);

  auto glyphs_count = hb_buffer_get_length(state.hb_buffer);
  auto glyph_infos = hb_buffer_get_glyph_infos(state.hb_buffer, nullptr);

/*
  for (ssize_t glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
    if (glyph_infos[glyph_index].codepoint == 0) { // no glyph for that character could be found in the font
      if (glyph_infos[glyph_index].cluster == current_start_offset) {
        for (glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
          if (glyph_infos[glyph_index].codepoint != 0) {
            current_end_offset = glyph_infos[glyph_index].cluster;
            break;
          }
        }  // if all characters had not glyphs, current_end_offset does not need to be changed
        ++font_fallback_index;
        goto retry;
      }
      else {
        current_end_offset = glyph_infos[glyph_index].cluster;
        break;
      }
    }
  }
  font_fallback_index = 0;
*/

  auto fitting_glyphs_count = CountGlyphsThatFit(state, state.width - state.current_x_position);
  auto offset_after_fitting_glyphs = glyph_infos[fitting_glyphs_count].cluster;

  ssize_t break_offset;
  if (fitting_glyphs_count == glyphs_count) {
    break_offset = current_end_offset;
  }
  else {
    auto offset_after_not_fitting_glyph_cluster = FindTextOffsetAfterGlyphCluster(state, fitting_glyphs_count);

    break_offset = offset_after_not_fitting_glyph_cluster;
    do {
      break_offset = ubrk_preceding(state.line_break_iterator, (int32_t)break_offset);
      // ignore line break boundaries that are not at grapheme cluster boundary
      // (for example between space and a combining mark)
    } while (!ubrk_isBoundary(state.grapheme_cluster_iterator, (int32_t)break_offset));

    if (break_offset <= current_start_offset) {
      // no line break boundary can fit, so we have to cut by grapheme cluster
      auto grapheme_clusters_count = CountGraphemeClusters(state.grapheme_cluster_iterator, offset_after_fitting_glyphs, offset_after_not_fitting_glyph_cluster);
      if (grapheme_clusters_count == 1) {
        break_offset = offset_after_fitting_glyphs;
      }
      else {
        // we have to retry shaping with just a part of the glyph cluster as that might fit
        current_end_offset = ubrk_preceding(state.grapheme_cluster_iterator, (int32_t)offset_after_not_fitting_glyph_cluster);
        goto retry;
      }
    }
    else {
      assert(break_offset <= offset_after_fitting_glyphs);  // if it is possible to have a line breakable in the middle of a glyph cluster, would have to retry shaping with just a part of the glyph cluster
    }

    // reshape with the break offset found
    Shape(state, current_start_offset, break_offset);
  }

  OutputShape(state);

  current_start_offset = break_offset;
  current_end_offset = line_end_offset;
  if (current_start_offset < current_end_offset) {
    StartNewLine(state);
    goto retry;
  }
}

void Typesetter::TypesetParagraph(TypesettingState &state) {
  StartNewLine(state);

  UErrorCode status = U_ZERO_ERROR;
  ubrk_setText(state.line_break_iterator, state.paragraph_text, (int32_t)state.paragraph_length, &status);
  assert(U_SUCCESS(status));
  ubrk_setText(state.grapheme_cluster_iterator, state.paragraph_text, (int32_t)state.paragraph_length, &status);
  assert(U_SUCCESS(status));

  ssize_t offset = 0, line_start_offset = 0;
  while (offset < state.paragraph_length) {
    auto codepoint_start_offset = offset;
    if (IsLineSeparator(ConsumeCodepoint(state.paragraph_text, state.paragraph_length, offset))) {
      TypesetLine(state, line_start_offset, codepoint_start_offset);
      StartNewLine(state);
      line_start_offset = offset;
    }
  }
  TypesetLine(state, line_start_offset, state.paragraph_length);
}

void Typesetter::OutputShape(TypesettingState &state) {
  auto glyphs_count = hb_buffer_get_length(state.hb_buffer);
  auto glyph_infos = hb_buffer_get_glyph_infos(state.hb_buffer, nullptr);
  auto glyph_pos = hb_buffer_get_glyph_positions(state.hb_buffer, nullptr);

  auto &last_line = state.typeset_lines[state.typeset_lines.size()-1];
  last_line.runs.emplace_back();
  auto &last_run = last_line.runs[last_line.runs.size()-1];
  auto &glyphs = last_run.glyphs;
  glyphs.resize(glyphs_count);

  last_run.font_size = state.font_size;
  last_run.font_face = state.font_face;

  const auto upem = hb_face_get_upem(hb_font_get_face(state.font_face->GetHBFont()));

  ssize_t base_x = state.current_x_position, base_y = 0;
  for (unsigned int glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
    auto &glyph = glyphs[glyph_index];
    glyph.id = uint16_t(glyph_infos[glyph_index].codepoint);
    glyph.position = {
      .x = double(base_x + glyph_pos[glyph_index].x_offset) * state.font_size / upem,
      .y = double(base_y + glyph_pos[glyph_index].y_offset) * state.font_size / upem,
    };
    glyph.offset = glyph_infos[glyph_index].cluster + state.paragraph_start_offset;
    base_x += glyph_pos[glyph_index].x_advance;
    base_y += glyph_pos[glyph_index].y_advance;
  }
  state.current_x_position = base_x;
}

void Typesetter::PositionGlyphs(TextBlock &text_block, size_t width, TypesetLines &typeset_lines) {
  const auto &font_face = text_block.default_font_face();
  auto ft_face = font_face->GetFTFace();

  auto font_size = text_block.default_font_size();
  const ssize_t width_in_font_units = size_t(double(width) * ft_face->units_per_EM / font_size);

  auto hb_buffer = hb_buffer_create();
  AT_SCOPE_EXIT([&hb_buffer] {
    hb_buffer_destroy(hb_buffer);
    hb_buffer = nullptr;
  });

  auto text_content = text_block.text_content();
  auto text_length = text_block.text_length();
  typeset_lines.clear();

  TypesettingState state = {
    .hb_buffer = hb_buffer,
    .font_face = font_face,
    .font_size = font_size,
    .typeset_lines = typeset_lines,
    .width = width_in_font_units,
    .line_break_iterator = line_break_iterator_,
    .grapheme_cluster_iterator = grapheme_cluster_iterator_,
  };

  ssize_t offset = 0, paragraph_start_offset = 0;
  while (offset < text_length) {
    auto codepoint_start_offset = offset;
    auto c = ConsumeCodepoint(text_content, text_length, offset);
    if (IsParagraphSeparator(c)) {
      state.paragraph_start_offset = paragraph_start_offset;
      state.paragraph_text = text_content+paragraph_start_offset;
      state.paragraph_length = codepoint_start_offset-paragraph_start_offset;
      state.current_x_position = 0;
      TypesetParagraph(state);
      if (offset < text_length && c == '\r' && text_content[offset] == '\n') {
        ++offset;
      }
      paragraph_start_offset = offset;
    }
  }
  state.paragraph_start_offset = paragraph_start_offset;
  state.paragraph_text = text_content+paragraph_start_offset;
  state.paragraph_length = text_length-paragraph_start_offset;
  state.current_x_position = 0;
  TypesetParagraph(state);
}

static CGFloat CoreTextLineHeight(CTFontRef font) {
  auto ascent = std::round(CTFontGetAscent(font));
  auto descent = std::round(CTFontGetDescent(font));
  auto leading = std::round(CTFontGetLeading(font));
  return ascent + descent + leading;
}

void Typesetter::DrawToContext(TextBlock &text_block, const size_t width, CGContextRef context) {
  const auto &default_font_face = text_block.default_font_face();
  auto default_font_size = text_block.default_font_size();
  auto default_ct_font = default_font_face->CreateCTFont(default_font_size);

  TypesetLines typeset_lines;
  PositionGlyphs(text_block, width, typeset_lines);

  CGContextSetTextMatrix(context, CGAffineTransformIdentity);
  CGFloat total_height = 0;
  auto lines_end = typeset_lines.end();
  for (auto line_it = typeset_lines.begin(); line_it != lines_end; ++line_it) {
    total_height += CoreTextLineHeight(default_ct_font.get());
  }
  total_height += std::round(CTFontGetDescent(default_ct_font.get())) + 0.5;
  CGContextTranslateCTM(context, 0, total_height);

  std::vector<GlyphId> glyph_ids;
  std::vector<GlyphPosition> glyph_positions;
  for (auto &line : typeset_lines) {
    CGContextTranslateCTM(context, 0, -CoreTextLineHeight(default_ct_font.get()));
    for (auto &run : line.runs) {
      glyph_ids.resize(0);
      glyph_positions.resize(0);
      glyph_ids.reserve(run.glyphs.size());
      glyph_positions.reserve(run.glyphs.size());
      for (auto &glyph : run.glyphs) {
        glyph_ids.push_back(glyph.id);
        glyph_positions.push_back(glyph.position);
      }
      CTFontDrawGlyphs(default_ct_font.get(), glyph_ids.data(), glyph_positions.data(), run.glyphs.size(), context);
    }
  }
  // TODO: use the ascent/descent/leading from the line, not from the font
}

Typesetter::Typesetter() {
  // TODO: maybe use a different line break iterator for each locale (at least for locales having a custom one)?
  UErrorCode status = U_ZERO_ERROR;
  line_break_iterator_ = ubrk_open(UBRK_LINE, "en", nullptr, 0, &status);
  assert(U_SUCCESS(status));
  grapheme_cluster_iterator_ = ubrk_open(UBRK_CHARACTER, "en", nullptr, 0, &status);
  assert(U_SUCCESS(status));
}

Typesetter::~Typesetter() {
  ubrk_close(grapheme_cluster_iterator_);
  ubrk_close(line_break_iterator_);
}

}
