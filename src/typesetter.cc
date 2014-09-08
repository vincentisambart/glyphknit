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
#include <CoreText/CoreText.h>

namespace glyphknit {

static ssize_t CountGraphemeClusters(UBreakIterator *grapheme_cluster_iterator, ssize_t start_offset, ssize_t end_offset) {
  ssize_t grapheme_clusters_count = 0;
  auto offset = start_offset;
  while (offset < end_offset) {
    offset = ubrk_following(grapheme_cluster_iterator, int32_t(offset));
    ++grapheme_clusters_count;
  }
  return grapheme_clusters_count;
}

void Typesetter::Shape(const TextBlock &text_block, ssize_t start_index, ssize_t end_index, FontDescriptor font_descriptor, Tag opentype_language_tag, UScriptCode script) {
  hb_buffer_clear_contents(hb_buffer_);
  hb_buffer_add_utf16(hb_buffer_, text_block.text_content(), int32_t(text_block.text_length()), uint32_t(start_index), int32_t(end_index-start_index));
  hb_buffer_set_direction(hb_buffer_, HB_DIRECTION_LTR);
  hb_buffer_set_language(hb_buffer_, hb_ot_tag_to_language(opentype_language_tag));
  if (script != USCRIPT_COMMON && script != USCRIPT_INHERITED) {
    hb_buffer_set_script(hb_buffer_, hb_script_from_string(uscript_getShortName(script), -1));
  }
  hb_shape(font_descriptor.GetHBFont(), hb_buffer_, nullptr, 0);
}

ssize_t Typesetter::CountGlyphsThatFit(const TextBlock &text_block, ssize_t width, ssize_t paragraph_end_index) {
  auto glyphs_count = hb_buffer_get_length(hb_buffer_);
  auto glyph_positions = hb_buffer_get_glyph_positions(hb_buffer_, nullptr);
  auto glyph_infos = hb_buffer_get_glyph_infos(hb_buffer_, nullptr);

  ssize_t glyphs_fitting_count = 0;

  ssize_t x_position = 0;
  for (ssize_t glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
    assert(glyph_index == 0 || glyph_infos[glyph_index-1].cluster <= glyph_infos[glyph_index].cluster);  // if it's not the case we need to reorder the clusters just after shaping

    if (glyph_index > 0) {  // the first glyph always fits
      uint32_t current_glyph_cluster = glyph_infos[glyph_index].cluster;
      bool at_start_of_cluster = (glyph_index == 0 || glyph_infos[glyph_index-1].cluster != current_glyph_cluster);
      bool at_end_of_cluster = (glyph_index == glyphs_count - 1 || glyph_infos[glyph_index+1].cluster != current_glyph_cluster);
      // u_isWhitespace does not include no-break spaces because they must be handled as non-spacing characters at the end of a line
      bool width_ignored_if_end_of_line = (at_start_of_cluster && at_end_of_cluster && u_isWhitespace(GetCodepoint(text_block.text_content(), paragraph_end_index, current_glyph_cluster)));

      if (!width_ignored_if_end_of_line && x_position + glyph_positions[glyph_index].x_advance > width) {
        return glyphs_fitting_count;
      }
    }
    ++glyphs_fitting_count;
    x_position += glyph_positions[glyph_index].x_advance;
  }
  return glyphs_fitting_count;
}

ssize_t Typesetter::FindTextOffsetAfterGlyphCluster(ssize_t glyph_index, ssize_t paragraph_end_index) {
  auto glyphs_count = hb_buffer_get_length(hb_buffer_);
  auto glyph_infos = hb_buffer_get_glyph_infos(hb_buffer_, nullptr);

  ssize_t glyph_cluster = glyph_infos[glyph_index].cluster;
  for (ssize_t compared_glyph_index = glyph_index+1; compared_glyph_index < glyphs_count; ++compared_glyph_index) {
    if (glyph_cluster != glyph_infos[compared_glyph_index].cluster) {
      return glyph_infos[compared_glyph_index].cluster;
    }
  }
  return paragraph_end_index;
}

#if 0

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

ssize_t Typesetter::PreviousBreak(ssize_t index, ssize_t paragraph_start_index) {
  do {
    index = ubrk_preceding(line_break_iterator_, int32_t(index-paragraph_start_index)) + paragraph_start_index;
    // ignore line break boundaries that are not at grapheme cluster boundary
    // (for example between space and a combining mark)
  } while (!ubrk_isBoundary(grapheme_cluster_iterator_, int32_t(index-paragraph_start_index)));
  return index;
}


TypesetLines Typesetter::TypesetParagraph(const TextBlock &text_block, ssize_t paragraph_start_index, ssize_t paragraph_end_index, double available_width) {
  TypesetLines typeset_lines;
  double current_text_width = 0;

  UErrorCode status = U_ZERO_ERROR;
  ubrk_setText(line_break_iterator_, text_block.text_content()+paragraph_start_index, int32_t(paragraph_end_index-paragraph_start_index), &status);
  assert(U_SUCCESS(status));
  ubrk_setText(grapheme_cluster_iterator_, text_block.text_content()+paragraph_start_index, int32_t(paragraph_end_index-paragraph_start_index), &status);
  assert(U_SUCCESS(status));

  bool has_saved_line_break = false;
  bool saved_at_end_of_run;
  // TODO: rename ParagraphRuns (as it may look like it's runs of paragraphs even though it's runs inside a paragraph)
  ParagraphRuns::iterator saved_run;
  ssize_t saved_start_index;
  ssize_t saved_line_break_point_index;
  ssize_t saved_line_runs_size;
  ssize_t saved_text_width;

  auto StartNewLine = [&]() {
    typeset_lines.emplace_back();
    current_text_width = 0;
    has_saved_line_break = false;
  };

  StartNewLine();

  auto runs = SplitRuns(text_block, paragraph_start_index, paragraph_end_index);
  auto runs_end = runs.end();
  for (auto current_run = runs.begin(); current_run != runs_end; ++current_run) {
    ssize_t current_start_index = current_run->start_index;
    ssize_t current_end_index = current_run->end_index;
    int font_fallback_index = 0;
reshape_part_of_run:
    ssize_t previous_text_width = current_text_width;
    auto &run = *current_run;  // TODO: use directly current_run and remove this reference
    auto font_descriptor = run.font_descriptor.GetFallback(font_fallback_index, run.language);
    Shape(text_block, current_start_index, current_end_index, font_descriptor, run.language.opentype_tag, run.script);

    auto glyphs_count = hb_buffer_get_length(hb_buffer_);
    auto glyph_infos = hb_buffer_get_glyph_infos(hb_buffer_, nullptr);

    for (ssize_t glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
      if (glyph_infos[glyph_index].codepoint == 0) { // no glyph for that character could be found in the font
        if (glyph_infos[glyph_index].cluster == current_start_index) {
          for (glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
            if (glyph_infos[glyph_index].codepoint != 0) {
              current_end_index = glyph_infos[glyph_index].cluster;
              break;
            }
          }  // if no character had any glyph found, current_end_index does not need to be changed
          ++font_fallback_index;
          goto reshape_part_of_run;
        }
        else {
          current_end_index = glyph_infos[glyph_index].cluster;
          break;
        }
      }
    }
    font_fallback_index = 0;

    const ssize_t width_in_font_units = SizeInFontUnits(available_width, font_descriptor, run.font_size);
    const ssize_t current_x_position_in_font_units = SizeInFontUnits(current_text_width, font_descriptor, run.font_size);
    auto fitting_glyphs_count = CountGlyphsThatFit(text_block, width_in_font_units - current_x_position_in_font_units, paragraph_end_index);

    ssize_t break_offset;
    if (fitting_glyphs_count == glyphs_count) {
      break_offset = current_end_index;
    }
    else {
      auto offset_after_fitting_glyphs = glyph_infos[fitting_glyphs_count].cluster;
      auto offset_after_not_fitting_glyph_cluster = FindTextOffsetAfterGlyphCluster(fitting_glyphs_count, paragraph_end_index);

      break_offset = PreviousBreak(offset_after_not_fitting_glyph_cluster, paragraph_start_index);

      if (break_offset <= current_start_index) {
        if (has_saved_line_break) {
          has_saved_line_break = false;
          auto &typeset_line = typeset_lines.back();
          current_run = saved_run;
          if (saved_at_end_of_run) {
            typeset_line.runs.erase(typeset_line.runs.begin()+saved_line_runs_size, typeset_line.runs.end());
            StartNewLine();
            continue;
          }
          else {
            typeset_line.runs.erase(typeset_line.runs.begin()+(saved_line_runs_size-1), typeset_line.runs.end());
            current_start_index = saved_start_index;
            current_end_index = saved_line_break_point_index;
            current_text_width = saved_text_width;
            goto reshape_part_of_run;
          }
        }
        // no line break boundary can fit, so we have to cut by grapheme cluster
        auto grapheme_clusters_count = CountGraphemeClusters(grapheme_cluster_iterator_, offset_after_fitting_glyphs-paragraph_start_index, offset_after_not_fitting_glyph_cluster-paragraph_start_index) + paragraph_start_index;
        if (grapheme_clusters_count == 1) {
          break_offset = offset_after_fitting_glyphs;
        }
        else {
          // we have to retry shaping with just a part of the glyph cluster as that might fit
          current_end_index = ubrk_preceding(grapheme_cluster_iterator_, int32_t(offset_after_not_fitting_glyph_cluster-paragraph_start_index));
          goto reshape_part_of_run;
        }
      }
      else {
        assert(break_offset <= offset_after_fitting_glyphs);  // if it is possible to have a line breakable in the middle of a glyph cluster, we would have to retry shaping with just a part of the glyph cluster
      }

      // reshape with the break offset found
      Shape(text_block, current_start_index, break_offset, font_descriptor, run.language.opentype_tag, run.script);
    }

    OutputShape(typeset_lines, current_text_width, font_descriptor, run.font_size);

    if (break_offset < run.end_index) {
      current_start_index = break_offset;
      current_end_index = run.end_index;
      StartNewLine();
      goto reshape_part_of_run;
    }

    if (run.end_of_line) {
      StartNewLine();
    }
    else {
      auto &typeset_line = typeset_lines.back();
      if (ubrk_isBoundary(line_break_iterator_, int32_t(current_end_index-paragraph_start_index))
          && ubrk_isBoundary(grapheme_cluster_iterator_, int32_t(current_end_index-paragraph_start_index))) {
        has_saved_line_break = true;
        saved_at_end_of_run = true;
        saved_run = current_run;
        saved_line_runs_size = typeset_line.runs.size();
      }
      else {
        ssize_t possible_break_index = PreviousBreak(current_end_index, paragraph_start_index);
        if (possible_break_index > current_start_index && possible_break_index < current_end_index) {
          has_saved_line_break = true;
          saved_at_end_of_run = false;
          saved_run = current_run;
          saved_start_index = current_start_index;
          saved_line_break_point_index = possible_break_index;
          saved_line_runs_size = typeset_line.runs.size();
          saved_text_width = previous_text_width;
        }
      }
    }
  }

  return typeset_lines;
}

void Typesetter::OutputShape(TypesetLines &typeset_lines, double &current_text_width, FontDescriptor font_descriptor, float font_size) {
  auto glyphs_count = hb_buffer_get_length(hb_buffer_);
  auto glyph_infos = hb_buffer_get_glyph_infos(hb_buffer_, nullptr);
  auto glyph_pos = hb_buffer_get_glyph_positions(hb_buffer_, nullptr);

  auto &last_line = typeset_lines.back();
  last_line.runs.emplace_back();
  auto &last_run = last_line.runs.back();
  auto &glyphs = last_run.glyphs;
  glyphs.resize(glyphs_count);

  last_run.font_size = font_size;
  last_run.font_descriptor = font_descriptor;

  const auto upem = hb_face_get_upem(hb_font_get_face(font_descriptor.GetHBFont()));

  double start_x = current_text_width;
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
  current_text_width += double(base_x) * font_size / upem;
}

TypesetLines Typesetter::PositionGlyphs(TextBlock &text_block, double available_width) {
  TypesetLines typeset_lines;

  ParagraphIterator paragraph_iterator{text_block.text_content(), 0, text_block.text_length()};
  for (auto paragraph = paragraph_iterator.FindNext(); paragraph.start < text_block.text_length(); paragraph = paragraph_iterator.FindNext()) {
    auto paragraph_lines = TypesetParagraph(text_block, paragraph.start, paragraph.end, available_width);
    typeset_lines.insert(typeset_lines.end(), paragraph_lines.begin(), paragraph_lines.end());
  }
  return typeset_lines;
}

static CGFloat CoreTextLineHeight(CTFontRef font) {
  auto ascent = std::round(CTFontGetAscent(font));
  auto descent = std::round(CTFontGetDescent(font));
  auto leading = std::round(CTFontGetLeading(font));
  return ascent + descent + leading;
}

void Typesetter::DrawToContext(TextBlock &text_block, size_t available_width, CGContextRef context) {
  TypesetLines typeset_lines = PositionGlyphs(text_block, available_width);

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
  hb_buffer_ = hb_buffer_create();
}

Typesetter::~Typesetter() {
  hb_buffer_destroy(hb_buffer_);
  ubrk_close(grapheme_cluster_iterator_);
  ubrk_close(line_break_iterator_);
}

}
