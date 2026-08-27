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

typedef HRESULT (WINAPI _ATL_CREATORARGFUNC)(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw);

struct _ATL_INTMAP_ENTRY {
  const IID* piid;
  DWORD_PTR dw;
  _ATL_CREATORARGFUNC* pFunc;
};

#define _ATL_SIMPLEMAPENTRY ((_ATL_CREATORARGFUNC*)1)
#define _ATL_2NDMAPENTRY    ((_ATL_CREATORARGFUNC*)2)

template <class ThreadModel>
class CComObjectRootEx {
public:
  ULONG m_dwRef = 0;
  ULONG InternalAddRef() { return ++m_dwRef; }
  ULONG InternalRelease() { return --m_dwRef; }

  static HRESULT WINAPI InternalQueryInterface(void* pThis,
                                               const _ATL_INTMAP_ENTRY* pEntries,
                                               REFIID iid,
                                               void** ppvObject) {
    if (!pThis || !ppvObject) return E_POINTER;
    if (iid == IID_IUnknown) {
      *ppvObject = pThis;
      static_cast<IUnknown*>(pThis)->AddRef();
      return S_OK;
    }
    for (const _ATL_INTMAP_ENTRY* p = pEntries; p->piid != nullptr; ++p) {
      if (*(p->piid) == iid) {
        *ppvObject = (void*)((DWORD_PTR)pThis + p->dw);
        static_cast<IUnknown*>(*ppvObject)->AddRef();
        return S_OK;
      }
    }
    return E_NOINTERFACE;
  }
};

template <class Base>
class CComObject : public Base {
public:
  CComObject() = default;
  virtual ~CComObject() = default;

  STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override {
    return Base::_InternalQueryInterface(this, Base::_GetEntries(), riid, ppv);
  }
  STDMETHOD_(ULONG, AddRef)() override {
    return this->InternalAddRef();
  }
  STDMETHOD_(ULONG, Release)() override {
    ULONG res = this->InternalRelease();
    if (res == 0) delete this;
    return res;
  }

  static HRESULT CreateInstance(CComObject<Base>** pp) {
    if (!pp) return E_POINTER;
    *pp = new CComObject<Base>();
    return S_OK;
  }
};

template <class T,
          const IID* piid = nullptr,
          const GUID* plibid = nullptr,
          WORD wMajor = 1,
          WORD wMinor = 0,
          class tihclass = void>
class ATL_NO_VTABLE IDispatchImpl : public T {
public:
  // IDispatch methods
  STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override {
    if (!pctinfo) return E_POINTER;
    *pctinfo = 0;
    return S_OK;
  }
  STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override {
    if (!ppTInfo) return E_POINTER;
    *ppTInfo = nullptr;
    return E_NOTIMPL;
  }
  STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
                           LCID lcid, DISPID* rgDispId) override {
    return E_NOTIMPL;
  }
  STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
                    WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
                    EXCEPINFO* pExcepInfo, UINT* puArgErr) override {
    return E_NOTIMPL;
  }
};

#define BEGIN_COM_MAP(x) \
public: \
  typedef x _ComMapClass; \
  static HRESULT WINAPI _InternalQueryInterface(void* pThis, const _ATL_INTMAP_ENTRY* pEntries, REFIID iid, void** ppvObject) { \
    return x::InternalQueryInterface(pThis, pEntries, iid, ppvObject); \
  } \
  static const _ATL_INTMAP_ENTRY* WINAPI _GetEntries() { \
    static const _ATL_INTMAP_ENTRY _entries[] = {

#define COM_INTERFACE_ENTRY(x) \
    {&__uuidof(x), (DWORD_PTR)((x*)((_ComMapClass*)8))-8, _ATL_SIMPLEMAPENTRY},

#define COM_INTERFACE_ENTRY2(x, x2) \
    {&__uuidof(x), (DWORD_PTR)((x*)((x2*)((_ComMapClass*)8)))-8, _ATL_SIMPLEMAPENTRY},

#define COM_INTERFACE_ENTRY_IID(iid, x) \
    {&iid, (DWORD_PTR)((x*)((_ComMapClass*)8))-8, _ATL_SIMPLEMAPENTRY},

#define END_COM_MAP() \
    {nullptr, 0, nullptr} \
    }; \
    return _entries; \
  }

class CComModule;
inline CComModule* _pAtlModule = nullptr;

class CComModule {
public:
  CComModule() { _pAtlModule = this; }
};

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
