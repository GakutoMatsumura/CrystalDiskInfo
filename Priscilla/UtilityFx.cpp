/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : https://crystalmark.info/
//      License : The MIT License
/*---------------------------------------------------------------------------*/

#include "../stdafx.h"
#include "UtilityFx.h"

#include <io.h>
#pragma comment(lib,"version.lib")

////------------------------------------------------
//   Debug
////------------------------------------------------

static const DWORD DEBUG_MODE_NONE = 0;
static const DWORD DEBUG_MODE_LOG = 1;
static const DWORD DEBUG_MODE_MESSAGE = 2;

static DWORD debugMode = DEBUG_MODE_NONE;

void SetDebugMode(DWORD mode)
{
	if (mode <= DEBUG_MODE_MESSAGE)
	{
		debugMode = mode;
	}
	else
	{
		debugMode = DEBUG_MODE_NONE;
	}
}

void DebugPrint(CString cstr)
{
	static BOOL flag = TRUE;
	static TCHAR file[MAX_PATH] = L"";
	static DWORD first = (DWORD)GetTickCountFx();
	CString output;

	output.Format(L"%08d ", (DWORD)GetTickCountFx() - first);
	output += cstr;
	output.Append(L"\n");
	output.Replace(L"\r", L"");

	if (flag)
	{
		TCHAR* ptrEnd;
		::GetModuleFileName(NULL, file, MAX_PATH);
		if ((ptrEnd = _tcsrchr(file, '.')) != NULL)
		{
			*ptrEnd = '\0';
			_tcscat_s(file, MAX_PATH, L".log");
		}
		DeleteFile(file);
		flag = FALSE;
	}

	if (debugMode == DEBUG_MODE_NONE)
	{
		return;
	}

	FILE* fp;
	_tfopen_s(&fp, file, L"ac");
	if (fp != NULL)
	{
		_ftprintf(fp, L"%s", (LPCTSTR)output);
		fflush(fp);
		fclose(fp);
	}

	if (debugMode == DEBUG_MODE_MESSAGE)
	{
		AfxMessageBox(output);
	}
}

////------------------------------------------------
//   File Information
////------------------------------------------------

int GetFileVersion(const TCHAR* file, TCHAR* version)
{
	ULONG reserved = 0;
	VS_FIXEDFILEINFO vffi{};
	TCHAR* buf = NULL;
	int  Locale = 0;
	TCHAR str[256]{};
	//str[0] = '\0';

	UINT size = GetFileVersionInfoSize((TCHAR*)file, &reserved);
	TCHAR* vbuf = new TCHAR[size];
	if (GetFileVersionInfo((TCHAR*)file, 0, size, vbuf))
	{
		VerQueryValue(vbuf, L"\\", (void**)&buf, &size);
		CopyMemory(&vffi, buf, sizeof(VS_FIXEDFILEINFO));

		VerQueryValue(vbuf, L"\\VarFileInfo\\Translation", (void**)&buf, &size);
		CopyMemory(&Locale, buf, sizeof(int));
		wsprintf(str, L"\\StringFileInfo\\%04X%04X\\%s",
			LOWORD(Locale), HIWORD(Locale), L"FileVersion");
		VerQueryValue(vbuf, str, (void**)&buf, &size);

		_tcscpy_s(str, 256, buf);
		if (version != NULL)
		{
			_tcscpy_s(version, 256, buf);
		}
	}
	delete[] vbuf;

	if (_tcscmp(str, L"") != 0)
	{
		return int(_tstof(str) * 100);
	}
	else
	{
		return 0;
	}
}

BOOL IsFileExist(const TCHAR* path)
{
	FILE* fp;
	errno_t err;

	err = _tfopen_s(&fp, path, L"rb");
	if (err != 0 || fp == NULL)
	{
		return FALSE;
	}
	fclose(fp);
	return TRUE;
}

////------------------------------------------------
//   Utility
////------------------------------------------------

typedef ULONGLONG(WINAPI* FuncGetTickCount64)();

ULONGLONG GetTickCountFx()
{
	static FuncGetTickCount64 pGetTickCount64 = (FuncGetTickCount64)GetProcAddress(GetModuleHandle(L"kernel32"), "GetTickCount64");

	if (pGetTickCount64 != NULL)
	{
		return (ULONGLONG)pGetTickCount64();
	}
	else
	{
#pragma warning( disable : 28159 )
		return (ULONGLONG)GetTickCount();
#pragma warning( error : 28159 )
	}
}

ULONG64 B8toB64(BYTE b0, BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6, BYTE b7)
{
	ULONG64 data =
		  ((ULONG64)b7 << 56)
		+ ((ULONG64)b6 << 48)
		+ ((ULONG64)b5 << 40)
		+ ((ULONG64)b4 << 32)
		+ ((ULONG64)b3 << 24)
		+ ((ULONG64)b2 << 16)
		+ ((ULONG64)b1 <<  8)
		+ ((ULONG64)b0 <<  0);

	return data;
}

DWORD B8toB32(BYTE b0, BYTE b1, BYTE b2, BYTE b3)
{
	DWORD data =
		  ((DWORD)b3 << 24)
		+ ((DWORD)b2 << 16)
		+ ((DWORD)b1 <<  8)
		+ ((DWORD)b0 <<  0);

	return data;
}


//+ 20220502
/*
need manifest
      <!-- Windows Vista -->
      <supportedOS Id="{e2011457-1546-43c5-a5fe-008deee3d3f0}"/>
      <!-- Windows 7 -->
      <supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/>
      <!-- Windows 8 -->
      <supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"/>
      <!-- Windows 8.1 -->
      <supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"/>
      <!-- Windows 10 -->
      <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/>
*/
bool UtilityFx__IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor) {
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
	DWORDLONG        const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
			VerSetConditionMask(
				0, VER_MAJORVERSION, VER_GREATER_EQUAL),
			VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = wMajorVersion;
	osvi.dwMinorVersion = wMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}


////------------------------------------------------
//   .ini support function
////------------------------------------------------

DWORD GetPrivateProfileStringFx(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR  lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	CString cstr = lpKeyName;
	cstr.Replace(L"=", L"%#3D");
	return GetPrivateProfileString(lpAppName, cstr, lpDefault, lpReturnedString, nSize, lpFileName);
}

UINT GetPrivateProfileIntFx(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
{
	CString cstr = lpKeyName;
	cstr.Replace(L"=", L"%#3D");
	return GetPrivateProfileInt(lpAppName, cstr, nDefault, lpFileName);
}

BOOL WritePrivateProfileStringFx(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	CString cstr = lpKeyName;
	cstr.Replace(L"=", L"%#3D");
	return WritePrivateProfileString(lpAppName, cstr, lpString, lpFileName);
}