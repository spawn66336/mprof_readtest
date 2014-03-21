#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <stack>

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

typedef enum {
	HEAP_CODE_NONE = 0,
	HEAP_CODE_OBJECT = 1,
	HEAP_CODE_FREE_OBJECT_CLASS = 2,
	HEAP_CODE_MASK = 3
} HeapProfilerJobValueCode;

class Profile_Block
{
public:
	Profile_Block() :
		code(0), 
		size(0), 
		counter_delta(0)
	{

	}
	unsigned short code;
	unsigned int size;
	unsigned int counter_delta;

	virtual bool InitFromStream(FILE* stream);
};

class Profile_Raw_Block : public Profile_Block
{
public:
	Profile_Raw_Block():
		Profile_Block(), pBuf(0){}
	~Profile_Raw_Block();

	virtual bool InitFromStream(FILE* stream);
	
	unsigned char* pBuf;
};



class Profile_Mapping : public Profile_Block
{
public:
	typedef struct{
		unsigned int class_id;
		unsigned int assembly_id;
		std::string  class_name;
	}Class_Info;

	typedef struct{
		unsigned int method_id;
		unsigned int class_id;
		unsigned int wrapper_type;
		std::string  method_name;
	}Method_Info;

	Profile_Mapping();
	~Profile_Mapping();

	virtual bool InitFromStream(FILE* stream);
	 
	unsigned __int64 start_counter;
	unsigned __int64 start_time;
	unsigned __int64 end_counter;
	unsigned __int64 end_time; 
	unsigned __int64 thread_id;
	std::map<unsigned int, Class_Info> class_map;
	std::map<unsigned int, Method_Info> method_map; 
};

class Profile_Heapshot_Summary : public Profile_Block
{
public:

	typedef struct{
		unsigned int class_id;
		unsigned int reachable_insts;
		unsigned int reachable_bytes;
		unsigned int unreachable_insts;
		unsigned int unreachable_bytes;
	}Summary_Item;

	Profile_Heapshot_Summary();
	~Profile_Heapshot_Summary(){}

	virtual bool InitFromStream(FILE* stream);

	unsigned __int64 start_counter;
	unsigned __int64 start_time;
	unsigned __int64 end_counter;
	unsigned __int64 end_time;
	unsigned int	 collection;
	std::vector<Summary_Item> items;
	Profile_Mapping* mapping;

};

class Profile_Heapshot_Data_Block : public Profile_Block
{
public:

	class Profile_Object_Info
	{
	public:
		Profile_Object_Info();
		~Profile_Object_Info();

		bool InitFromStream(FILE* stream);

		unsigned int code;
		unsigned int obj;
		unsigned int class_id;
		unsigned int size;
		unsigned int ref_count;
		std::vector<unsigned int> refs;
	};

	Profile_Heapshot_Data_Block();
	~Profile_Heapshot_Data_Block();

	virtual bool InitFromStream(FILE* stream);


	unsigned __int64 job_start_counter;
	unsigned __int64 job_start_time;
	unsigned __int64 job_end_counter;
	unsigned __int64 job_end_time;
	unsigned int	 job_collection;

	unsigned __int64 write_start_counter;
	unsigned __int64 write_start_time;
	unsigned __int64 write_end_counter;
	unsigned __int64 write_end_time;

	std::map<unsigned int, Profile_Object_Info> objs;

};


Profile_Block* ProfileBlockFactory(unsigned int type);
 
extern std::map<unsigned int, Profile_Mapping::Class_Info> g_class_mapping; 

class ProfilerReaderUtil
{
public: 

	static bool ReadUShort(FILE* stream, unsigned short& val);
	static bool ReadUInt(FILE* stream, unsigned int& val);
	static bool ReadUInt64(FILE* stream, unsigned __int64& val);
	static bool ReadString(FILE* stream, std::string& val);
	static bool ReadBuffer(FILE* stream, void* buf, unsigned int size);
	static Profile_Block* ReadBlock(FILE* stream);

private:
	static void SkipBlock(FILE* stream);
	
};

std::string MonoProfilerFileBlockKindMap(unsigned int code);
