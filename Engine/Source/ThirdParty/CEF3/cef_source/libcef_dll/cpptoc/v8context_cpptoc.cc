// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#include "libcef_dll/cpptoc/browser_cpptoc.h"
#include "libcef_dll/cpptoc/frame_cpptoc.h"
#include "libcef_dll/cpptoc/task_runner_cpptoc.h"
#include "libcef_dll/cpptoc/v8context_cpptoc.h"
#include "libcef_dll/cpptoc/v8exception_cpptoc.h"
#include "libcef_dll/cpptoc/v8value_cpptoc.h"


// GLOBAL FUNCTIONS - Body may be edited by hand.

CEF_EXPORT cef_v8context_t* cef_v8context_get_current_context() {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  CefRefPtr<CefV8Context> _retval = CefV8Context::GetCurrentContext();

  // Return type: refptr_same
  return CefV8ContextCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8context_t* cef_v8context_get_entered_context() {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  CefRefPtr<CefV8Context> _retval = CefV8Context::GetEnteredContext();

  // Return type: refptr_same
  return CefV8ContextCppToC::Wrap(_retval);
}

CEF_EXPORT int cef_v8context_in_context() {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  bool _retval = CefV8Context::InContext();

  // Return type: bool
  return _retval;
}


// MEMBER FUNCTIONS - Body may be edited by hand.

cef_task_runner_t* CEF_CALLBACK v8context_get_task_runner(
    struct _cef_v8context_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefRefPtr<CefTaskRunner> _retval = CefV8ContextCppToC::Get(
      self)->GetTaskRunner();

  // Return type: refptr_same
  return CefTaskRunnerCppToC::Wrap(_retval);
}

int CEF_CALLBACK v8context_is_valid(struct _cef_v8context_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ContextCppToC::Get(self)->IsValid();

  // Return type: bool
  return _retval;
}

cef_browser_t* CEF_CALLBACK v8context_get_browser(
    struct _cef_v8context_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefRefPtr<CefBrowser> _retval = CefV8ContextCppToC::Get(self)->GetBrowser();

  // Return type: refptr_same
  return CefBrowserCppToC::Wrap(_retval);
}

cef_frame_t* CEF_CALLBACK v8context_get_frame(struct _cef_v8context_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefRefPtr<CefFrame> _retval = CefV8ContextCppToC::Get(self)->GetFrame();

  // Return type: refptr_same
  return CefFrameCppToC::Wrap(_retval);
}

struct _cef_v8value_t* CEF_CALLBACK v8context_get_global(
    struct _cef_v8context_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8ContextCppToC::Get(self)->GetGlobal();

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

int CEF_CALLBACK v8context_enter(struct _cef_v8context_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ContextCppToC::Get(self)->Enter();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8context_exit(struct _cef_v8context_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ContextCppToC::Get(self)->Exit();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8context_is_same(struct _cef_v8context_t* self,
    struct _cef_v8context_t* that) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: that; type: refptr_same
  DCHECK(that);
  if (!that)
    return 0;

  // Execute
  bool _retval = CefV8ContextCppToC::Get(self)->IsSame(
      CefV8ContextCppToC::Unwrap(that));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8context_eval(struct _cef_v8context_t* self,
    const cef_string_t* code, struct _cef_v8value_t** retval,
    struct _cef_v8exception_t** exception) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: code; type: string_byref_const
  DCHECK(code);
  if (!code)
    return 0;
  // Verify param: retval; type: refptr_same_byref
  DCHECK(retval);
  if (!retval)
    return 0;
  // Verify param: exception; type: refptr_same_byref
  DCHECK(exception);
  if (!exception)
    return 0;

  // Translate param: retval; type: refptr_same_byref
  CefRefPtr<CefV8Value> retvalPtr;
  if (retval && *retval)
    retvalPtr = CefV8ValueCppToC::Unwrap(*retval);
  CefV8Value* retvalOrig = retvalPtr.get();
  // Translate param: exception; type: refptr_same_byref
  CefRefPtr<CefV8Exception> exceptionPtr;
  if (exception && *exception)
    exceptionPtr = CefV8ExceptionCppToC::Unwrap(*exception);
  CefV8Exception* exceptionOrig = exceptionPtr.get();

  // Execute
  bool _retval = CefV8ContextCppToC::Get(self)->Eval(
      CefString(code),
      retvalPtr,
      exceptionPtr);

  // Restore param: retval; type: refptr_same_byref
  if (retval) {
    if (retvalPtr.get()) {
      if (retvalPtr.get() != retvalOrig) {
        *retval = CefV8ValueCppToC::Wrap(retvalPtr);
      }
    } else {
      *retval = NULL;
    }
  }
  // Restore param: exception; type: refptr_same_byref
  if (exception) {
    if (exceptionPtr.get()) {
      if (exceptionPtr.get() != exceptionOrig) {
        *exception = CefV8ExceptionCppToC::Wrap(exceptionPtr);
      }
    } else {
      *exception = NULL;
    }
  }

  // Return type: bool
  return _retval;
}


// CONSTRUCTOR - Do not edit by hand.

CefV8ContextCppToC::CefV8ContextCppToC(CefV8Context* cls)
    : CefCppToC<CefV8ContextCppToC, CefV8Context, cef_v8context_t>(cls) {
  struct_.struct_.get_task_runner = v8context_get_task_runner;
  struct_.struct_.is_valid = v8context_is_valid;
  struct_.struct_.get_browser = v8context_get_browser;
  struct_.struct_.get_frame = v8context_get_frame;
  struct_.struct_.get_global = v8context_get_global;
  struct_.struct_.enter = v8context_enter;
  struct_.struct_.exit = v8context_exit;
  struct_.struct_.is_same = v8context_is_same;
  struct_.struct_.eval = v8context_eval;
}

#ifndef NDEBUG
template<> base::AtomicRefCount CefCppToC<CefV8ContextCppToC, CefV8Context,
    cef_v8context_t>::DebugObjCt = 0;
#endif

