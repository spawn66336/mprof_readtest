#pragma once

#include <stdio.h>
#include <string>

typedef enum {
	MONO_PROFILER_FILE_BLOCK_KIND_INTRO = 1,
	MONO_PROFILER_FILE_BLOCK_KIND_END = 2,
	MONO_PROFILER_FILE_BLOCK_KIND_MAPPING = 3,
	MONO_PROFILER_FILE_BLOCK_KIND_LOADED = 4,
	MONO_PROFILER_FILE_BLOCK_KIND_UNLOADED = 5,
	MONO_PROFILER_FILE_BLOCK_KIND_EVENTS = 6,
	MONO_PROFILER_FILE_BLOCK_KIND_STATISTICAL = 7,
	MONO_PROFILER_FILE_BLOCK_KIND_HEAP_DATA = 8,
	MONO_PROFILER_FILE_BLOCK_KIND_HEAP_SUMMARY = 9,
	MONO_PROFILER_FILE_BLOCK_KIND_DIRECTIVES = 10
} MonoProfilerFileBlockKind;

typedef struct _Profile_Block_Header
{
public:
	_Profile_Block_Header():
		code(0), 
		size(0), 
		counter_delta(0)
	{

	}
	unsigned short code;
	unsigned int size;
	unsigned int counter_delta;
}Profile_Block_Header_t;

typedef struct _Profile_Block
{
public:

	_Profile_Block():
		head(),
		pbuf(0)
	{ 
	}
	~_Profile_Block()
	{
		if (pbuf)
		{
			delete[] pbuf;
			pbuf = NULL;
		}
	}

	Profile_Block_Header_t head;
	unsigned char* pbuf;
}Profile_Block_t;

class ProfilerReaderUtil
{
public: 

	static bool ReadUShort(FILE* stream, unsigned short& val);
	static bool ReadUInt(FILE* stream, unsigned int& val);
	static bool ReadUInt64(FILE* stream, unsigned __int64& val);
	static bool ReadString(FILE* stream, std::string& val);
	static bool ReadBuffer(FILE* stream, void* buf, unsigned int size);
	static bool ReadBlockHeader(FILE* stream,Profile_Block_Header_t& val);
	static bool ReadBlock(FILE* stream, Profile_Block_t& val);
	
};

std::string MonoProfilerFileBlockKindMap(unsigned int code);
