// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_WIN_ATL_H_
#define BASE_WIN_ATL_H_

// Check no prior poisonous defines were made.
#include "base/win/windows_defines.inc"
// Undefine before windows header will make the poisonous defines
#include "base/win/windows_undefines.inc"

// clang-format off
// Declare our own exception thrower (atl_throw.h includes atldef.h).
#include "base/win/atl_throw.h"
// clang-format on

#if defined(__has_include) && __has_include(<atlbase.h>)
// Now include the real ATL headers.
#include <atlbase.h>

#include <atlcom.h>
#include <atlcomcli.h>
#include <atlctl.h>
#include <atlhost.h>
#include <atlsecurity.h>
#include <atltypes.h>
#include <atlwin.h>
#else
#include <unknwn.h>
#include <wrl/client.h>

#ifndef ATL_NO_VTABLE
#define ATL_NO_VTABLE __declspec(novtable)
#endif

class CComSingleThreadModel {};
class CComMultiThreadModel {};

template <class ThreadModel>
class CComObjectRootEx {
public:
  ULONG InternalAddRef() { return 1; }
  ULONG InternalRelease() { return 1; }
};

template <class Base>
class CComObject : public Base {
public:
  ULONG m_refCount = 0;
  CComObject() = default;
  virtual ~CComObject() = default;

  static HRESULT CreateInstance(CComObject<Base>** pp) {
    if (!pp) return E_POINTER;
    *pp = new CComObject<Base>();
    return S_OK;
  }

  STDMETHOD_(ULONG, AddRef)() override {
    return ++m_refCount;
  }

  STDMETHOD_(ULONG, Release)() override {
    ULONG res = --m_refCount;
    if (res == 0) delete this;
    return res;
  }

  STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override {
    return this->_InternalQueryInterface(riid, ppvObject);
  }
};

#define BEGIN_COM_MAP(x) \
  HRESULT _InternalQueryInterface(REFIID riid, void** ppv) { \
    if (!ppv) return E_POINTER; \
    if (riid == IID_IUnknown) { \
      *ppv = this; \
      AddRef(); \
      return S_OK; \
    }

#define COM_INTERFACE_ENTRY(iface) \
    if (riid == __uuidof(iface)) { \
      *ppv = static_cast<iface*>(this); \
      AddRef(); \
      return S_OK; \
    }

#define COM_INTERFACE_ENTRY_IID(iid, iface) \
    if (riid == (iid)) { \
      *ppv = static_cast<iface*>(this); \
      AddRef(); \
      return S_OK; \
    }

#define END_COM_MAP() \
    *ppv = nullptr; \
    return E_NOINTERFACE; \
  }

class CComModule {
public:
  CComModule() { _pAtlModule = this; }
};
inline CComModule* _pAtlModule = nullptr;

namespace ATL {
  using ::CComSingleThreadModel;
  using ::CComMultiThreadModel;
  using ::CComObjectRootEx;
  using ::CComObject;
  using ::CComModule;
}
#endif

// Undefine the poisonous defines
#include "base/win/windows_undefines.inc"  // NOLINT(build/include)
// Check no poisonous defines follow this include
#include "base/win/windows_defines.inc"  // NOLINT(build/include)

#endif  // BASE_WIN_ATL_H_
