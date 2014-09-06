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
#include "newline.hh"
#include "split_runs.hh"

#include <cassert>
#include <cmath>
#include <unistd.h>
#include <iostream>  // for debugging

#include <hb-ot.h>

#include <unicode/brkiter.h>

#include <CoreText/CoreText.h>

// TODO: The state should be in a lighter Typesetter object, not a separate struct

namespace glyphknit {

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
  ssize_t paragraph_start_offset;
  ssize_t paragraph_end_offset;
  TextBlock &text_block;
  TypesetLines &typeset_lines;
  double width;
  double current_x_position;
  UBreakIterator *line_break_iterator;
  UBreakIterator *grapheme_cluster_iterator;
};

void Typesetter::Shape(TypesettingState &state, ssize_t start_offset, ssize_t end_offset, FontDescriptor font_descriptor, Tag opentype_language_tag, UScriptCode script) {
  hb_buffer_clear_contents(state.hb_buffer);
  hb_buffer_add_utf16(state.hb_buffer, state.text_block.text_content(), (int)state.text_block.text_length(), (unsigned int)start_offset, (int)(end_offset-start_offset));
  hb_buffer_set_direction(state.hb_buffer, HB_DIRECTION_LTR);
  hb_buffer_set_language(state.hb_buffer, hb_ot_tag_to_language(opentype_language_tag));
  if (script != USCRIPT_COMMON && script != USCRIPT_INHERITED) {
    hb_buffer_set_script(state.hb_buffer, hb_script_from_string(uscript_getShortName(script), -1));
  }
  hb_shape(font_descriptor.GetHBFont(), state.hb_buffer, nullptr, 0);
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
      bool width_ignored_if_end_of_line = (at_start_of_cluster && at_end_of_cluster && u_isWhitespace(GetCodepoint(state.text_block.text_content(), state.paragraph_end_offset, current_glyph_cluster)));

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
  return state.paragraph_end_offset;
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
  auto run_end = std::min(script_run.start, line_run.start);  // TODO: language_run, font_descriptor_run, font_size_run, bidi_run

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

void Typesetter::TypesetParagraph(TypesettingState &state) {
  UErrorCode status = U_ZERO_ERROR;
  ubrk_setText(state.line_break_iterator, state.text_block.text_content()+state.paragraph_start_offset, (int32_t)(state.paragraph_end_offset-state.paragraph_start_offset), &status);
  assert(U_SUCCESS(status));
  ubrk_setText(state.grapheme_cluster_iterator, state.text_block.text_content()+state.paragraph_start_offset, (int32_t)(state.paragraph_end_offset-state.paragraph_start_offset), &status);
  assert(U_SUCCESS(status));

  StartNewLine(state);

  auto runs = SplitRuns(state.text_block, state.paragraph_start_offset, state.paragraph_end_offset);
  for (auto &run : runs) {
    ssize_t current_start_offset = run.start_index;
    ssize_t current_end_offset = run.end_index;
    int font_fallback_index = 0;
retry:
    auto font_descriptor = run.font_descriptor.GetFallback(font_fallback_index, run.language);
    Shape(state, current_start_offset, current_end_offset, font_descriptor, run.language.opentype_tag, run.script);

    auto glyphs_count = hb_buffer_get_length(state.hb_buffer);
    auto glyph_infos = hb_buffer_get_glyph_infos(state.hb_buffer, nullptr);

    for (ssize_t glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
      if (glyph_infos[glyph_index].codepoint == 0) { // no glyph for that character could be found in the font
        if (glyph_infos[glyph_index].cluster == current_start_offset) {
          for (glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
            if (glyph_infos[glyph_index].codepoint != 0) {
              current_end_offset = glyph_infos[glyph_index].cluster;
              break;
            }
          }  // if all characters had no glyphs, current_end_offset does not need to be changed
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

    const ssize_t width_in_font_units = SizeInFontUnits(state.width, font_descriptor, run.font_size);
    const ssize_t current_x_position_in_font_units = SizeInFontUnits(state.current_x_position, font_descriptor, run.font_size);
    auto fitting_glyphs_count = CountGlyphsThatFit(state, width_in_font_units - current_x_position_in_font_units);

    ssize_t break_offset;
    if (fitting_glyphs_count == glyphs_count) {
      break_offset = current_end_offset;
    }
    else {
      auto offset_after_fitting_glyphs = glyph_infos[fitting_glyphs_count].cluster;
      auto offset_after_not_fitting_glyph_cluster = FindTextOffsetAfterGlyphCluster(state, fitting_glyphs_count);

      break_offset = offset_after_not_fitting_glyph_cluster;
      do {
        break_offset = ubrk_preceding(state.line_break_iterator, (int32_t)(break_offset-state.paragraph_start_offset)) + state.paragraph_start_offset;
        // ignore line break boundaries that are not at grapheme cluster boundary
        // (for example between space and a combining mark)
      } while (!ubrk_isBoundary(state.grapheme_cluster_iterator, (int32_t)(break_offset-state.paragraph_start_offset)));

      if (break_offset <= current_start_offset) {
        // no line break boundary can fit, so we have to cut by grapheme cluster
        auto grapheme_clusters_count = CountGraphemeClusters(state.grapheme_cluster_iterator, offset_after_fitting_glyphs-state.paragraph_start_offset, offset_after_not_fitting_glyph_cluster-state.paragraph_start_offset) + state.paragraph_start_offset;
        if (grapheme_clusters_count == 1) {
          break_offset = offset_after_fitting_glyphs;
        }
        else {
          // we have to retry shaping with just a part of the glyph cluster as that might fit
          current_end_offset = ubrk_preceding(state.grapheme_cluster_iterator, (int32_t)(offset_after_not_fitting_glyph_cluster-state.paragraph_start_offset));
          goto retry;
        }
      }
      else {
        assert(break_offset <= offset_after_fitting_glyphs);  // if it is possible to have a line breakable in the middle of a glyph cluster, we would have to retry shaping with just a part of the glyph cluster
      }

      // reshape with the break offset found
      Shape(state, current_start_offset, break_offset, font_descriptor, run.language.opentype_tag, run.script);
    }

    OutputShape(state, font_descriptor, run.font_size);

    if (break_offset < run.end_index) {
      current_start_offset = break_offset;
      current_end_offset = run.end_index;
      StartNewLine(state);
      goto retry;
    }

    if (run.end_of_line) {
      StartNewLine(state);
    }
  }
}

void Typesetter::OutputShape(TypesettingState &state, FontDescriptor font_descriptor, float font_size) {
  auto glyphs_count = hb_buffer_get_length(state.hb_buffer);
  auto glyph_infos = hb_buffer_get_glyph_infos(state.hb_buffer, nullptr);
  auto glyph_pos = hb_buffer_get_glyph_positions(state.hb_buffer, nullptr);

  auto &last_line = state.typeset_lines.back();
  last_line.runs.emplace_back();
  auto &last_run = last_line.runs.back();
  auto &glyphs = last_run.glyphs;
  glyphs.resize(glyphs_count);

  last_run.font_size = font_size;
  last_run.font_descriptor = font_descriptor;

  const auto upem = hb_face_get_upem(hb_font_get_face(font_descriptor.GetHBFont()));

  double start_x = state.current_x_position;
  ssize_t base_x = 0, base_y = 0;
  for (unsigned int glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
    auto &glyph = glyphs[glyph_index];
    glyph.id = uint16_t(glyph_infos[glyph_index].codepoint);
    glyph.position = {
      .x = start_x + double(base_x + glyph_pos[glyph_index].x_offset) * font_size / upem,
      .y = double(base_y + glyph_pos[glyph_index].y_offset) * font_size / upem,
    };
    glyph.offset = glyph_infos[glyph_index].cluster;
    base_x += glyph_pos[glyph_index].x_advance;
    base_y += glyph_pos[glyph_index].y_advance;
  }
  state.current_x_position += double(base_x) * font_size / upem;
}

void Typesetter::PositionGlyphs(TextBlock &text_block, size_t width, TypesetLines &typeset_lines) {
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
    .text_block = text_block,
    .typeset_lines = typeset_lines,
    .width = double(width),
    .line_break_iterator = line_break_iterator_,
    .grapheme_cluster_iterator = grapheme_cluster_iterator_,
  };

  ParagraphIterator paragraph_iterator{text_content, 0, text_length};
  for (auto paragraph = paragraph_iterator.FindNext(); paragraph.start < text_length; paragraph = paragraph_iterator.FindNext()) {
    state.paragraph_start_offset = paragraph.start;
    state.paragraph_end_offset = paragraph.end;
    TypesetParagraph(state);
  }
}

static CGFloat CoreTextLineHeight(CTFontRef font) {
  auto ascent = std::round(CTFontGetAscent(font));
  auto descent = std::round(CTFontGetDescent(font));
  auto leading = std::round(CTFontGetLeading(font));
  return ascent + descent + leading;
}

void Typesetter::DrawToContext(TextBlock &text_block, const size_t width, CGContextRef context) {
  TypesetLines typeset_lines;
  PositionGlyphs(text_block, width, typeset_lines);

  // TODO: use line heights from lines and/or runs
  const auto &default_font_descriptor = text_block.attributes_runs().front().attributes.font_descriptor;
  auto default_font_size = text_block.attributes_runs().front().attributes.font_size;
  auto default_ct_font = default_font_descriptor.CreateNativeFont(default_font_size);

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
      auto native_font = run.font_descriptor.CreateNativeFont(run.font_size);
      CTFontDrawGlyphs(native_font.get(), glyph_ids.data(), glyph_positions.data(), run.glyphs.size(), context);
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
