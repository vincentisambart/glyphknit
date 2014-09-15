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

void Typesetter::Shape(const TextBlock &text_block, ssize_t start_index, ssize_t end_index, FontDescriptor font_descriptor, Tag opentype_language_tag, UScriptCode script, UBiDiDirection bidi_direction) {
  hb_buffer_clear_contents(hb_buffer_);
  hb_buffer_add_utf16(hb_buffer_, text_block.text_content(), int32_t(text_block.text_length()), uint32_t(start_index), int32_t(end_index-start_index));
  hb_buffer_set_direction(hb_buffer_, bidi_direction == UBIDI_RTL ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
  hb_buffer_set_language(hb_buffer_, hb_ot_tag_to_language(opentype_language_tag));
  if (script != USCRIPT_COMMON && script != USCRIPT_INHERITED) {
    hb_buffer_set_script(hb_buffer_, hb_script_from_string(uscript_getShortName(script), -1));
  }
  hb_shape(font_descriptor.GetHBFont(), hb_buffer_, nullptr, 0);
}

ssize_t Typesetter::CountGlyphsThatFit(const TextBlock &text_block, ssize_t width, bool start_of_line) {
  auto glyphs_count = hb_buffer_get_length(hb_buffer_);
  auto glyph_positions = hb_buffer_get_glyph_positions(hb_buffer_, nullptr);
  auto glyph_infos = hb_buffer_get_glyph_infos(hb_buffer_, nullptr);
  auto direction = hb_buffer_get_direction(hb_buffer_);

  ssize_t glyphs_fitting_count = 0;

  ssize_t x_position = 0;

  for (ssize_t relative_glyph_index = 0; relative_glyph_index < glyphs_count; ++relative_glyph_index) {
    ssize_t glyph_index, previous_glyph_index, next_glyph_index;
    if (HB_DIRECTION_IS_FORWARD(direction)) {
      glyph_index = relative_glyph_index;
      previous_glyph_index = glyph_index - 1;
      next_glyph_index = glyph_index + 1;
    }
    else {
      glyph_index = glyphs_count-relative_glyph_index-1;
      previous_glyph_index = glyph_index + 1;
      next_glyph_index = glyph_index - 1;
    }
    assert(relative_glyph_index == 0 || glyph_infos[previous_glyph_index].cluster <= glyph_infos[glyph_index].cluster);  // if it's not the case we need to reorder the clusters just after shaping

    if (relative_glyph_index > 0 || !start_of_line) {  // the first glyph of a line always fits
      uint32_t current_glyph_cluster = glyph_infos[glyph_index].cluster;
      bool at_start_of_cluster = (relative_glyph_index == 0 || glyph_infos[previous_glyph_index].cluster != current_glyph_cluster);
      bool at_end_of_cluster = (relative_glyph_index == glyphs_count - 1 || glyph_infos[next_glyph_index].cluster != current_glyph_cluster);
      // u_isWhitespace does not include no-break spaces because they must be handled as non-spacing characters at the end of a line
      bool width_ignored_if_end_of_line = (at_start_of_cluster && at_end_of_cluster && u_isWhitespace(GetCodepoint(text_block.text_content(), text_block.text_length(), current_glyph_cluster)));

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
  auto direction = hb_buffer_get_direction(hb_buffer_);

  ssize_t glyph_cluster = glyph_infos[glyph_index].cluster;
  if (HB_DIRECTION_IS_FORWARD(direction)) {
    for (ssize_t compared_glyph_index = glyph_index+1; compared_glyph_index < glyphs_count; ++compared_glyph_index) {
      if (glyph_cluster != glyph_infos[compared_glyph_index].cluster) {
        return glyph_infos[compared_glyph_index].cluster;
      }
    }
  }
  else {
    for (ssize_t compared_glyph_index = glyph_index-1; compared_glyph_index >= 0; --compared_glyph_index) {
      if (glyph_cluster != glyph_infos[compared_glyph_index].cluster) {
        return glyph_infos[compared_glyph_index].cluster;
      }
    }
  }
  return paragraph_end_index;
}

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

  bool broke_line = false;

  bool has_saved_line_break = false;
  // all the saves_xxxx variables below do not need to be initialized but they are to silence the compiler
  bool saved_at_end_of_run = false;
  ListOfRuns::iterator saved_run;
  ssize_t saved_start_index = 0;
  ssize_t saved_line_break_point_index = 0;
  ssize_t saved_line_runs_size = 0;
  double saved_text_width = 0;

  auto StartNewLine = [&]() {
    typeset_lines.emplace_back();
    current_text_width = 0;
    has_saved_line_break = false;
    broke_line = false;
  };

  StartNewLine();

  auto runs = SplitRuns(text_block, paragraph_start_index, paragraph_end_index);
  auto runs_end = runs.end();
  for (auto current_run = runs.begin(); current_run != runs_end; ++current_run) {
    ssize_t current_start_index = current_run->start_index;
    ssize_t current_end_index = current_run->end_index;
    int bidi_visual_subindex = (current_run->bidi_direction == UBIDI_RTL ? -1 : 1);
    int font_fallback_index = 0;
reshape_part_of_run:
    auto previous_text_width = current_text_width;
    auto font_descriptor = current_run->font_descriptor.GetFallback(font_fallback_index, current_run->language);
    Shape(text_block, current_start_index, current_end_index, font_descriptor, current_run->language.opentype_tag, current_run->script, current_run->bidi_direction);

    auto glyphs_count = hb_buffer_get_length(hb_buffer_);
    auto glyph_infos = hb_buffer_get_glyph_infos(hb_buffer_, nullptr);
    auto direction = hb_buffer_get_direction(hb_buffer_);

    // font fallback handling
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
          goto reshape_part_of_run;
        }
      }
    }
    font_fallback_index = 0;

    const ssize_t width_in_font_units = PixelsToFontUnits(available_width, font_descriptor, current_run->font_size);
    const ssize_t current_x_position_in_font_units = PixelsToFontUnits(current_text_width, font_descriptor, current_run->font_size);
    auto fitting_glyphs_count = CountGlyphsThatFit(text_block, width_in_font_units - current_x_position_in_font_units, current_x_position_in_font_units == 0);

    ssize_t break_offset;
    if (fitting_glyphs_count == glyphs_count) {
      break_offset = current_end_index;
    }
    else {
      ssize_t glyph_index_after_fitting_glyphs;
      if (HB_DIRECTION_IS_FORWARD(direction)) {
        glyph_index_after_fitting_glyphs = fitting_glyphs_count;
      }
      else {
        glyph_index_after_fitting_glyphs = glyphs_count - fitting_glyphs_count - 1;
      }
      ssize_t offset_after_fitting_glyphs = glyph_infos[glyph_index_after_fitting_glyphs].cluster;
      ssize_t offset_after_not_fitting_glyph_cluster = FindTextOffsetAfterGlyphCluster(glyph_index_after_fitting_glyphs, paragraph_end_index);

      break_offset = PreviousBreak(offset_after_not_fitting_glyph_cluster, paragraph_start_index);
      broke_line = true;

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
        auto grapheme_clusters_count = CountGraphemeClusters(grapheme_cluster_iterator_, offset_after_fitting_glyphs-paragraph_start_index, offset_after_not_fitting_glyph_cluster-paragraph_start_index);
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
      Shape(text_block, current_start_index, break_offset, font_descriptor, current_run->language.opentype_tag, current_run->script, current_run->bidi_direction);
    }

    OutputShape(typeset_lines, current_text_width, font_descriptor, current_run->font_size, current_run->bidi_direction, current_run->bidi_visual_index, bidi_visual_subindex);
    if (bidi_visual_subindex < 0) {
      --bidi_visual_subindex;
    }
    else {
      ++bidi_visual_subindex;
    }

    if (break_offset < current_run->end_index) {
      if (broke_line) {
        StartNewLine();
      }
      current_start_index = break_offset;
      current_end_index = current_run->end_index;
      goto reshape_part_of_run;
    }

    if (current_run->end_of_line) {
      StartNewLine();
    }
    else {
      // save the last braking point in case we have to go back to it later
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

  // cleaning up:
  // - reorder BiDi runs
  // - empty runs are removed
  // - if 2 runs have a different script but end up with the same font, we have to merge them
  for (auto &line : typeset_lines) {
    std::sort(line.runs.begin(), line.runs.end(), [](const auto &run_a, const auto &run_b) {
      if (run_a.bidi_visual_index == run_b.bidi_visual_index) {
        return run_a.bidi_visual_subindex < run_b.bidi_visual_subindex;
      }
      else {
        return run_a.bidi_visual_index < run_b.bidi_visual_index;
      }
    });

    size_t run_index = 0;
    while (run_index < line.runs.size()) {
      if (line.runs[run_index].glyphs.size() == 0) {
        line.runs.erase(line.runs.begin()+run_index);
      }
      else if (run_index + 1 < line.runs.size()) {
        auto &current_run = line.runs[run_index];
        auto &following_run = line.runs[run_index+1];
        if (current_run.bidi_direction == following_run.bidi_direction && IsFontSizeSimilar(current_run.font_size, following_run.font_size) && current_run.font_descriptor == following_run.font_descriptor) {
          current_run.glyphs.insert(current_run.glyphs.end(), following_run.glyphs.begin(), following_run.glyphs.end());
          line.runs.erase(line.runs.begin()+(run_index+1));
        }
        else {
          ++run_index;
        }
      }
      else {
        ++run_index;
      }
    }
  }

  return typeset_lines;
}

void Typesetter::OutputShape(TypesetLines &typeset_lines, double &current_text_width, FontDescriptor font_descriptor, float font_size, UBiDiDirection bidi_direction, int bidi_visual_index, int bidi_visual_subindex) {
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

  last_run.bidi_direction = bidi_direction;
  last_run.bidi_visual_index = bidi_visual_index;
  last_run.bidi_visual_subindex = bidi_visual_subindex;

  auto ft_face = font_descriptor.GetFTFace();
  CGFloat ascent = std::round(FontUnitsToPixels(ft_face->ascender, font_descriptor, font_size));
  CGFloat descent = std::round(FontUnitsToPixels(std::abs(ft_face->descender), font_descriptor, font_size));
  CGFloat leading = std::round(FontUnitsToPixels(ft_face->height - ft_face->ascender - std::abs(ft_face->descender), font_descriptor, font_size));
  last_line.ascent = std::max(last_line.ascent, ascent);
  last_line.descent = std::max(last_line.descent, descent);
  last_line.leading = std::max(last_line.leading, leading);

  ssize_t base_x = 0;
  for (unsigned int glyph_index = 0; glyph_index < glyphs_count; ++glyph_index) {
    auto &glyph = glyphs[glyph_index];
    glyph.id = uint16_t(glyph_infos[glyph_index].codepoint);
    glyph.x_advance = FontUnitsToPixels(glyph_pos[glyph_index].x_advance, font_descriptor, font_size);
    glyph.y_advance = FontUnitsToPixels(glyph_pos[glyph_index].y_advance, font_descriptor, font_size);
    glyph.x_offset = FontUnitsToPixels(glyph_pos[glyph_index].x_offset, font_descriptor, font_size);
    glyph.y_offset = FontUnitsToPixels(glyph_pos[glyph_index].y_offset, font_descriptor, font_size);
    glyph.offset = glyph_infos[glyph_index].cluster;
    base_x += glyph_pos[glyph_index].x_advance;
  }
  const auto upem = hb_face_get_upem(hb_font_get_face(font_descriptor.GetHBFont()));
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

void Typesetter::DrawToContext(TextBlock &text_block, size_t available_width, CGContextRef context) {
  TypesetLines typeset_lines = PositionGlyphs(text_block, available_width);

  CGContextSetTextMatrix(context, CGAffineTransformIdentity);
  CGFloat total_height = 0;
  for (auto &line : typeset_lines) {
    total_height += line.height();
  }
  total_height += typeset_lines.front().descent + 0.5;
  CGContextTranslateCTM(context, 0, total_height);

  std::vector<GlyphId> glyph_ids;
  std::vector<GlyphPosition> glyph_positions;
  auto previous_line = &typeset_lines.front();
  for (auto &line : typeset_lines) {
    CGFloat x = 0;
    CGFloat y = 0;
    CGContextTranslateCTM(context, 0, -(previous_line->descent + line.ascent + line.leading));
    for (auto &run : line.runs) {
      glyph_ids.resize(0);
      glyph_positions.resize(0);
      glyph_ids.reserve(run.glyphs.size());
      glyph_positions.reserve(run.glyphs.size());
      for (auto &glyph : run.glyphs) {
        glyph_ids.push_back(glyph.id);
        glyph_positions.push_back(CGPointMake(x + glyph.x_offset, y + glyph.y_offset));
        x += glyph.x_advance;
        y += glyph.y_advance;
      }
      auto native_font = run.font_descriptor.CreateNativeFont(run.font_size);
      CTFontDrawGlyphs(native_font.get(), glyph_ids.data(), glyph_positions.data(), run.glyphs.size(), context);
    }
    previous_line = &line;
  }
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
