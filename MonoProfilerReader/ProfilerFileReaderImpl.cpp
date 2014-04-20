#include "ProfilerFileReaderImpl.h"
#include <stdio.h>
#include "ProfileReaderInterfaceImpl.h"
#include <windows.h>

ProfilerLoggingFileReader::ProfilerLoggingFileReader()
{

}

ProfilerLoggingFileReader::~ProfilerLoggingFileReader()
{

}

IHeapShot* ProfilerLoggingFileReader::CreateHeapShotFromFile(const char* filename)
{
	IHeapShot* pHeapShot = NULL;
	STREAM_HANDLE hFile = 0; 
	hFile = OpenReadStream(filename);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		Profile_HeapShot_Data* pHeapShotData = new Profile_HeapShot_Data;
		pHeapShotData->InitFromStream(hFile);
		pHeapShot = pHeapShotData->MakeHeapShotData();
		CloseStream(hFile);

		delete pHeapShotData;
		pHeapShotData = NULL; 
	}
	return pHeapShot;
}

unsigned int ProfilerLoggingFileReader::ParseHeapShotFromFile(
	const char* filename, 
	unsigned int offset, 
	std::vector<ClassParseInfo>& classes, 
	std::vector<HeapDataParseInfo>& heapDataInfos)
{
	STREAM_HANDLE hFile = 0;
	hFile = OpenReadStream(filename);

	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	StreamSeek(hFile, offset, FILE_BEGIN); 
	unsigned int newOffset = ProfilerReaderUtil::ParseHeapShot(hFile, classes, heapDataInfos);
	CloseStream(hFile);
	hFile = NULL;

	return newOffset;
}

bool ProfilerLoggingFileReader::LoadHeapData(
	const char* filename, 
	unsigned int offset, 
	HeapDataInfo& heapData)
{
	bool rs = false;
	STREAM_HANDLE hFile = 0;
	hFile = OpenReadStream(filename);

	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	StreamSeek(hFile, offset, FILE_BEGIN);
	 
	ProfilerLoggingBlockHeader header;
	if (ProfilerReaderUtil::ReadBlockHeader(hFile,header))
	{
		if (header.m_type == MONO_PROFILER_FILE_BLOCK_KIND_HEAP_DATA)
		{
			Profile_Heapshot_Data_Block* pHeapDataBlock = 
				(Profile_Heapshot_Data_Block*)ProfilerReaderUtil::ReadBlock(hFile);
			if (pHeapDataBlock)
			{
				auto itObj = pHeapDataBlock->objs.begin();
				while (itObj != pHeapDataBlock->objs.end())
				{
					ObjectInfo objInfo;
					objInfo.obj = (*itObj).second.obj;
					objInfo.class_id = (*itObj).second.class_id;
					objInfo.size = (*itObj).second.size;
					objInfo.refs = (*itObj).second.refs;
					heapData.objs.push_back(objInfo);
					++itObj;
				}

				//释放HeapDataBlock
				delete pHeapDataBlock;
				pHeapDataBlock = NULL;
				rs = true;
			}
		}
	} 
	CloseStream(hFile);
	hFile = NULL;
	return rs;
}


#define VERIFY_STREAM(s) do{\
if ( !s || s == INVALID_HANDLE_VALUE || IsEOF(s)) return false;\
}while (0)


bool ProfilerReaderUtil::ReadUShort(STREAM_HANDLE stream, unsigned short& val)
{
	VERIFY_STREAM(stream);
	DWORD bytes = 0;
	ReadStream(stream, &val, sizeof(val),bytes); 
	if (bytes != sizeof(val))
		return false;
	return true;
}

bool ProfilerReaderUtil::ReadUInt(STREAM_HANDLE stream, unsigned int& val)
{
	VERIFY_STREAM(stream);
	DWORD bytes = 0;
	ReadStream(stream, &val, sizeof(val), bytes);
	if (bytes != sizeof(val))
		return false;
	return true;
}

bool ProfilerReaderUtil::ReadUInt64(STREAM_HANDLE stream, unsigned __int64& val)
{
	VERIFY_STREAM(stream);
	DWORD bytes = 0;
	ReadStream(stream, &val, sizeof(val), bytes);
	if (bytes != sizeof(val))
		return false;
	return true;
}

