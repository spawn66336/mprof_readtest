#include "Stdafx.h"
#include "Util.h"
#include <windows.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)


UMConverter::UMConverter(const char *str)
: _IsMBCS(true)
, _UnicodeString(NULL)
, _MBCSString(NULL)
{
	if (str == NULL)
	{
		_MBCSString = NULL;
		return;
	}
	int len = ::MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, NULL, 0);
	_UnicodeString = new wchar_t[len];
	::MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, _UnicodeString, len);

	_MBCSString = const_cast<char *>(str);
}

UMConverter::UMConverter(const wchar_t *str)
: _IsMBCS(false)
, _MBCSString(NULL)
{
	if (str == NULL)
	{
		_UnicodeString = NULL;
		return;
	}
	int len = ::WideCharToMultiByte(CP_ACP, 0, str, wcslen(str) + 1, NULL, 0, NULL, NULL);
	_MBCSString = new char[len];
	::WideCharToMultiByte(CP_ACP, 0, str, wcslen(str) + 1, _MBCSString, len, NULL, NULL);

	_UnicodeString = const_cast<wchar_t *>(str);
}

UMConverter::~UMConverter(void)
{
	if (_IsMBCS)
	{
		if (_UnicodeString != NULL)
		{
			delete[]_UnicodeString;
			_UnicodeString = NULL;
		}
	}
	else
	{
		if (_MBCSString != NULL)
		{
			delete[]_MBCSString;
			_MBCSString = NULL;
		}
	}
}



#else//this is need for compile. 

UMConverter::UMConverter(const char *str)
: _IsMBCS(true)
, _UnicodeString(NULL)
{
	if (str == 0)
		return;

	_MBCSString = new char[strlen(str) + 1];
	_MBCSString[strlen(str)] = 0;
	strcpy(_MBCSString, str);
}

UMConverter::UMConverter(const wchar_t *str)
: _IsMBCS(false)
, _MBCSString(NULL)
{
	//	if(str == 0)
	//		return;
	//
	//	_UnicodeString = new wchar_t[strlen(str)+1];
	//	_UnicodeString[strlen(str)] = 0;
	//	strcpy(_UnicodeString, str);
}

UMConverter::~UMConverter(void)
{
	if (_IsMBCS)
	{
		if (_MBCSString != NULL)
		{
			delete[]_MBCSString;
			_MBCSString = NULL;
		}
	}
}

#endif