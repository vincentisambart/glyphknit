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

#ifndef GLYPHKNIT_AT_SCOPE_EXIT_H_
#define GLYPHKNIT_AT_SCOPE_EXIT_H_

namespace glyphknit {

template <typename Callable>
class AtScopeExitHandler {
 public:
  AtScopeExitHandler(Callable &f) : f_(f), executed_(false) {}
  AtScopeExitHandler(const AtScopeExitHandler<Callable> &) = delete;
  AtScopeExitHandler<Callable> &operator=(const AtScopeExitHandler<Callable> &) = delete;
  ~AtScopeExitHandler() { execute(); }
  void disable() { executed_ = true; }
  void execute() {
    if (!executed_) {
      f_();
      executed_ = true;
    }
  }

 private:
  Callable &f_;
  bool executed_;
};

#define _AT_SCOPE_EXIT_MERGE(a, b) a ## b
#define _NAMED_AT_SCOPE_EXIT(handler_name, counter, f) \
  auto _AT_SCOPE_EXIT_MERGE(at_scope_exit_callable, counter) = (f); \
  ::glyphknit::AtScopeExitHandler<decltype(_AT_SCOPE_EXIT_MERGE(at_scope_exit_callable, counter))> handler_name{_AT_SCOPE_EXIT_MERGE(at_scope_exit_callable, counter)};
#define _UNNAMED_AT_SCOPE_EXIT(counter, f) _NAMED_AT_SCOPE_EXIT(_AT_SCOPE_EXIT_MERGE(at_scope_exit_anonymous_holder_, counter), counter, f)

#define NAMED_AT_SCOPE_EXIT(name, f) _NAMED_AT_SCOPE_EXIT(name, __COUNTER__, f)
#define AT_SCOPE_EXIT(f) _UNNAMED_AT_SCOPE_EXIT(__COUNTER__, f)

}

#endif  // GLYPHKNIT_AT_SCOPE_EXIT_H_
