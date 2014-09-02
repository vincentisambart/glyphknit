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

#ifndef GLYPHKNIT_LANGUAGE_ITERATOR_H_
#define GLYPHKNIT_LANGUAGE_ITERATOR_H_

#include "script_iterator.hh"
#include "text_block.hh"

namespace glyphknit {

class LanguageIterator {
 private:
  const TextBlock &text_block_;
  ssize_t start_index_;
  ssize_t end_index_;
  ScriptIterator script_iterator_;
  ScriptIterator::Run script_run_;
  std::list<TextAttributesRun>::const_iterator attributes_run_it_;
  Language previous_language_;
  Language default_language_;
  ssize_t run_start_;

  void FindNextScriptRun();

 public:
  struct Run {
    UScriptCode script;
    Language language;
    ssize_t start, end;
  };

  LanguageIterator(const TextBlock &text_block, ssize_t start_index, ssize_t end_index) :
      text_block_{text_block}, start_index_{start_index}, end_index_{end_index}, script_iterator_{text_block.text_content(), start_index, end_index}, attributes_run_it_{text_block.attributes_runs().begin()}, run_start_{start_index} {
    while (attributes_run_it_->end < start_index_) {
      ++attributes_run_it_;
    }
    FindNextScriptRun();
  }
  LanguageIterator::Run FindNextRun();
};

}

#endif  // GLYPHKNIT_LANGUAGE_ITERATOR_H_
