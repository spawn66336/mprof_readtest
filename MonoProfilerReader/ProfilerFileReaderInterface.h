#pragma once

#include <string>
#include <vector>

class IHeapShot;


typedef enum{
	PROFILER_FILE_LOGGING_READER = 1
}ProfilerFileReaderType;


class HeapDataParseInfo
{
public:
	HeapDataParseInfo() :m_offset(0){}
	~HeapDataParseInfo(){}

	unsigned int m_offset;
};

class ClassParseInfo
{
public:
	ClassParseInfo() :m_id(0){}
	~ClassParseInfo(){}

	unsigned int m_id;	//id
	std::string m_name; //类名
};

class ObjectInfo
{
public:
	ObjectInfo():obj(0),class_id(0),size(0){}
	~ObjectInfo(){} 

	unsigned int obj;
	unsigned int class_id;
	unsigned int size;
	std::vector<unsigned int> refs;
};

class HeapDataInfo
{
public:
	HeapDataInfo(){}
	~HeapDataInfo(){}

	std::vector<ObjectInfo> objs;
};


class IProfilerFileReader
{
public:
	IProfilerFileReader(){}
	virtual ~IProfilerFileReader(){}

	virtual IHeapShot* CreateHeapShotFromFile(const char* filename) = 0;  

		
	// 解析HeapShot文件返回解析结果
	//
	// {参数}
	// filename [in]: HeapShot文件名
	// offset [in]: 从HeapShot文件的哪块解析
	// classes [out]: 返回的类解析信息（从offset开始处往后的类信息）
	// heapDataInfos [out]: 返回的HeapData解析信息（从offset开始往后的堆截面信息）
	// 
	// {返回值}
	// 返回当前读取到的文件字节偏移 
	virtual unsigned int ParseHeapShotFromFile(const char* filename, unsigned int offset,std::vector<ClassParseInfo>& classes , std::vector<HeapDataParseInfo>& heapDataInfos ) = 0;

	// 从指定位置处解析HeapData
	//
	// {参数}
	// filename [in]:HeapShot文件名
	// offset [in]:从HeapShot文件的哪块加载
	// heapdata [out]: 返回的堆截面数据信息
	// 
	// {返回值}
	// 若读取成功返回true,否则返回false。 
	virtual bool LoadHeapData(const char* filename, unsigned int offset, HeapDataInfo& heapdata) = 0;

	static IProfilerFileReader* CreateInstance(const ProfilerFileReaderType type);
};

