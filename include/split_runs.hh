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

#ifndef GLYPHKNIT_RUN_SPLIT_H_
#define GLYPHKNIT_RUN_SPLIT_H_

#include "language.hh"
#include "text_block.hh"

namespace glyphknit {

struct TextRun {
  ssize_t start_index;
  ssize_t end_index;

  UScriptCode script;
  Language language;
  bool end_of_line;
};

typedef std::list<TextRun> ParagraphRuns;

void SplitRunsByLanguage(ParagraphRuns &runs, const TextBlock &text_block, ssize_t paragraph_start_index, ssize_t paragraph_end_index);
void SplitRunsInLines(ParagraphRuns &runs, const TextBlock &text_block, ssize_t paragraph_start_index, ssize_t paragraph_end_index);
ParagraphRuns CreateBaseParagraphRuns(ssize_t paragraph_start_index, ssize_t paragraph_end_index);
ParagraphRuns SplitRuns(const TextBlock &text_block, ssize_t paragraph_start_index, ssize_t paragraph_end_index);

}

#endif  // GLYPHKNIT_RUN_SPLIT_H_
