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

#include <unicode/ubidi.h>
#include <sys/types.h>
#include <cassert>

namespace glyphknit {

class BidiIterator {
 private:
  const uint16_t *paragraph_text_;
  ssize_t start_offset_;
  ssize_t end_offset_;
  ssize_t current_offset_;
  UBiDi *bidi_;

 public:
  struct Run {
    UBiDiLevel level;
    ssize_t start, end;
    bool ltr() { return (level & 1) == 0; }
  };

  BidiIterator(const uint16_t *paragraph_text, ssize_t start_offset, ssize_t end_offset) :
      paragraph_text_{paragraph_text}, start_offset_{start_offset}, end_offset_{end_offset}, current_offset_{start_offset} {

    auto length = end_offset_ - start_offset_;

    UErrorCode error_code = U_ZERO_ERROR;
    bidi_ = ubidi_openSized(int32_t(length), 0, &error_code);
    assert(bidi_ != nullptr && U_SUCCESS(error_code));

    ubidi_setPara(bidi_, paragraph_text_+start_offset_, int32_t(length), UBIDI_DEFAULT_LTR, nullptr, &error_code);
    assert(U_SUCCESS(error_code));
  }
  BidiIterator::Run FindNextRun();
};

BidiIterator::Run BidiIterator::FindNextRun() {
  if (current_offset_ >= end_offset_) {
    return {
      .level = 0,
      .start = end_offset_,
      .end = end_offset_,
    };
  }
  
  UBiDiLevel level;
  int32_t end_offset_in_paragraph;
  ubidi_getLogicalRun(bidi_, int32_t(current_offset_ - start_offset_), &end_offset_in_paragraph, &level);
  ssize_t end_offset_of_run = end_offset_in_paragraph + start_offset_;
  BidiIterator::Run run = {
    .level = level,
    .start = current_offset_,
    .end = end_offset_of_run,
  };
  current_offset_ = end_offset_of_run;
  
  return run;
}

}
