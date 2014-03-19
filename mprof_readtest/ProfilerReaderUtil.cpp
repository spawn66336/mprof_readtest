#include "ProfilerReaderUtil.h"

#define VERIFY_STREAM(s) do{\
if (!s || feof(s)) return false; \
}while (0)


bool ProfilerReaderUtil::ReadUShort(FILE* stream, unsigned short& val)
{
	VERIFY_STREAM(stream);
	fread_s(&val, sizeof(val), sizeof(val), 1, stream);
	return true;
}

bool ProfilerReaderUtil::ReadUInt(FILE* stream, unsigned int& val)
{
	VERIFY_STREAM(stream);
	fread_s(&val, sizeof(val), sizeof(val), 1, stream);
	return true;
}

bool ProfilerReaderUtil::ReadUInt64(FILE* stream, unsigned __int64& val)
{
	VERIFY_STREAM(stream);
	fread_s(&val, sizeof(val), sizeof(val), 1, stream);
	return true;
}

bool ProfilerReaderUtil::ReadString(FILE* stream, std::string& val)
{
	VERIFY_STREAM(stream);
	val.clear();
	char c = 0;
	while (!feof(stream))
	{
		fread(&c, 1, 1, stream);
		if (0 == c)
			break;
		val.push_back(c);
	}
	return true;
}


bool ProfilerReaderUtil::ReadBuffer(FILE* stream, void* buf, unsigned int size)
{
	VERIFY_STREAM(stream);
	size_t bytesRead = fread_s(buf, size,1,size, stream);
	if (bytesRead != size)
		return false;
	return true;
}


bool ProfilerReaderUtil::ReadBlockHeader(FILE* stream, Profile_Block_Header_t& val)
{
	VERIFY_STREAM(stream);
	bool rs = false;
	rs = ReadUShort(stream, val.code);
	rs = ReadUInt(stream, val.size);
	rs = ReadUInt(stream, val.counter_delta);
	return rs;
}

bool ProfilerReaderUtil::ReadBlock(FILE* stream, Profile_Block_t& val)
{
	VERIFY_STREAM(stream);
	bool rs = false;
	rs = ReadBlockHeader(stream, val.head);
	if (!rs) return false;

	if (val.pbuf)
	{
		delete[] val.pbuf;
		val.pbuf = NULL;
	}
	val.pbuf = new unsigned char[val.head.size];
	rs = ReadBuffer(stream, val.pbuf, val.head.size);
	return rs;
}

std::string MonoProfilerFileBlockKindMap(unsigned int code)
{
	switch (code)
	{
		case MONO_PROFILER_FILE_BLOCK_KIND_INTRO:
			return "KIND_INTRO";
			break;
		case MONO_PROFILER_FILE_BLOCK_KIND_END:
			return "KIND_END";
			break;
		case MONO_PROFILER_FILE_BLOCK_KIND_MAPPING:
			return "KIND_MAPPING";
			break;
		case MONO_PROFILER_FILE_BLOCK_KIND_LOADED:
			return "KIND_LOADED";
			break;
		case MONO_PROFILER_FILE_BLOCK_KIND_UNLOADED:
			return "KIND_UNLOADED";
			break;
		case MONO_PROFILER_FILE_BLOCK_KIND_EVENTS:
			return "KIND_EVENTS";
			break;
		case MONO_PROFILER_FILE_BLOCK_KIND_STATISTICAL:
			return "KIND_STATISTICAL";
			break;
		case MONO_PROFILER_FILE_BLOCK_KIND_HEAP_DATA:
			return "KIND_HEAP_DATA";
			break;
		case MONO_PROFILER_FILE_BLOCK_KIND_HEAP_SUMMARY:
			return "KIND_HEAP_SUMMARY";
			break;
		case MONO_PROFILER_FILE_BLOCK_KIND_DIRECTIVES:
			return "KIND_DIRECTIVES";
			break;
		default:
			break;
	}
	return "KIND_UNKNOWN";
}
