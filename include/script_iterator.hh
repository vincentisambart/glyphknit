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

#ifndef GLYPHKNIT_SCRIPT_ITERATOR_H_
#define GLYPHKNIT_SCRIPT_ITERATOR_H_

#include <unicode/uscript.h>
#include <cstring>

namespace glyphknit {

class ScriptIterator {
 private:
  struct StackElement {
    UScriptCode script;
    UChar32 codepoint;
  };

  const uint16_t *text_;
  ssize_t length_;
  ssize_t current_index_;
  ssize_t run_start_;
  UScriptCode last_script_;
  static int const kStackSize = 128;
  ScriptIterator::StackElement pair_starts_[kStackSize];
  int stack_length_;

  UScriptCode FindNextFixedScript() const;
  template <typename Callable>
  void IfIsPairEnd(UChar32 codepoint, Callable to_call);

 public:
  struct Run {
    UScriptCode script;
    ssize_t start, end;
  };

  ScriptIterator(const uint16_t *text, ssize_t length) :
      text_(text), length_(length), current_index_(0), run_start_(0), stack_length_(0) {
    // knowing the first fixed script from the start makes things much easier
    last_script_ = FindNextFixedScript();
  }
  ScriptIterator::Run FindNextRun();
};

}

#endif  // GLYPHKNIT_SCRIPT_ITERATOR_H_
