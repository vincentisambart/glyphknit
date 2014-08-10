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

#ifndef GLYPHKNIT_TYPESETTER_H_
#define GLYPHKNIT_TYPESETTER_H_

#include <vector>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-sign-overflow"
#include <hb-ft.h>
#pragma clang diagnostic pop

#include "text_block.hh"

namespace glyphknit {

typedef uint16_t GlyphId;
typedef CGPoint GlyphPosition;
typedef ssize_t TextOffset;
struct TypesetRun {
  struct Glyph {
    GlyphId id;
    GlyphPosition position;
    TextOffset offset;
  };

  // TODO: SizedFont sized_font;
  std::vector<Glyph> glyphs;
  // TODO: direction
};
struct TypesetLine {
  std::vector<TypesetRun> runs;
  CGFloat ascent;
  CGFloat descent;
  CGFloat leading;
};
typedef std::vector<TypesetLine> TypesetLines;

struct TypesettingState;

class Typesetter {
 public:
  Typesetter();
  ~Typesetter();
  void PositionGlyphs(TextBlock &, size_t width, TypesetLines &);
  void DrawToContext(TextBlock &, size_t width, CGContextRef);

 private:
  UBreakIterator *line_break_iterator_;
  UBreakIterator *grapheme_cluster_iterator_;

  void Shape(TypesettingState &, ssize_t start_offset, ssize_t end_offset);
  ssize_t CountGlyphsThatFit(TypesettingState &, ssize_t width);
  ssize_t FindTextOffsetAfterGlyphCluster(const TypesettingState &, ssize_t glyph_index);
  void TypesetLine(TypesettingState &, ssize_t line_start_offset, ssize_t line_end_offset);
  void TypesetParagraph(TypesettingState &);
  void OutputShape(TypesettingState &);
  void StartNewLine(TypesettingState &);
};

};

#endif  // GLYPHKNIT_TYPESETTER_H_
