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

#include "stdafx.h"
#include "resource.h"
#include "XenStoreWrapper.h"

#define XSW_XENTOOLS_REGKEY  _T("Software\\Citrix\\XenTools")
#define XSW_DEFAULT_XSPVDRIVERPATH  _T("C:\\Program Files\\Citrix\\XenTools\\XenPVDAccess.dll")
#define XSW_XENTOOLS_XSPVDRIVERDLL _T("XenPVDAccess.dll")

#if !defined(_WIN64)
#define XSPVDRIVER_OPEN "_XSPVDriver_open@0"
#else
#define XSPVDRIVER_OPEN "XSPVDriver_open"
#endif

#if !defined(_WIN64)
#define XSPVDRIVER_CLOSE "_XSPVDriver_close@4"
#else
#define XSPVDRIVER_CLOSE "XSPVDriver_close"
#endif

#if !defined(_WIN64)
#define XSPVDRIVER_FREE "_XSPVDriver_free@4"
#else
#define XSPVDRIVER_FREE "XSPVDriver_free"
#endif

#if !defined(_WIN64)
#define XSPVDRIVER_WRITE "_XSPVDriver_write@12"
#else
#define XSPVDRIVER_WRITE "XSPVDriver_write"
#endif

#if !defined(_WIN64)
#define XSPVDRIVER_READ "_XSPVDriver_read@16"
#else
#define XSPVDRIVER_READ "XSPVDriver_read"
#endif

#if !defined(_WIN64)
#define XSPVDRIVER_DIRECTORY "_XSPVDriver_directory@16"
#else
#define XSPVDRIVER_DIRECTORY "XSPVDriver_directory"
#endif

#if !defined(_WIN64)
#define XSPVDRIVER_REMOVE "_XSPVDriver_remove@8"
#else
#define XSPVDRIVER_REMOVE "XSPVDriver_remove"
#endif

#if !defined(_WIN64)
#define XSPVDRIVER_WATCH "_XSPVDriver_watch@12"
#else
#define XSPVDRIVER_WATCH "XSPVDriver_watch"
#endif

#if !defined(_WIN64)
#define XSPVDRIVER_UNWATCH "_XSPVDriver_unwatch@4"
#else
#define XSPVDRIVER_UNWATCH "XSPVDriver_unwatch"
#endif

typedef void *(WINAPI *XSPVDriver_open_t)(void);
typedef void (WINAPI *XSPVDriver_close_t)(void *handle);
typedef void (WINAPI *XSPVDriver_free_t)(const void *mem);
typedef BOOL (WINAPI *XSPVDriver_write_t)(void *handle,
								   const char *path,
								   const char *data);
typedef BOOL (WINAPI *XSPVDriver_write_bin_t)(void *handle,
									   const char *path,
									   const void *data,
									   size_t size);
typedef char** (WINAPI *XSPVDriver_directory_t)(void *handle,
                                                const char *path,
                                                size_t bufferByteCount,
                                                char *returnBuffer);
typedef BOOL (WINAPI *XSPVDriver_read_t)(void *handle,
								   const char *path,
                                                                   size_t bufferLen,
                                                                   char *readBuffer);
typedef BOOL (WINAPI *XSPVDriver_remove_t)(void *handle,
									const char *path);
typedef struct XSPVDriver_watch* (WINAPI *XSPVDriver_watch_t)(void *handle,
												const char *path,
												HANDLE event);
typedef void (WINAPI *XSPVDriver_unwatch_t)(struct XSPVDriver_watch *watch);

typedef struct _XSW_FUNCTIONS {
        XSPVDriver_open_t      fp_XSPVDriver_open;
        XSPVDriver_close_t     fp_XSPVDriver_close;
        XSPVDriver_write_t     fp_XSPVDriver_write;
        XSPVDriver_write_bin_t fp_XSPVDriver_write_bin;
        XSPVDriver_read_t      fp_XSPVDriver_read;
        XSPVDriver_directory_t fp_XSPVDriver_directory;
        XSPVDriver_remove_t    fp_XSPVDriver_remove;
        XSPVDriver_watch_t     fp_XSPVDriver_watch;
        XSPVDriver_unwatch_t   fp_XSPVDriver_unwatch;
} XSW_FUNCTIONS;

void *CXenStoreWrapper::m_fps = NULL;
HMODULE CXenStoreWrapper::m_hXSPVDriver = NULL;

