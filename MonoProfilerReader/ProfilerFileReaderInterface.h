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
	std::string m_name; //����
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

		
	// ����HeapShot�ļ����ؽ������
	//
	// {����}
	// filename [in]: HeapShot�ļ���
	// offset [in]: ��HeapShot�ļ����Ŀ����
	// classes [out]: ���ص��������Ϣ����offset��ʼ�����������Ϣ��
	// heapDataInfos [out]: ���ص�HeapData������Ϣ����offset��ʼ����Ķѽ�����Ϣ��
	// 
	// {����ֵ}
	// ���ص�ǰ��ȡ�����ļ��ֽ�ƫ�� 
	virtual unsigned int ParseHeapShotFromFile(const char* filename, unsigned int offset,std::vector<ClassParseInfo>& classes , std::vector<HeapDataParseInfo>& heapDataInfos ) = 0;

	// ��ָ��λ�ô�����HeapData
	//
	// {����}
	// filename [in]:HeapShot�ļ���
	// offset [in]:��HeapShot�ļ����Ŀ����
	// heapdata [out]: ���صĶѽ���������Ϣ
	// 
	// {����ֵ}
	// ����ȡ�ɹ�����true,���򷵻�false�� 
	virtual bool LoadHeapData(const char* filename, unsigned int offset, HeapDataInfo& heapdata) = 0;

	static IProfilerFileReader* CreateInstance(const ProfilerFileReaderType type);
};

