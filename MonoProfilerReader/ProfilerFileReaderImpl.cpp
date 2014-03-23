#include "ProfilerFileReaderImpl.h"
#include <stdio.h>
#include "ProfileReaderInterfaceImpl.h"

ProfilerLoggingFileReader::ProfilerLoggingFileReader()
{

}

ProfilerLoggingFileReader::~ProfilerLoggingFileReader()
{

}

IHeapShot* ProfilerLoggingFileReader::CreateHeapShotFromFile(const char* filename)
{
	IHeapShot* pHeapShot = NULL;
	FILE* pFile = NULL;
	fopen_s(&pFile,filename,"rb");
	if (pFile)
	{
		Profile_HeapShot_Data* pHeapShotData = new Profile_HeapShot_Data;
		pHeapShotData->InitFromStream(pFile);
		pHeapShot = pHeapShotData->MakeHeapShotData();
		fclose(pFile);

		delete pHeapShotData;
		pHeapShotData = NULL; 
	}
	return pHeapShot;
}


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
	size_t bytesRead = fread_s(buf, size, 1, size, stream);
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
	fseek(stream, ftell(stream) - 2, SEEK_SET);
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
		pBlock = new Profile_Mapping_Block;
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

Profile_Heapshot_Summary::Profile_Heapshot_Summary() :
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
		ProfilerReaderUtil::ReadUInt(stream, item.reachable_insts);
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
	ProfilerReaderUtil::ReadUShort(stream, code);
	ProfilerReaderUtil::ReadUInt(stream, size);
	ProfilerReaderUtil::ReadUInt(stream, counter_delta);
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

Profile_Mapping_Block::Profile_Mapping_Block() :
Profile_Block(),
start_counter(0),
start_time(0),
end_counter(0),
end_time(0),
thread_id(0)
{
}

Profile_Mapping_Block::~Profile_Mapping_Block()
{
}

bool Profile_Mapping_Block::InitFromStream(FILE* stream)
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

		if (!obj_info.InitFromStream(stream))
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

	return true;
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

		for (int i = 0; i < (int)(ref_count); i++)
		{
			unsigned int objRef = 0;
			ProfilerReaderUtil::ReadUInt(stream, objRef);
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

Profile_HeapShot_Data::Profile_HeapShot_Data()
{

}

Profile_HeapShot_Data::~Profile_HeapShot_Data()
{
	Clear();
}

void Profile_HeapShot_Data::InitFromStream(FILE* stream)
{
	Clear();

	Profile_Block* pBlock = NULL;
	while (pBlock = ProfilerReaderUtil::ReadBlock(stream))
	{
		m_blocks.push_back(pBlock);
	}

	//将所有映射文件中的类信息注册至类注册表
	auto itBlock = m_blocks.begin();
	while (itBlock != m_blocks.end())
	{
		if (MONO_PROFILER_FILE_BLOCK_KIND_MAPPING == (*itBlock)->code)
		{
			Profile_Mapping_Block* pMapBlock = (Profile_Mapping_Block*)(*itBlock);
			auto itClass = pMapBlock->class_map.begin();
			while (itClass != pMapBlock->class_map.end())
			{
				m_classMap.insert(std::make_pair(itClass->first, itClass->second));
				++itClass;
			}
		}
		++itBlock;
	}
}

IHeapShot* Profile_HeapShot_Data::MakeHeapShotData()
{
	HeapShotImpl* pHeapShot = new HeapShotImpl;
	if (pHeapShot)
	{
		auto itClass = m_classMap.begin();
		while (itClass != m_classMap.end())
		{
			ClassInfoImpl* pClass = new ClassInfoImpl;
			pClass->m_id = itClass->second.class_id;
			pClass->m_name = itClass->second.class_name;

			auto insertRs =
				pHeapShot->m_classMap.insert(std::make_pair(pClass->m_id, pClass));
			if (!insertRs.second)
			{
				delete pClass;
				pClass = NULL;
			}

			++itClass;
		}

		auto itBlock = m_blocks.begin();
		while (itBlock != m_blocks.end())
		{
			if (MONO_PROFILER_FILE_BLOCK_KIND_HEAP_DATA == (*itBlock)->code)
			{
				Profile_Heapshot_Data_Block* pHeapDataBlock = (Profile_Heapshot_Data_Block*)(*itBlock);
				HeapDataImpl* pHeapData = new HeapDataImpl;
				if (pHeapData)
				{
					auto itObj = pHeapDataBlock->objs.begin();
					while (itObj != pHeapDataBlock->objs.end())
					{
						if (HEAP_CODE_OBJECT == itObj->second.code)
						{
							ObjectInfoImpl* pObj = new ObjectInfoImpl;
							pObj->m_id = itObj->second.obj;
							pObj->m_classID = itObj->second.class_id;
							pObj->m_size = itObj->second.size;
							pObj->m_isReachable = true;
							pObj->m_refObjs = itObj->second.refs;

							pHeapData->m_objs.insert(std::make_pair(pObj->m_id, pObj));
						}

						++itObj;
					}//while (itObj != pHeapDataBlock->objs.end())  
					pHeapShot->m_heapDataList.push_back(pHeapData);
				}//if (pHeapData)
			}//if (MONO_PROFILER_FILE_BLOCK_KIND_HEAP_DATA == (*itBlock)->code)
			++itBlock;
		}//while (itBlock != m_blocks.end()) 
		return pHeapShot;
	}
	return NULL;
}

void Profile_HeapShot_Data::Clear()
{
	m_classMap.clear();
	auto itBlock = m_blocks.begin();
	while (itBlock != m_blocks.end())
	{
		delete *itBlock;
		*itBlock = NULL;
		++itBlock;
	}
	m_blocks.clear();
}


IProfilerFileReader* IProfilerFileReader::CreateInstance(const ProfilerFileReaderType type)
{
	return new ProfilerLoggingFileReader;
}
