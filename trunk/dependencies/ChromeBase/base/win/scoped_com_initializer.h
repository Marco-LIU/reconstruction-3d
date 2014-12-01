// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_WIN_SCOPED_COM_INITIALIZER_H_
#define BASE_WIN_SCOPED_COM_INITIALIZER_H_

#include <objbase.h>

#include "base/basictypes.h"
#include "base/logging.h"
#include "build/build_config.h"
#include "base/debug/alias.h"

namespace base {
namespace win {

// Initializes COM in the constructor (STA or MTA), and uninitializes COM in the
// destructor.
//
// WARNING: This should only be used once per thread, ideally scoped to a
// similar lifetime as the thread itself.  You should not be using this in
// random utility functions that make COM calls -- instead ensure these
// functions are running on a COM-supporting thread!
class ScopedCOMInitializer {
 public:
  // Enum value provided to initialize the thread as an MTA instead of STA.
  enum SelectMTA { kMTA };

  // Constructor for STA initialization.
  ScopedCOMInitializer() {
    Initialize(COINIT_APARTMENTTHREADED);
  }

  // Constructor for MTA initialization.
  explicit ScopedCOMInitializer(SelectMTA mta) {
    Initialize(COINIT_MULTITHREADED);
  }

  ~ScopedCOMInitializer() {
#ifndef NDEBUG
    // Using the windows API directly to avoid dependency on platform_thread.
    DCHECK_EQ(GetCurrentThreadId(), thread_id_);
#endif
    if (succeeded())
      CoUninitialize();
  }

  bool succeeded() const { return SUCCEEDED(hr_); }

 private:
  void Initialize(COINIT init) {
#ifndef NDEBUG
    thread_id_ = GetCurrentThreadId();
#endif
    // ksbrowser {
    hr_ = CoInitializerSafe(init);
    // }
#ifndef NDEBUG
    if (hr_ == S_FALSE)
      LOG(ERROR) << "Multiple CoInitialize() calls for thread " << thread_id_;
    //else
      //DCHECK_NE(RPC_E_CHANGED_MODE, hr_) << "Invalid COM thread model change";
#endif
  }

  HRESULT hr_;
#ifndef NDEBUG
  // In debug builds we use this variable to catch a potential bug where a
  // ScopedCOMInitializer instance is deleted on a different thread than it
  // was initially created on.  If that ever happens it can have bad
  // consequences and the cause can be tricky to track down.
  DWORD thread_id_;
#endif

  DISALLOW_COPY_AND_ASSIGN(ScopedCOMInitializer);

  // ksbrowser {
private:
  HRESULT CoInitializerSafe(COINIT init) {

    HRESULT hr = S_FALSE;
    bool failed = false;
    __try {
      hr = CoInitializeEx(NULL, init);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
      failed = true;
    }

    if (failed)
    {
      IMalloc* malloc_ptr = NULL;
      CoGetMalloc(1, &malloc_ptr);
      if (malloc_ptr != NULL)
      {
        void* vptr = *(void**)malloc_ptr;
        if (vptr == NULL)
        {
          base::debug::Alias(&vptr);
          base::debug::Alias(&malloc_ptr);
          base::debug::Alias(&hr);

          HMODULE module = GetModuleHandle(L"ole32.dll");
          PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)module;
          PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((PBYTE)dos_header + dos_header->e_lfanew);
          BOOL(WINAPI * fnDllMain)(HMODULE, DWORD, LPVOID);

          (PBYTE&)fnDllMain = (PBYTE)module + nt_headers->OptionalHeader.AddressOfEntryPoint;
          if (fnDllMain(module, DLL_PROCESS_ATTACH, NULL)) {
            hr= CoInitializeEx(NULL, init);
          }
        }
      }
    }
   return hr;
  }
  // }
};

}  // namespace win
}  // namespace base

#endif  // BASE_WIN_SCOPED_COM_INITIALIZER_H_
