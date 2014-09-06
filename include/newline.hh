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

#ifndef GLYPHKNIT_CORETEXT_NEWLINE_H_
#define GLYPHKNIT_CORETEXT_NEWLINE_H_

#include "utf.hh"

namespace glyphknit {

// The Unicode specification (section 5.8) makes the difference between paragraph and line breaking characters.
// The character concerned are CR (U+000D), LF (U+000A), NEL (U+0085), VT (U+000B), FF (U+000C), LS (U+2028), PS (U+2029).
// "CR, LF, CRLF, and NEL should be treated the same" (they are called "NLF" below)
// "In word processing, interpret any NLF the same as PS." (we'll be using the behavior for word processing apps)
// "Always interpret PS as paragraph separator and LS as line separator."
// The spec says that "FF does not interrupt a paragraph" meaning it's not a paragraph separator.
// The specification also mentions that VT is used as a line separator in Microsoft Word.

struct Range {
  ssize_t start, end;
};

inline bool IsLineSeparator(UChar32 c) {
  switch (c) {
    case 0x000B: // LINE/VERTICAL TABULATION (VT) \v
    case 0x000C: // FORM FEED (FF) \f
    case 0x2028: // LINE SEPARATOR (LS)
      return true;
    default:
      return false;
  }
}

class ParagraphIterator {
 public:
  ParagraphIterator(const uint16_t *text, ssize_t start_offset, ssize_t end_offset) :
      text_{text}, end_offset_{end_offset}, current_offset_{start_offset} {
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

  Range FindNext() {
    auto paragraph_start_offset = current_offset_;
    while (current_offset_ < end_offset_) {
      auto codepoint_start_offset = current_offset_;
      auto c = ConsumeCodepoint(text_, end_offset_, current_offset_);
      if (IsParagraphSeparator(c)) {
        if (current_offset_ < end_offset_ && c == '\r' && text_[current_offset_] == '\n') {
          ++current_offset_;
        }
        return {.start = paragraph_start_offset, .end = codepoint_start_offset};
      }
    }
    // the following is for both when where paragraph_start_offset is already at the end of the paragraph and when it's not
    return {.start = paragraph_start_offset, .end = end_offset_};
  }

 private:
  const uint16_t *text_;
  ssize_t end_offset_;
  ssize_t current_offset_;
};

}

#endif  // GLYPHKNIT_CORETEXT_NEWLINE_H_
