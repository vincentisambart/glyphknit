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

#include "test.h"

TEST(TextBlock, SetText) {
  using glyphknit::TextBlock;

  auto font_face = glyphknit::FontManager::LoadFontFromPostScriptName("SourceSansPro-Regular");

  glyphknit::TextBlock text_block{font_face, 12.0f};
  {
    ASSERT_EQ(0, text_block.text_length());
    ASSERT_EQ(1u, text_block.attributes_runs().size());

    const auto &first_run = text_block.attributes_runs().front();
    ASSERT_EQ(0, first_run.start);
    ASSERT_EQ(0, first_run.end);
    const auto &first_attributes = first_run.attributes;
    ASSERT_NEAR(12.0f, first_attributes.font_size, glyphknit::kFontComparisonDelta);
  }

  text_block.SetText("abcdefghijklmnopqrstuvwxyz");
  {
    ASSERT_EQ(26, text_block.text_length());
    ASSERT_EQ(1u, text_block.attributes_runs().size());

    const auto &first_run = text_block.attributes_runs().front();
    ASSERT_EQ(0, first_run.start);
    ASSERT_EQ(26, first_run.end);
    const auto &first_attributes = first_run.attributes;
    ASSERT_NEAR(12.0f, first_attributes.font_size, glyphknit::kFontComparisonDelta);
  }

  text_block.SetText("1234");
  {
    ASSERT_EQ(4, text_block.text_length());
    ASSERT_EQ(1u, text_block.attributes_runs().size());

    const auto &first_run = text_block.attributes_runs().front();
    ASSERT_EQ(0, first_run.start);
    ASSERT_EQ(4, first_run.end);
    const auto &first_attributes = first_run.attributes;
    ASSERT_NEAR(12.0f, first_attributes.font_size, glyphknit::kFontComparisonDelta);
  }
}

TEST(TextBlock, SetFontSize) {
  using glyphknit::TextBlock;

  auto font_face = glyphknit::FontManager::LoadFontFromPostScriptName("SourceSansPro-Regular");

  glyphknit::TextBlock text_block{font_face, 12.0f};
  text_block.SetText("abcdefghijklmnopqrstuvwxyz");
  {
    ASSERT_EQ(1u, text_block.attributes_runs().size());

    const auto &first_run = text_block.attributes_runs().front();
    ASSERT_EQ(0, first_run.start);
    ASSERT_EQ(26, first_run.end);
    ASSERT_NEAR(12.0f, first_run.attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(*font_face, *first_run.attributes.font_face);
  }

  text_block.SetFontSize(14.0f, 1, 4);
  {
    ASSERT_EQ(3u, text_block.attributes_runs().size());

    auto current_run = text_block.attributes_runs().begin();
    ASSERT_EQ(0, current_run->start);
    ASSERT_EQ(1, current_run->end);
    ASSERT_NEAR(12, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(*font_face, *current_run->attributes.font_face);

    ++current_run;
    ASSERT_EQ(1, current_run->start);
    ASSERT_EQ(4, current_run->end);
    ASSERT_NEAR(14.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(*font_face, *current_run->attributes.font_face);

    ++current_run;
    ASSERT_EQ(4, current_run->start);
    ASSERT_EQ(26, current_run->end);
    ASSERT_NEAR(12.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(*font_face, *current_run->attributes.font_face);
  }
}
