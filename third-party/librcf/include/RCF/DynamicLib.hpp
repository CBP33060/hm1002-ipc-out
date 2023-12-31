
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2018, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 3.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

#ifndef INCLUDE_RCF_DYNAMICLIB_HPP
#define INCLUDE_RCF_DYNAMICLIB_HPP

#include <string>
#include <memory>

#include <RCF/Config.hpp>
#include <RCF/Exception.hpp>

#ifdef RCF_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace RCF {

    class                                   DynamicLib;
    typedef std::shared_ptr<DynamicLib>   DynamicLibPtr;

    class DynamicLib
    {
    public:
        DynamicLib(const std::string & dllName);
        virtual ~DynamicLib();

    private:

        std::string mDllName;

#ifdef RCF_WINDOWS

    public:

        template<typename Pfn>
        void loadDllFunction(Pfn & pfn, const std::string & funcName)
        {
            pfn = NULL;
            pfn = (Pfn) GetProcAddress(mhDll, funcName.c_str());
            if (pfn == NULL)
            {
                DWORD dwErr = GetLastError();
                Exception e(RcfError_DllFuncLoad, mDllName, funcName, osError(dwErr));
                throw e;
            }
        }

    private:

        HMODULE mhDll;

#else

    public:

        template<typename Pfn>
        void loadDllFunction(Pfn & pfn, const std::string & funcName)
        {
            pfn = NULL;

            // Consume any existing error value.
            const char * szErr = dlerror();
            RCF_UNUSED_VARIABLE(szErr);

            pfn = (Pfn) dlsym(mhDll, funcName.c_str());
            if (pfn == NULL)
            {
                std::string strErr;
                const char * szErr = dlerror();
                if (szErr)
                {
                    strErr = szErr;
                }
                Exception e(RcfError_UnixDllFuncLoad, mDllName, funcName, strErr);
                throw e;
            }
        }

    private:

        void * mhDll;

#endif

    };

#define RCF_LOAD_DLL_FUNCTION(funcName)                                     \
    mDynamicLibPtr->loadDllFunction<Pfn_##funcName>(pfn_##funcName, #funcName);

#define RCF_LOAD_LIB_FUNCTION(funcName)                                     \
    pfn_##funcName = & funcName;

} // namespace RCF

#endif // ! INCLUDE_RCF_DYNAMICLIB_HPP
