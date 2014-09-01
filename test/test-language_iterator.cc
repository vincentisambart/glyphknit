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

#include "language_iterator.hh"

#include "test.h"

static glyphknit::Language LANG(const char *language) {
  return glyphknit::FindLanguageCodeAndOpenTypeLanguageTag(language);
}

TEST(LanguageIterator, SimpleText) {
  using glyphknit::LanguageIterator;
  auto font = glyphknit::FontManager::CreateDescriptorFromPostScriptName("Arial");
  assert(font.is_valid());
  glyphknit::TextBlock text_block{font, 12.0};

  {
    text_block.SetText("abcd");
    text_block.SetLanguage(LANG("en"));
    auto it = LanguageIterator{text_block, 0, text_block.text_length()};
    auto run = it.FindNextRun();
    ASSERT_EQ(0, run.start);
    ASSERT_EQ(4, run.end);
    ASSERT_EQ(LANG("en"), run.language);
  }

  {
    text_block.SetText("abあcd");
    text_block.SetLanguage(LANG("en"));
    auto it = LanguageIterator{text_block, 0, text_block.text_length()};
    auto run = it.FindNextRun();
    ASSERT_EQ(0, run.start);
    ASSERT_EQ(2, run.end);
    ASSERT_EQ(LANG("en"), run.language);
    run = it.FindNextRun();
    ASSERT_EQ(2, run.start);
    ASSERT_EQ(3, run.end);
    ASSERT_EQ(LANG("ja"), run.language);
    run = it.FindNextRun();
    ASSERT_EQ(3, run.start);
    ASSERT_EQ(5, run.end);
    ASSERT_EQ(LANG("en"), run.language);
  }

  {
    text_block.SetText("ああ");
    text_block.SetLanguage(LANG("en"), 0, 1);
    text_block.SetLanguage(LANG("ja"), 1, 2);
    auto it = LanguageIterator{text_block, 0, text_block.text_length()};
    auto run = it.FindNextRun();
    ASSERT_EQ(0, run.start);
    ASSERT_EQ(2, run.end);
    ASSERT_EQ(LANG("ja"), run.language);
  }

  {
    text_block.SetText("亜亜");
    text_block.SetLanguage(LANG("ja"), 0, 1);
    text_block.SetLanguage(LANG("zh"), 1, 2);
    auto it = LanguageIterator{text_block, 0, text_block.text_length()};
    auto run = it.FindNextRun();
    ASSERT_EQ(0, run.start);
    ASSERT_EQ(1, run.end);
    ASSERT_EQ(LANG("ja"), run.language);
    run = it.FindNextRun();
    ASSERT_EQ(1, run.start);
    ASSERT_EQ(2, run.end);
    ASSERT_EQ(LANG("zh"), run.language);
  }
}
