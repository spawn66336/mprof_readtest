#include "ProfilerReaderUtil.h"

#define VERIFY_STREAM(s) do{\
if (!s || feof(s)) return false; \
}while (0)


std::map<unsigned int, Profile_Mapping::Class_Info> g_class_mapping; 

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


 

Profile_Block* ProfilerReaderUtil::ReadBlock(FILE* stream)
{ 
	if (NULL == stream || feof(stream))
		return NULL;

	unsigned short blockType = 0;
	bool rs = false;
	rs = ReadUShort(stream, blockType);
	//回退两个字节
	fseek(stream,ftell(stream)-2,SEEK_SET); 
	if (!rs) return NULL;
	printf("%s\n", MonoProfilerFileBlockKindMap(blockType).c_str());
	Profile_Block* pBlock = ProfileBlockFactory(blockType);
	if (!pBlock)
	{
		SkipBlock(stream);
		return NULL;
	}
	
	pBlock->InitFromStream(stream); 

	return pBlock;
}

void ProfilerReaderUtil::SkipBlock(FILE* stream)
{
	unsigned short blockType = 0;
	unsigned int   blockSize = 0;
	unsigned int   blockCounterDelta = 0;

	ReadUShort(stream, blockType);
	ReadUInt(stream, blockSize);
	ReadUInt(stream, blockCounterDelta);
	fseek(stream, blockSize, SEEK_CUR);
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

Profile_Block* ProfileBlockFactory(unsigned int type)
{
	Profile_Block* pBlock = NULL;
	switch (type)
	{
	case MONO_PROFILER_FILE_BLOCK_KIND_INTRO: 
		break;
	case MONO_PROFILER_FILE_BLOCK_KIND_END: 
		break;
	case MONO_PROFILER_FILE_BLOCK_KIND_MAPPING: 
		pBlock = new Profile_Mapping;
		break;
	case MONO_PROFILER_FILE_BLOCK_KIND_LOADED: 
		break;
	case MONO_PROFILER_FILE_BLOCK_KIND_UNLOADED: 
		break;
	case MONO_PROFILER_FILE_BLOCK_KIND_EVENTS: 
		break;
	case MONO_PROFILER_FILE_BLOCK_KIND_STATISTICAL: 
		break;
	case MONO_PROFILER_FILE_BLOCK_KIND_HEAP_DATA:
		pBlock = new Profile_Heapshot_Data_Block;
		break;
	case MONO_PROFILER_FILE_BLOCK_KIND_HEAP_SUMMARY: 
		pBlock = new Profile_Heapshot_Summary;
		break;
	case MONO_PROFILER_FILE_BLOCK_KIND_DIRECTIVES: 
		break;
	default:
		break;
	}

	if (!pBlock)
	{
		pBlock = new Profile_Raw_Block;
	}
	return pBlock;
}

Profile_Heapshot_Summary::Profile_Heapshot_Summary():
Profile_Block(),
start_counter(0),
start_time(0),
collection(0),
mapping(NULL)
{ 
}

bool Profile_Heapshot_Summary::InitFromStream(FILE* stream)
{
	if (!Profile_Block::InitFromStream(stream))
		return false;

	VERIFY_STREAM(stream);
	ProfilerReaderUtil::ReadUInt64(stream, start_counter);
	ProfilerReaderUtil::ReadUInt64(stream, start_time);
	ProfilerReaderUtil::ReadUInt(stream, collection);

	while (1)
	{
		unsigned int class_id = 0;
		ProfilerReaderUtil::ReadUInt(stream, class_id);
		if (!class_id)
			break;
		Summary_Item item;
		item.class_id = class_id;
		ProfilerReaderUtil::ReadUInt(stream,item.reachable_insts);
		ProfilerReaderUtil::ReadUInt(stream, item.reachable_bytes);
		ProfilerReaderUtil::ReadUInt(stream, item.unreachable_insts);
		ProfilerReaderUtil::ReadUInt(stream, item.unreachable_bytes);
		items.push_back(item);
	}

	ProfilerReaderUtil::ReadUInt64(stream, end_counter);
	ProfilerReaderUtil::ReadUInt64(stream, end_time); 

	return true;

}



bool Profile_Block::InitFromStream(FILE* stream)
{
	VERIFY_STREAM(stream);
	ProfilerReaderUtil::ReadUShort(stream,code);
	ProfilerReaderUtil::ReadUInt(stream,size);
	ProfilerReaderUtil::ReadUInt(stream,counter_delta);
	return true;
}

bool Profile_Raw_Block::InitFromStream(FILE* stream)
{
	if (!Profile_Block::InitFromStream(stream))
		return false;

	VERIFY_STREAM(stream);
	if (pBuf)
	{
		delete[] pBuf;
		pBuf = NULL;
	}
	pBuf = new unsigned char[size];
	ProfilerReaderUtil::ReadBuffer(stream, pBuf, size);
	return true;

}

Profile_Raw_Block::~Profile_Raw_Block()
{
	if (pBuf)
	{
		delete[] pBuf;
		pBuf = NULL;
	}
}

Profile_Mapping::Profile_Mapping() :
Profile_Block(),
start_counter(0),
start_time(0),
end_counter(0),
end_time(0),
thread_id(0)
{ 
}

Profile_Mapping::~Profile_Mapping()
{ 
}

bool Profile_Mapping::InitFromStream(FILE* stream)
{
	if (!Profile_Block::InitFromStream(stream))
		return false;

	VERIFY_STREAM(stream);
	ProfilerReaderUtil::ReadUInt64(stream, start_counter);
	ProfilerReaderUtil::ReadUInt64(stream, start_time);
	ProfilerReaderUtil::ReadUInt64(stream, thread_id);

	while (1)
	{
		unsigned int class_id = 0;
		ProfilerReaderUtil::ReadUInt(stream, class_id);
		if (!class_id)
			break;
		Class_Info info; 
		info.class_id = class_id;
		ProfilerReaderUtil::ReadUInt(stream, info.assembly_id);
		ProfilerReaderUtil::ReadString(stream, info.class_name); 
		class_map.insert(std::make_pair(info.class_id, info));

		//注册全局类信息
		g_class_mapping.insert(std::make_pair(info.class_id, info));  
	}

	while (1)
	{
		unsigned int method_id = 0;
		ProfilerReaderUtil::ReadUInt(stream, method_id);
		if (!method_id)
			break;
		Method_Info info;
		info.method_id = method_id;
		ProfilerReaderUtil::ReadUInt(stream, info.class_id);
		ProfilerReaderUtil::ReadUInt(stream, info.wrapper_type);
		ProfilerReaderUtil::ReadString(stream, info.method_name);
		method_map.insert(std::make_pair(info.method_id, info));
	} 
	ProfilerReaderUtil::ReadUInt64(stream, end_counter);
	ProfilerReaderUtil::ReadUInt64(stream, end_time); 

	return true;
}



Profile_Heapshot_Data_Block::Profile_Heapshot_Data_Block()
{

}

Profile_Heapshot_Data_Block::~Profile_Heapshot_Data_Block()
{

}

bool Profile_Heapshot_Data_Block::InitFromStream(FILE* stream)
{
	if (!Profile_Block::InitFromStream(stream))
		return false;

	VERIFY_STREAM(stream);
	ProfilerReaderUtil::ReadUInt64(stream, job_start_counter);
	ProfilerReaderUtil::ReadUInt64(stream, job_start_time);
	ProfilerReaderUtil::ReadUInt64(stream, job_end_counter);
	ProfilerReaderUtil::ReadUInt64(stream, job_end_time);
	ProfilerReaderUtil::ReadUInt(stream, job_collection);

	ProfilerReaderUtil::ReadUInt64(stream, write_start_counter);
	ProfilerReaderUtil::ReadUInt64(stream, write_start_time);

	while (1)
	{
		unsigned int code = HEAP_CODE_NONE;
		ProfilerReaderUtil::ReadUInt(stream, code);
		if (0 == code)
			break;

		fseek(stream, ftell(stream) - 4, SEEK_SET);
		Profile_Object_Info obj_info;

		if(!obj_info.InitFromStream(stream))
		{
			printf("解析ObjectInfo意外失败！\n");
			break;
		}

		if (obj_info.code == HEAP_CODE_OBJECT)
		{
			objs.insert(std::make_pair(obj_info.obj, obj_info));
		}
	} 

	ProfilerReaderUtil::ReadUInt64(stream, write_end_counter);
	ProfilerReaderUtil::ReadUInt64(stream, write_end_time);

}

bool Profile_Heapshot_Data_Block::Profile_Object_Info::InitFromStream(FILE* stream)
{
	VERIFY_STREAM(stream);

	ProfilerReaderUtil::ReadUInt(stream, code);

	if (code == HEAP_CODE_OBJECT)
	{
		ProfilerReaderUtil::ReadUInt(stream, obj);
		ProfilerReaderUtil::ReadUInt(stream, class_id);
		ProfilerReaderUtil::ReadUInt(stream, size);
		ProfilerReaderUtil::ReadUInt(stream, ref_count);

		for (int i = 0; i < ref_count; i++)
		{
			unsigned int objRef = 0;
			ProfilerReaderUtil::ReadUInt(stream,objRef);
			refs.push_back(objRef);
		}
		return true;
	}
	else if (code == HEAP_CODE_FREE_OBJECT_CLASS){
		ProfilerReaderUtil::ReadUInt(stream, class_id);
		ProfilerReaderUtil::ReadUInt(stream, size);
		return true;
	}
	return false;
}

Profile_Heapshot_Data_Block::Profile_Object_Info::Profile_Object_Info()
:code(0),
obj(0),
class_id(0),
size(0),
ref_count(0)
{ 
}

Profile_Heapshot_Data_Block::Profile_Object_Info::~Profile_Object_Info()
{

}
