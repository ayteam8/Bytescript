/*
 * Copyright (c) 2026 Guille
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef _WIN32

#include_next <windows.h>

#else

#ifndef _FAKE_WINDOWS_H
#define _FAKE_WINDOWS_H

#include <limits.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

typedef void* HMODULE;
typedef void (*FARPROC)(void);

static inline void dll2so(char *name) {
	char *last_ch = strrchr(name, '.');
	if (last_ch==NULL) return;
	memcpy(last_ch, ".so", 4);
}

static inline HMODULE LoadLibrary(char* lpLibFileName) {
	//dll2so(lpLibFileName);
	return dlopen(lpLibFileName, RTLD_LAZY);
}

static inline void *GetProcAddress(HMODULE hModule, const char* lpProcName) {
	return dlsym((void*)hModule, lpProcName);	
}

static inline uint32_t GetModuleFileName(HMODULE hModule, char *buffer, uint32_t nSize) {
	Dl_info info;
	memset(buffer, 0, nSize);
	if (hModule!=NULL) {
		// devolvemos el path del basura esta...
		// eso como se llame el path del .so
		if (dladdr(hModule, &info)!=0) return 0; // oh no error
		strncpy(buffer, info.dli_fname, nSize);
		return nSize-1; // me da pereza hacerlo bn
	} else {
		// pillamos el del propio programa
		// jejejej
		return readlink("/proc/self/exe", buffer, nSize-1);
	}
}

static inline uint32_t GetFullPathNameA(const char *lpFileName, uint32_t nBufferLength, char *lpBuffer, char *lpFilePart) {
	char temp[PATH_MAX];
	char *buffer = realpath(lpFileName, temp);
	if (buffer==NULL) return 0;
	if (nBufferLength!=0 && lpBuffer!=NULL)  {
		// OK, ok, entonces hagamos strncpy
		strncpy(lpBuffer, buffer, nBufferLength);
	}
		
	uint32_t length = (uint32_t)strlen(buffer)+1;
	return ((length < nBufferLength) || (nBufferLength==0)) ? length : nBufferLength;
}

typedef union _LARGE_INTEGER {
    struct {
        uint32_t LowPart;
        uint32_t  HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        uint32_t LowPart;
        uint32_t  HighPart;
    } u;
    uint64_t QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

// LARGE_INTEGER
// QueryPerformanceCounter(LARGE_INTEGER *tick);
// QueryPerformanceFrequency(LARGE_INTEGER *freq);
// Sleep();

static inline void Sleep(uint32_t stime) {
	usleep(stime*1000);
}

static inline bool QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency) {
    if (!lpFrequency) return false;
    
    
    lpFrequency->QuadPart = 1000000000LL; 
    return true;
}


static inline bool QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount) {
    if (!lpPerformanceCount) return false;
    
    struct timespec ts;
    
    
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0) {
        return false;
    }
    
    
    lpPerformanceCount->QuadPart = ((uint64_t)ts.tv_sec * 1000000000LL) + ts.tv_nsec;
    return true;
}

#endif

#endif