bool CXenStoreWrapper::XSPVDriverInitialize()
{
    OutputDebugStringA(__FUNCTION__);
    OutputDebugStringA("\n");
        bool rc = false;
	LONG lRes;
	DWORD dwLen;
	CRegKey clXenTools;
        TCHAR tszXSPVDriverPath[_MAX_PATH + 1];
	XSW_FUNCTIONS *fps;

	do {
                if (m_hXSPVDriver != NULL)
		{
			::SetLastError(ERROR_GEN_FAILURE);
			break;
		}

		m_fps = malloc(sizeof(XSW_FUNCTIONS));
		if (m_fps == NULL)
		{
			::SetLastError(ERROR_OUTOFMEMORY);
			break;
		}
		::ZeroMemory(m_fps, sizeof(XSW_FUNCTIONS));
		fps = (XSW_FUNCTIONS*)m_fps;

		// Load a default location
                _tcsncpy_s(tszXSPVDriverPath, _MAX_PATH, XSW_DEFAULT_XSPVDRIVERPATH, _TRUNCATE);

		// Find library in registry, load and get proc addresses.
		lRes = clXenTools.Open(HKEY_LOCAL_MACHINE, XSW_XENTOOLS_REGKEY, KEY_READ);
		if (lRes == ERROR_SUCCESS)
		{
                    OutputDebugStringA("Openned the registry\n");
			dwLen = _MAX_PATH;
                        lRes = clXenTools.QueryStringValue(XSW_XENTOOLS_XSPVDRIVERDLL, tszXSPVDriverPath, &dwLen);
			if ((lRes != ERROR_SUCCESS)||(dwLen == 0))
			{
                                _tcsncpy_s(tszXSPVDriverPath, _MAX_PATH, XSW_DEFAULT_XSPVDRIVERPATH, _TRUNCATE);
			}
		}


                m_hXSPVDriver = ::LoadLibrary(tszXSPVDriverPath);
                if (m_hXSPVDriver == NULL) {
                    OutputDebugStringA("Driver Load Failed\n");

			break;
                }

                OutputDebugStringA("Mapping functions\n");

#define XSW_CHECK_FP(f, function) \
        if (f == NULL) {::SetLastError(ERROR_INVALID_FUNCTION); OutputDebugStringA(function); OutputDebugStringA("not found.\n");}

                fps->fp_XSPVDriver_open = (XSPVDriver_open_t)::GetProcAddress(m_hXSPVDriver, XSPVDRIVER_OPEN);
                XSW_CHECK_FP(fps->fp_XSPVDriver_open, XSPVDRIVER_OPEN);

                fps->fp_XSPVDriver_close = (XSPVDriver_close_t)::GetProcAddress(m_hXSPVDriver, XSPVDRIVER_CLOSE);
                XSW_CHECK_FP(fps->fp_XSPVDriver_close, XSPVDRIVER_CLOSE);

                fps->fp_XSPVDriver_write = (XSPVDriver_write_t)::GetProcAddress(m_hXSPVDriver, XSPVDRIVER_WRITE);
                XSW_CHECK_FP(fps->fp_XSPVDriver_write, XSPVDRIVER_WRITE);

                fps->fp_XSPVDriver_read = (XSPVDriver_read_t)::GetProcAddress(m_hXSPVDriver, XSPVDRIVER_READ);
                XSW_CHECK_FP(fps->fp_XSPVDriver_read, XSPVDRIVER_READ);

                fps->fp_XSPVDriver_directory = (XSPVDriver_directory_t)::GetProcAddress(m_hXSPVDriver, XSPVDRIVER_DIRECTORY);
                XSW_CHECK_FP(fps->fp_XSPVDriver_directory, XSPVDRIVER_DIRECTORY);

                fps->fp_XSPVDriver_remove = (XSPVDriver_remove_t)::GetProcAddress(m_hXSPVDriver, XSPVDRIVER_REMOVE);
                XSW_CHECK_FP(fps->fp_XSPVDriver_remove, XSPVDRIVER_REMOVE);

                fps->fp_XSPVDriver_watch = (XSPVDriver_watch_t)::GetProcAddress(m_hXSPVDriver, XSPVDRIVER_WATCH);
                XSW_CHECK_FP(fps->fp_XSPVDriver_watch, XSPVDRIVER_WATCH);

                fps->fp_XSPVDriver_unwatch = (XSPVDriver_unwatch_t)::GetProcAddress(m_hXSPVDriver, XSPVDRIVER_UNWATCH);
                XSW_CHECK_FP(fps->fp_XSPVDriver_unwatch, XSPVDRIVER_UNWATCH);

		rc = true;
                OutputDebugStringA("Map function complete\n");

	} while (false);

	if (!rc)
                CXenStoreWrapper::XSPVDriverUninitialize();

	return rc;
}

void CXenStoreWrapper::XSPVDriverUninitialize()
{
        if (m_hXSPVDriver != NULL)
	{
                ::FreeLibrary(m_hXSPVDriver);
                m_hXSPVDriver = NULL;
	}

	if (m_fps != NULL)
	{
		free(m_fps);
		m_fps = NULL;
	}
}

bool CXenStoreWrapper::XSPVDriverOpen()
{
	XSW_FUNCTIONS *fps = (XSW_FUNCTIONS*)m_fps;

        if (m_hXSPVDriver == NULL)
	{
		::SetLastError(ERROR_INVALID_FUNCTION);
		return false;
	}

        // Note if the XSPVDriver call fails it will set the last error
        m_xsh = fps->fp_XSPVDriver_open();
	
	return (m_xsh != NULL) ? true : false;
}

