#pragma once

class StringItem
{
public:
	StringItem(System::String^ str)
	{
		int l = str->Length;
		m_buf = new char[l + 1];
		for (int i = 0; i < l; ++i)
			m_buf[i] = str[i];
		m_buf[l] = 0;
	}
	~StringItem()
	{
		delete[]m_buf;
	}
	const char* getBuf()
	{
		return m_buf;
	}
protected:
	char* m_buf;
};