/*
 * Copyright (c) 2012 Citrix Systems, Inc.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#pragma once
#include "resource.h"       // main symbols
#include "XenPVDAccess.h"

#define XSW_MAX_WATCHES 16

class CXenStoreWrapper
{
private:
	static void       *m_fps;
        static HMODULE     m_hXSPVDriver;

        void *m_xsh;
	LPVOID             m_watches[16];
	
	
public:
	CXenStoreWrapper() : m_xsh(NULL)
	{
		::ZeroMemory(m_watches, 16*sizeof(LPVOID));
	}

	~CXenStoreWrapper()
	{
                XSPVDriverClose();
	}

        static bool XSPVDriverInitialize();
        static void XSPVDriverUninitialize();
	
        bool XSPVDriverOpen();
        bool XSPVDriverClose();
        bool XSPVDriverWrite(LPCSTR szPath, LPCSTR szData);
        bool XSPVDriverWriteBin(LPCSTR szPath, LPVOID pvData, DWORD dwLen);
        bool XSPVDriverRead(LPCSTR szPath, size_t bufferSize, char *result);
        bool XSPVDriverDirectory(LPCSTR szPath, size_t bufferByteCount, char *resultBuffer, unsigned &listCount);
        bool XSPVDriverRemove(LPCSTR szPath);
        LPVOID XSPVDriverWatch(LPCSTR szPath, HANDLE hEvent);
        void XSPVDriverUnwatch(LPVOID pvWatch);
};