bool CXenStoreWrapper::XSPVDriverClose()
{
	XSW_FUNCTIONS *fps = (XSW_FUNCTIONS*)m_fps;
	ULONG i;

        if ((m_hXSPVDriver == NULL)||(m_xsh == NULL))
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Clean up any watches that were opened
	for (i = 0; i < XSW_MAX_WATCHES; i++)
	{
		if (m_watches[i] != NULL)
		{
                        XSPVDriverUnwatch(m_watches[i]);
			m_watches[i] = NULL;
		}
	}

        fps->fp_XSPVDriver_close(m_xsh);
	m_xsh = NULL;

	return true;
}

bool CXenStoreWrapper::XSPVDriverWrite(LPCSTR szPath, LPCSTR szData)
{
	XSW_FUNCTIONS *fps = (XSW_FUNCTIONS*)m_fps;

        if ((m_hXSPVDriver == NULL)||(m_xsh == NULL)||(szPath == NULL)||(szData == NULL))
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

        if (!fps->fp_XSPVDriver_write(m_xsh, szPath, szData))
		return false;

	return true;
}

bool CXenStoreWrapper::XSPVDriverWriteBin(LPCSTR szPath, LPVOID pvData, DWORD dwLen)
{
	XSW_FUNCTIONS *fps = (XSW_FUNCTIONS*)m_fps;

        if ((m_hXSPVDriver == NULL)||(m_xsh == NULL)||(szPath == NULL)||(pvData == NULL))
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

        if (!fps->fp_XSPVDriver_write_bin(m_xsh, szPath, pvData, dwLen))
		return false;

	return true;
}

bool CXenStoreWrapper::XSPVDriverRead(LPCSTR szPath, size_t bufferSize, char *result)
{
	XSW_FUNCTIONS *fps = (XSW_FUNCTIONS*)m_fps;

        if ((m_hXSPVDriver == NULL)||(m_xsh == NULL)||(szPath == NULL))
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

        return fps->fp_XSPVDriver_read(m_xsh, szPath, bufferSize, result) ? true : false;
}

bool CXenStoreWrapper::XSPVDriverDirectory(LPCSTR szPath, size_t bufferByteCount, char *resultBuffer, unsigned &listCount)
{
        if ((m_hXSPVDriver == NULL)||(m_xsh == NULL)||(0 == bufferByteCount)||(0 == resultBuffer))
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
                return false;
	}

        listCount = 0;
        XSW_FUNCTIONS *fps = (XSW_FUNCTIONS*)m_fps;
        if (fps->fp_XSPVDriver_directory(m_xsh, szPath, bufferByteCount, resultBuffer)) {
            // Count the number of entries in the list by detecting a double null termination
            char *resultOffset = resultBuffer;
            do {
                resultOffset = &resultOffset[strlen(resultOffset) + 2];
                listCount++;
            } while (*resultOffset);

            return true;
        }

        return false;
}

bool CXenStoreWrapper::XSPVDriverRemove(LPCSTR szPath)
{
	XSW_FUNCTIONS *fps = (XSW_FUNCTIONS*)m_fps;

        if ((m_hXSPVDriver == NULL)||(m_xsh == NULL)||(szPath == NULL))
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

        if (!fps->fp_XSPVDriver_remove(m_xsh, szPath))
		return false;

	return true;
}

LPVOID CXenStoreWrapper::XSPVDriverWatch(LPCSTR szPath, HANDLE hEvent)
{
	XSW_FUNCTIONS *fps = (XSW_FUNCTIONS*)m_fps;
	void *pv;
	ULONG i;

        if ((m_hXSPVDriver == NULL)||(m_xsh == NULL)||(szPath == NULL)||(hEvent == NULL))
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	for (i = 0; i < XSW_MAX_WATCHES; i++)
	{
		if (m_watches[i] == NULL)
			break;
	}

	if (i >= XSW_MAX_WATCHES)
	{
		::SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

        pv = (LPVOID)fps->fp_XSPVDriver_watch(m_xsh, szPath, hEvent);
	m_watches[i] = pv;

	return pv;
}

void CXenStoreWrapper::XSPVDriverUnwatch(LPVOID pvWatch)
{
	XSW_FUNCTIONS *fps = (XSW_FUNCTIONS*)m_fps;
	ULONG i;

        if ((m_hXSPVDriver == NULL)||(m_xsh == NULL)||(pvWatch == NULL))
		return;

	for (i = 0; i < XSW_MAX_WATCHES; i++)
	{
		if (m_watches[i] == pvWatch)
		{
			m_watches[i] = NULL;
			break;
		}
	}

        fps->fp_XSPVDriver_unwatch((struct XSPVDriver_watch*)pvWatch);
}
