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

#include "script_iterator.hh"

#include "test.h"

#include <unicode/ustring.h>

const int kBufferSize = 2000;

#define SCRIPT_ITERATOR_TEST_DECLARATIONS \
  uint16_t text[kBufferSize]; \
  int32_t length; \
  UErrorCode status = U_ZERO_ERROR; \
  glyphknit::ScriptIterator::Run run; \
  glyphknit::ScriptIterator it = {nullptr, 0};


TEST(ScriptIterator, SimpleText) {
  SCRIPT_ITERATOR_TEST_DECLARATIONS;

  u_strFromUTF8(text, kBufferSize, &length, "abcde", -1, &status);
  assert(U_SUCCESS(status));

  it = glyphknit::ScriptIterator{text, length};
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(5, run.end);


  u_strFromUTF8(text, kBufferSize, &length, "abアイウエオcde風", -1, &status);
  it = glyphknit::ScriptIterator{text, length};

  assert(U_SUCCESS(status));
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(2, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(2, run.start);
  ASSERT_EQ(7, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(7, run.start);
  ASSERT_EQ(10, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_HAN, run.script);
  ASSERT_EQ(10, run.start);
  ASSERT_EQ(11, run.end);


  u_strFromUTF8(text, kBufferSize, &length, "bあアあ123あ亜亜亜亜あcd", -1, &status);
  assert(U_SUCCESS(status));
  it = glyphknit::ScriptIterator{text, length};
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(1, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(1, run.start);
  ASSERT_EQ(8, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_HAN, run.script);
  ASSERT_EQ(8, run.start);
  ASSERT_EQ(12, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(12, run.start);
  ASSERT_EQ(13, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(13, run.start);
  ASSERT_EQ(15, run.end);
}

TEST(ScriptIterator, SimpleCommonScriptPairing) {
  SCRIPT_ITERATOR_TEST_DECLARATIONS;

  u_strFromUTF8(text, kBufferSize, &length, "ア(イウ)エオ", -1, &status);
  assert(U_SUCCESS(status));

  it = glyphknit::ScriptIterator{text, length};
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(7, run.end);


  u_strFromUTF8(text, kBufferSize, &length, "ab(アイウ)エオ", -1, &status);
  assert(U_SUCCESS(status));

  it = glyphknit::ScriptIterator{text, length};
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(3, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(3, run.start);
  ASSERT_EQ(6, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(6, run.start);
  ASSERT_EQ(7, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(7, run.start);
  ASSERT_EQ(9, run.end);
}

TEST(ScriptIterator, ScriptExtensions) {
  SCRIPT_ITERATOR_TEST_DECLARATIONS;

  u_strFromUTF8(text, kBufferSize, &length, "ab「c」de", -1, &status);
  it = glyphknit::ScriptIterator{text, length};
  assert(U_SUCCESS(status));

  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(7, run.end);


  u_strFromUTF8(text, kBufferSize, &length, "ア「ab」イウ", -1, &status);
  it = glyphknit::ScriptIterator{text, length};
  assert(U_SUCCESS(status));

  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(2, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(2, run.start);
  ASSERT_EQ(4, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(4, run.start);
  ASSERT_EQ(7, run.end);


  // "「" and "」" have Katakana in their script extensions making them "prefer" Katakana to Latin
  u_strFromUTF8(text, kBufferSize, &length, "ab「ア」cd", -1, &status);
  it = glyphknit::ScriptIterator{text, length};
  assert(U_SUCCESS(status));

  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(2, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(2, run.start);
  ASSERT_EQ(5, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(5, run.start);
  ASSERT_EQ(7, run.end);
}

TEST(ScriptIterator, KatakanaAndHiraganaHandledTheSameWay) {
  SCRIPT_ITERATOR_TEST_DECLARATIONS;

  u_strFromUTF8(text, kBufferSize, &length, "あイうエお", -1, &status);
  assert(U_SUCCESS(status));

  it = glyphknit::ScriptIterator{text, length};
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);  // note that technically it's a lie, it's a mix of Hiragana and Katakana, but that fits our purpose well
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(5, run.end);
}

TEST(ScriptIterator, NotClosedPairs) {
  SCRIPT_ITERATOR_TEST_DECLARATIONS;

  u_strFromUTF8(text, kBufferSize, &length, "abcd(アイ[ウ>エ)オ", -1, &status);
  assert(U_SUCCESS(status));

  it = glyphknit::ScriptIterator{text, length};
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(5, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(5, run.start);
  ASSERT_EQ(11, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(11, run.start);
  ASSERT_EQ(12, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(12, run.start);
  ASSERT_EQ(13, run.end);
}

TEST(ScriptIterator, PairsWorkingBothWays) {
  SCRIPT_ITERATOR_TEST_DECLARATIONS;

  u_strFromUTF8(text, kBufferSize, &length, "abcd«アイ»オ", -1, &status);
  assert(U_SUCCESS(status));

  it = glyphknit::ScriptIterator{text, length};
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(5, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(5, run.start);
  ASSERT_EQ(7, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(7, run.start);
  ASSERT_EQ(8, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(8, run.start);
  ASSERT_EQ(9, run.end);


  u_strFromUTF8(text, kBufferSize, &length, "abcd»アイ«オ", -1, &status);
  assert(U_SUCCESS(status));

  it = glyphknit::ScriptIterator{text, length};
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(5, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(5, run.start);
  ASSERT_EQ(7, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(7, run.start);
  ASSERT_EQ(8, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(8, run.start);
  ASSERT_EQ(9, run.end);


  // this last test is just as a reference
  u_strFromUTF8(text, kBufferSize, &length, "abcd»アイ»オ", -1, &status);
  assert(U_SUCCESS(status));

  it = glyphknit::ScriptIterator{text, length};
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_LATIN, run.script);
  ASSERT_EQ(0, run.start);
  ASSERT_EQ(5, run.end);
  run = it.FindNextRun();
  ASSERT_EQ(USCRIPT_KATAKANA, run.script);
  ASSERT_EQ(5, run.start);
  ASSERT_EQ(9, run.end);
}