bool ProfilerReaderUtil::ReadString(STREAM_HANDLE stream, std::string& val)
{
	VERIFY_STREAM(stream);
	val.clear();
	char c = 0;
	DWORD bytes = 0;
	while (!IsEOF(stream))
	{
		ReadStream(stream, &c, 1, bytes);
		if (0 == c)
		{
			break;
		}
		val.push_back(c);
	} 
	return true;
}


bool ProfilerReaderUtil::ReadBuffer(STREAM_HANDLE stream, void* buf, unsigned int size)
{
	VERIFY_STREAM(stream);
	DWORD bytes = 0;
	ReadStream(stream, buf, size, bytes);
	if (bytes != size)
		return false;
	return true;
}




Profile_Block* ProfilerReaderUtil::ReadBlock(STREAM_HANDLE stream)
{
	if (NULL == stream || INVALID_HANDLE_VALUE == stream || IsEOF(stream))
		return NULL;

	unsigned short blockType = 0; 
	ReadUShort(stream, blockType);  

	//若块类型不在范围内，则直接返回空
	if (blockType >= MONO_PROFILER_FILE_BLOCK_KIND_MAX)
		return NULL;
	

	//回退两个字节 
	StreamSeek(stream, -2, FILE_CURRENT); 
	printf("%s\n", MonoProfilerFileBlockKindMap(blockType).c_str());
	Profile_Block* pBlock = ProfileBlockFactory(blockType);
	if (!pBlock)
	{
		SkipBlock(stream);
		return NULL;
	}

	//从文件中创建失败则删除此块
	if (!pBlock->InitFromStream(stream))
	{
		delete pBlock;
		pBlock = NULL;
	}

	return pBlock;
}

void ProfilerReaderUtil::SkipBlock(STREAM_HANDLE stream)
{
	unsigned short blockType = 0;
	unsigned int   blockSize = 0;
	unsigned int   blockCounterDelta = 0; 

	ReadUShort(stream, blockType);
	ReadUInt(stream, blockSize);
	ReadUInt(stream, blockCounterDelta);
	StreamSeek(stream, blockSize, FILE_CURRENT);
}

bool ProfilerReaderUtil::ReadBlockHeader(STREAM_HANDLE stream, ProfilerLoggingBlockHeader& header)
{
	//记录在读取文件头前的文件偏移
	unsigned int prevOffset = StreamTell(stream);
	bool rs = true;
	rs &= ReadUShort(stream, header.m_type);
	rs &= ReadUInt(stream, header.m_size);
	rs &= ReadUInt(stream, header.m_counter_data);
	
	//若读取头失败
	if (!rs)
	{
		StreamSeek(stream, prevOffset, FILE_BEGIN);
		return false;
	}

	//若文件头类型非法
	if (header.m_type >= MONO_PROFILER_FILE_BLOCK_KIND_MAX)
	{
		StreamSeek(stream, prevOffset, FILE_BEGIN);
		return false;
	}

	//计算剩余空间
	DWORD surplusSize = StreamTotalSize(stream) - StreamTell(stream);

	//若当前块的后续内容大小，小于文件
	//剩余待读取区域大小则说明文件残缺
	//返回失败
	if (header.m_size > surplusSize)
	{
		StreamSeek(stream, prevOffset, FILE_BEGIN);
		return false;
	} 

	StreamSeek(stream, prevOffset, FILE_BEGIN);
	return true;
}


