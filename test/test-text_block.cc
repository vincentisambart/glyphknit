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

  auto font_descriptor = glyphknit::FontManager::CreateDescriptorFromPostScriptName("SourceSansPro-Regular");

  glyphknit::TextBlock text_block{font_descriptor, 12.0f};
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

  auto font_descriptor = glyphknit::FontManager::CreateDescriptorFromPostScriptName("SourceSansPro-Regular");

  glyphknit::TextBlock text_block{font_descriptor, 12.0f};
  text_block.SetText("abcdefghijklmnopqrstuvwxyz");
  {
    ASSERT_EQ(1u, text_block.attributes_runs().size());

    auto current_run = text_block.attributes_runs().begin();
    ASSERT_EQ(0, current_run->start);
    ASSERT_EQ(26, current_run->end);
    ASSERT_NEAR(12.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);
  }

  // changing font size in the middle of an already existing run
  text_block.SetFontSize(14.0f, 1, 6);
  {
    ASSERT_EQ(3u, text_block.attributes_runs().size());

    auto current_run = text_block.attributes_runs().begin();
    ASSERT_EQ(0, current_run->start);
    ASSERT_EQ(1, current_run->end);
    ASSERT_NEAR(12.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);

    ++current_run;
    ASSERT_EQ(1, current_run->start);
    ASSERT_EQ(6, current_run->end);
    ASSERT_NEAR(14.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);

    ++current_run;
    ASSERT_EQ(6, current_run->start);
    ASSERT_EQ(26, current_run->end);
    ASSERT_NEAR(12.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);
  }

  // change font size over the separation between 2 existing runs
  text_block.SetFontSize(16.0f, 4, 10);
  {
    ASSERT_EQ(4u, text_block.attributes_runs().size());

    auto current_run = text_block.attributes_runs().begin();
    ASSERT_EQ(0, current_run->start);
    ASSERT_EQ(1, current_run->end);
    ASSERT_NEAR(12.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);

    ++current_run;
    ASSERT_EQ(1, current_run->start);
    ASSERT_EQ(4, current_run->end);
    ASSERT_NEAR(14.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);

    ++current_run;
    ASSERT_EQ(4, current_run->start);
    ASSERT_EQ(10, current_run->end);
    ASSERT_NEAR(16.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);

    ++current_run;
    ASSERT_EQ(10, current_run->start);
    ASSERT_EQ(26, current_run->end);
    ASSERT_NEAR(12.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);
  }

  // change font size to the end
  text_block.SetFontSize(18.0f, 9, 26);
  {
    ASSERT_EQ(4u, text_block.attributes_runs().size());

    auto current_run = text_block.attributes_runs().begin();
    ASSERT_EQ(0, current_run->start);
    ASSERT_EQ(1, current_run->end);
    ASSERT_NEAR(12.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);

    ++current_run;
    ASSERT_EQ(1, current_run->start);
    ASSERT_EQ(4, current_run->end);
    ASSERT_NEAR(14.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);

    ++current_run;
    ASSERT_EQ(4, current_run->start);
    ASSERT_EQ(9, current_run->end);
    ASSERT_NEAR(16.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);

    ++current_run;
    ASSERT_EQ(9, current_run->start);
    ASSERT_EQ(26, current_run->end);
    ASSERT_NEAR(18.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);
  }

  // change font size of the full block
  text_block.SetFontSize(20.0f);
  {
    ASSERT_EQ(1u, text_block.attributes_runs().size());

    auto current_run = text_block.attributes_runs().begin();
    ASSERT_EQ(0, current_run->start);
    ASSERT_EQ(26, current_run->end);
    ASSERT_NEAR(20.0f, current_run->attributes.font_size, glyphknit::kFontComparisonDelta);
    ASSERT_EQ(font_descriptor, current_run->attributes.font_descriptor);
  }
}

// TODO: add tests for SetFontFace and SetLanguage
