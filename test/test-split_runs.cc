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

#include "split_runs.hh"

#include "test.h"

static glyphknit::Language LANG(const char *language) {
  return glyphknit::FindLanguageCodeAndOpenTypeLanguageTag(language);
}

static glyphknit::ParagraphRuns GenerateRunsByLanguage(const glyphknit::TextBlock &text_block, ssize_t paragraph_start_index, ssize_t paragraph_end_index) {
  auto runs = glyphknit::CreateBaseParagraphRuns(paragraph_start_index, paragraph_end_index);
  glyphknit::SplitRunsByLanguage(runs, text_block, paragraph_start_index, paragraph_end_index);
  return runs;
}

TEST(SplitRunsByLanguage, SimpleText) {
  auto font = glyphknit::FontManager::CreateDescriptorFromPostScriptName("Arial");
  assert(font.is_valid());
  glyphknit::TextBlock text_block{font, 12.0};

  {
    text_block.SetText("abcd");
    text_block.SetLanguage(LANG("en"));
    auto runs = GenerateRunsByLanguage(text_block, 0, text_block.text_length());
    auto run = runs.begin();
    ASSERT_EQ(0, run->start_index);
    ASSERT_EQ(4, run->end_index);
    ASSERT_EQ(LANG("en"), run->language);
    ASSERT_EQ(USCRIPT_LATIN, run->script);
  }

  {
    text_block.SetText("abあcd");
    text_block.SetLanguage(LANG("en"));
    auto runs = GenerateRunsByLanguage(text_block, 0, text_block.text_length());
    auto run = runs.begin();
    ASSERT_EQ(0, run->start_index);
    ASSERT_EQ(2, run->end_index);
    ASSERT_EQ(LANG("en"), run->language);
    ASSERT_EQ(USCRIPT_LATIN, run->script);
    ++run;
    ASSERT_EQ(2, run->start_index);
    ASSERT_EQ(3, run->end_index);
    ASSERT_EQ(LANG("ja"), run->language);
    ASSERT_EQ(USCRIPT_KATAKANA, run->script);
    ++run;
    ASSERT_EQ(3, run->start_index);
    ASSERT_EQ(5, run->end_index);
    ASSERT_EQ(LANG("en"), run->language);
    ASSERT_EQ(USCRIPT_LATIN, run->script);
  }

  {
    text_block.SetText("ああ");
    text_block.SetLanguage(LANG("en"), 0, 1);
    text_block.SetLanguage(LANG("ja"), 1, 2);
    auto runs = GenerateRunsByLanguage(text_block, 0, text_block.text_length());
    auto run = runs.begin();
    ASSERT_EQ(0, run->start_index);
    ASSERT_EQ(2, run->end_index);
    ASSERT_EQ(LANG("ja"), run->language);
    ASSERT_EQ(USCRIPT_KATAKANA, run->script);
  }

  {
    text_block.SetText("ああああ");
    text_block.SetLanguage(LANG("en"), 0, 2);
    text_block.SetLanguage(LANG("ja"), 2, 4);
    auto runs = GenerateRunsByLanguage(text_block, 1, text_block.text_length()-1);
    auto run = runs.begin();
    ASSERT_EQ(1, run->start_index);
    ASSERT_EQ(3, run->end_index);
    ASSERT_EQ(LANG("ja"), run->language);
    ASSERT_EQ(USCRIPT_KATAKANA, run->script);
  }

  {
    text_block.SetText("亜亜");
    text_block.SetLanguage(LANG("ja"), 0, 1);
    text_block.SetLanguage(LANG("zh"), 1, 2);
    auto runs = GenerateRunsByLanguage(text_block, 0, text_block.text_length());
    auto run = runs.begin();
    ASSERT_EQ(0, run->start_index);
    ASSERT_EQ(1, run->end_index);
    ASSERT_EQ(LANG("ja"), run->language);
    ASSERT_EQ(USCRIPT_HAN, run->script);
    ++run;
    ASSERT_EQ(1, run->start_index);
    ASSERT_EQ(2, run->end_index);
    ASSERT_EQ(LANG("zh"), run->language);
    ASSERT_EQ(USCRIPT_HAN, run->script);
  }

  {
    text_block.SetText("abあアあ123あ亜亜亜亜あcdef");
    text_block.SetLanguage(LANG("ja"));
    auto runs = GenerateRunsByLanguage(text_block, 1, text_block.text_length()-2);
    auto run = runs.begin();
    ASSERT_EQ(1, run->start_index);
    ASSERT_EQ(2, run->end_index);
    ASSERT_NE(LANG("ja"), run->language);
    ASSERT_EQ(USCRIPT_LATIN, run->script);
    ++run;
    ASSERT_EQ(2, run->start_index);
    ASSERT_EQ(9, run->end_index);
    ASSERT_EQ(LANG("ja"), run->language);
    ASSERT_EQ(USCRIPT_KATAKANA, run->script);
    ++run;
    ASSERT_EQ(9, run->start_index);
    ASSERT_EQ(13, run->end_index);
    ASSERT_EQ(LANG("ja"), run->language);
    ASSERT_EQ(USCRIPT_HAN, run->script);
    ++run;
    ASSERT_EQ(13, run->start_index);
    ASSERT_EQ(14, run->end_index);
    ASSERT_EQ(LANG("ja"), run->language);
    ASSERT_EQ(USCRIPT_KATAKANA, run->script);
    ++run;
    ASSERT_EQ(14, run->start_index);
    ASSERT_EQ(16, run->end_index);
    ASSERT_NE(LANG("ja"), run->language);
    ASSERT_EQ(USCRIPT_LATIN, run->script);
  }
}
