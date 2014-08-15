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

#include "text_block.hh"

#include <unicode/ustring.h>
#include <cassert>

namespace glyphknit {

void TextBlock::SetText(const uint16_t *utf16_text, const size_t utf16_length) {
  text_.resize(utf16_length);
  std::copy(utf16_text, utf16_text+utf16_length, text_.begin());

  attributes_runs_.resize(1);
  attributes_runs_.front().end = utf16_length;
}

const uint32_t kReplacementCharacter = 0xfffd;
void TextBlock::SetText(const char *utf8_text, const size_t utf8_length) {
  text_.resize(utf8_length);  // a UTF-16 string is always smaller than its UTF-8 equivalent (in code units of course, not necessarily in bytes)
  int32_t utf16_length;
  UErrorCode errorCode = U_ZERO_ERROR;
  u_strFromUTF8WithSub(text_.data(), int32_t(text_.capacity()), &utf16_length, utf8_text, int32_t(utf8_length), kReplacementCharacter, NULL, &errorCode);
  text_.resize(utf16_length);

  attributes_runs_.resize(1);
  attributes_runs_.front().end = utf16_length;
}

void TextBlock::SetText(const char *utf8_text) {
  SetText(utf8_text, std::strlen(utf8_text));
}

TextBlock::~TextBlock() {
}

void TextBlock::SetFontSize(float font_size, ssize_t start, ssize_t end) {
  // TODO: Write setters for the other attributes, and tests
  assert(start >= 0);
  if (end < 0 || end > ssize_t(text_.size())) {
    end = text_.size();
  }
  if (start >= end) {
    return;
  }

  // std::list iterators are not invalidated if the current element is not deleted so we can modify the elements without too much of a problem
  auto attributes_runs_end = attributes_runs_.end();
  decltype(attributes_runs_)::iterator previous_run;
  for (auto current_run = attributes_runs_.begin(); current_run != attributes_runs_end; ++current_run) {
    if (start >= current_run->end) {
      previous_run = current_run;
      continue;
    }

    if (current_run->start > 0 && current_run->attributes == previous_run->attributes) {
      // merge runs that became identical after a change
      current_run->start = previous_run->start;
      attributes_runs_.erase(previous_run, current_run);
    }
    previous_run = current_run;

    if (end <= current_run->start) {
      break;
    }
    if (IsFontSizeSimilar(current_run->attributes.font_size, font_size)) {
      continue;
    }

    if (start <= current_run->start && end >= current_run->end) {
      current_run->attributes.font_size = font_size;
    }
    else if (start <= current_run->start) {
      auto new_run_before = attributes_runs_.emplace(current_run);
      new_run_before->start = current_run->start;
      new_run_before->end = end;
      new_run_before->attributes = current_run->attributes;
      new_run_before->attributes.font_size = font_size;

      current_run->start = end;
    }
    else if (end >= current_run->end) {
      auto new_run_before = attributes_runs_.emplace(current_run);
      new_run_before->start = current_run->start;
      new_run_before->end = start;
      new_run_before->attributes = current_run->attributes;

      current_run->start = start;
      current_run->attributes.font_size = font_size;
    }
    else {
      auto new_run_before = attributes_runs_.emplace(current_run);
      new_run_before->start = current_run->start;
      new_run_before->end = start;
      new_run_before->attributes = current_run->attributes;

      new_run_before = attributes_runs_.emplace(current_run);
      new_run_before->start = start;
      new_run_before->end = end;
      new_run_before->attributes = current_run->attributes;
      new_run_before->attributes.font_size = font_size;

      current_run->start = end;
    }
  }
}

}
