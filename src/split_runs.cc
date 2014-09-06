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
#include "script_iterator.hh"
#include "utf.hh"
#include "newline.hh"

namespace glyphknit {

namespace {

class RunSplitter {
 public:
  TextRun &previous_run() {
    assert(current_run_ != runs_.begin());
    auto previous_run = current_run_;
    --previous_run;
    return *previous_run;
  }

  RunSplitter(ParagraphRuns &runs) : runs_(runs), current_run_{runs.begin()} {
  }

  template <typename Callback>
  void RunGoesTo(ssize_t index, Callback c) {
    assert(current_run_ != runs_.end());
    while (current_run_->end_index < index) {
      c(*current_run_);
      ++current_run_;
    }
    if (current_run_->end_index == index) {
      c(*current_run_);
      ++current_run_;
    }
    else {
      auto new_run = runs_.insert(current_run_, *current_run_);
      new_run->end_index = index;
      c(*new_run);
      current_run_->start_index = index;
    }
  }

  void RunGoesTo(ssize_t index) {
    RunGoesTo(index, [](auto&){});
  }

  void ThrowAwayUpTo(ssize_t index) {
    assert(current_run_ != runs_.end());
    while (current_run_->end_index < index) {
      ++current_run_;
    }
    if (current_run_->end_index == index) {
      runs_.erase(current_run_++);
    }
    else {
      current_run_->start_index = index;
    }
  }

 private:
  ParagraphRuns &runs_;
  ParagraphRuns::iterator current_run_;
};

}

void SplitRunsByLanguage(ParagraphRuns &runs, const TextBlock &text_block, ssize_t paragraph_start_index, ssize_t paragraph_end_index) {
  ScriptIterator script_iterator{text_block.text_content(), paragraph_start_index, paragraph_end_index};
  auto current_attributes_run = text_block.attributes_runs().begin();
  while (current_attributes_run->end < paragraph_start_index) {
    ++current_attributes_run;
  }

  auto run_start = paragraph_start_index;
  RunSplitter splitter{runs};

  auto script_run = script_iterator.FindNextRun();
  auto default_language = GuessLanguageFromScript(script_run.script);
  auto previous_language = default_language;
  auto attributes_run_end = text_block.attributes_runs().end();

  while (script_run.start < paragraph_end_index) {
    for (; current_attributes_run != attributes_run_end && current_attributes_run->end <= script_run.end; ++current_attributes_run) {
      auto language = current_attributes_run->attributes.language;
      if (!IsScriptUsedForLanguage(script_run.script, language)) {
        language = default_language;
      }
      if (language != previous_language) {
        auto run_end = std::max(current_attributes_run->start, script_run.start);
        if (run_start < run_end) {
          splitter.RunGoesTo(run_end, [&](auto &run) {
            run.script = script_run.script;
            run.language = previous_language;
          });
          run_start = run_end;
        }
        previous_language = language;
      }
    }

    auto run_end = script_run.end;
    if (run_start < run_end) {
      Language language;
      if (current_attributes_run == attributes_run_end) {
        language = previous_language;
      }
      else {
        language = current_attributes_run->attributes.language;
        if (!IsScriptUsedForLanguage(script_run.script, language)) {
          language = default_language;
        }
      }
      if (!IsScriptUsedForLanguage(script_run.script, language)) {
        language = default_language;
      }
      splitter.RunGoesTo(run_end, [&](auto &run) {
        run.script = script_run.script;
        run.language = language;
      });
      run_start = run_end;
    }

    script_run = script_iterator.FindNextRun();
    default_language = GuessLanguageFromScript(script_run.script);
  }
}

void SplitRunsInLines(ParagraphRuns &runs, const TextBlock &text_block, ssize_t paragraph_start_index, ssize_t paragraph_end_index) {
  RunSplitter splitter{runs};
  const uint16_t *text = text_block.text_content();
  auto current_index = paragraph_start_index;
  while (current_index < paragraph_end_index) {
    auto codepoint_start_index = current_index;
    auto c = ConsumeCodepoint(text, paragraph_end_index, current_index);
    if (IsLineSeparator(c)) {
      splitter.RunGoesTo(codepoint_start_index);
      splitter.previous_run().end_of_line = true;
      splitter.ThrowAwayUpTo(current_index);
    }
  }
}

ParagraphRuns CreateBaseParagraphRuns(ssize_t paragraph_start_index, ssize_t paragraph_end_index) {
  ParagraphRuns runs;

  TextRun base_paragraph_run = {
    .start_index = paragraph_start_index,
    .end_index = paragraph_end_index,
    .end_of_line = false,
  };
  runs.push_back(base_paragraph_run);

  return runs;
}

ParagraphRuns SplitRuns(const TextBlock &text_block, ssize_t paragraph_start_index, ssize_t paragraph_end_index) {
  auto runs = CreateBaseParagraphRuns(paragraph_start_index, paragraph_end_index);
  if (paragraph_start_index == paragraph_end_index) {
    return runs;
  }

  SplitRunsByLanguage(runs, text_block, paragraph_start_index, paragraph_end_index);

  // splitting in lines must be last to be sure runs with end_of_line set to true are not split or thrown away
  SplitRunsInLines(runs, text_block, paragraph_start_index, paragraph_end_index);

  return runs;
}

}
