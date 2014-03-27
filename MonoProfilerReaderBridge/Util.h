#pragma once

class StringItem
{
public:
	StringItem(System::String^ str)
	{
		int l = str->Length;
		m_buf = new wchar_t[l + 1];
		for (int i = 0; i < l; ++i)
			m_buf[i] = str[i];
		m_buf[l] = 0;
	}
	~StringItem()
	{
		delete[]m_buf;
	}
	const wchar_t* getBuf()
	{
		return m_buf;
	}
protected:
	wchar_t* m_buf;
};


// unicode与multibyte的转换类
class UMConverter
{
protected:
	bool		_IsMBCS;
	char*		_MBCSString;
	wchar_t*	_UnicodeString;

public:
	// 接收两种类型的字符串的构造函数
	UMConverter(const wchar_t *str);
	UMConverter(const char *str);

	~UMConverter(void);

	// 获得multibyte的字符串
	const char *GetMBCS(void) const { return(_MBCSString); };
	// 获得unicode的字符串
	const wchar_t *GetUnicode(void) const { return(_UnicodeString); };

	static const char* GetMBFromUTF8(const char* utf8){ return utf8; };

protected:
	UMConverter(void);
private:
	// disable
	UMConverter(const UMConverter&);
	const UMConverter& operator=(const UMConverter&);
};