unsigned int ProfilerReaderUtil::ParseHeapShot(STREAM_HANDLE stream, std::vector<ClassParseInfo>& classes, std::vector<HeapDataParseInfo>& heapDataInfos)
{ 
	if (NULL == stream || INVALID_HANDLE_VALUE == stream )
		return 0;
	 
	if (IsEOF(stream))
	{
		return StreamTotalSize(stream);
	}

	unsigned int currOffset = StreamTell(stream);

	printf("currOffset = %d\n", currOffset);

	while (!IsEOF(stream))
	{  
		//在解析块前获取偏移
		currOffset = StreamTell(stream);

		ProfilerLoggingBlockHeader header;

		if (!ReadBlockHeader(stream, header))
		{//若块头解析失败或遇到非法块头则直接退出
			return currOffset;
		}

		if (header.m_type == MONO_PROFILER_FILE_BLOCK_KIND_MAPPING)
		{
			Profile_Mapping_Block* pMapBlock = (Profile_Mapping_Block*)ReadBlock(stream);

			//若Mapping块读取失败
			if (!pMapBlock)
			{
				return currOffset;
			}

			auto itClass = pMapBlock->class_map.begin();
			while (itClass != pMapBlock->class_map.end())
			{
				ClassParseInfo classInfo;
				classInfo.m_id = itClass->second.class_id;
				classInfo.m_name = itClass->second.class_name;
				classes.push_back(classInfo);
				++itClass;
			}
		}
		else if (header.m_type == MONO_PROFILER_FILE_BLOCK_KIND_HEAP_DATA){
			HeapDataParseInfo heapDataInfo;
			heapDataInfo.m_offset = StreamTell(stream);
			heapDataInfos.push_back(heapDataInfo);

			//略过此块
			SkipBlock(stream);
		}
		else{//非感兴趣的块直接略过
			SkipBlock(stream);
		} 
	}// while (!IsEOF(stream))

	currOffset = StreamTell(stream);
	return currOffset;
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

STREAM_HANDLE OpenReadStream(const char* filename)
{
	return CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

BOOL ReadStream(STREAM_HANDLE stream, void* pbuf, DWORD bytes2Read, DWORD& bytesRead)
{
	return ReadFile(stream, pbuf, bytes2Read, &bytesRead, NULL);
}

BOOL IsEOF(STREAM_HANDLE stream)
{
	char b = 0;
	DWORD bytesRead = 0;
	BOOL rs = ReadFile(stream,&b,1,&bytesRead , NULL);
	if (rs && bytesRead == 0)
	{//EOF
		return TRUE;
	}
	StreamSeek(stream, -1, FILE_CURRENT);
	return FALSE;
}

DWORD StreamSeek(STREAM_HANDLE stream, int offset, DWORD flag)
{
	return SetFilePointer(stream, offset, NULL, flag);
}


DWORD StreamTell(STREAM_HANDLE stream)
{
	return SetFilePointer(stream, 0, NULL, FILE_CURRENT);
}


BOOL CloseStream(STREAM_HANDLE stream)
{
	return CloseHandle(stream);
}

DWORD StreamTotalSize(STREAM_HANDLE stream)
{
	return GetFileSize(stream, NULL);
}

Profile_Heapshot_Summary::Profile_Heapshot_Summary() :
Profile_Block(),
start_counter(0),
start_time(0),
collection(0),
mapping(NULL)
{
}

bool Profile_Heapshot_Summary::InitFromStream(STREAM_HANDLE stream)
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



bool Profile_Block::InitFromStream(STREAM_HANDLE stream)
{
	VERIFY_STREAM(stream);
	ProfilerReaderUtil::ReadUShort(stream, code);
	ProfilerReaderUtil::ReadUInt(stream, size);
	ProfilerReaderUtil::ReadUInt(stream, counter_delta);
	
	//计算剩余空间
	DWORD surplusSize = StreamTotalSize(stream) - StreamTell(stream);

	//若当前块的后续内容大小，小于文件
	//剩余待读取区域大小则说明文件残缺
	//返回失败
	if (size > surplusSize)
	{
		return false;
	}

	return true;
}

bool Profile_Raw_Block::InitFromStream(STREAM_HANDLE stream)
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

bool Profile_Mapping_Block::InitFromStream(STREAM_HANDLE stream)
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

bool Profile_Heapshot_Data_Block::InitFromStream(STREAM_HANDLE stream)
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
		 
		StreamSeek(stream, -4, FILE_CURRENT);
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

bool Profile_Heapshot_Data_Block::Profile_Object_Info::InitFromStream(STREAM_HANDLE stream)
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

void Profile_HeapShot_Data::InitFromStream(STREAM_HANDLE stream)
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

			//插入重复的类信息
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
