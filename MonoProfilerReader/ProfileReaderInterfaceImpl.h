#pragma once
#include "ProfileReaderInterface.h"
#include <string>
#include <map>
#include <vector>
#include "ProfilerFileReaderInterface.h"

class ClassInfoImpl : public IClassInfo
{
public:
	ClassInfoImpl();
	virtual ~ClassInfoImpl();

	virtual unsigned int GetID()const;
	virtual const char* GetName() const;
	

	unsigned int m_id;
	std::string  m_name;
};

class ObjectInfoImpl : public IObjectInfo
{
public:
	ObjectInfoImpl();
	virtual ~ObjectInfoImpl();

	virtual unsigned int GetID() const;
	virtual unsigned int GetClassID()const;
	virtual unsigned int GetSize()const;
	virtual unsigned int GetRefObjCount()const;
	virtual unsigned int GetRefObjIDByIndex(const unsigned int i) const;
	virtual bool IsReachable()const;

	unsigned int m_id;
	unsigned int m_classID;
	unsigned int m_size;
	bool m_isReachable;
	std::vector<unsigned int> m_refObjs;
};

class HeapDataImpl : public IHeapData
{
public:
	HeapDataImpl();
	virtual ~HeapDataImpl();

	virtual unsigned int GetObjectCount() const; 
	virtual IObjectInfo* GetObjectByID(const unsigned int id) const;

	virtual void MoveFirstObject();
	virtual void MoveNextObject();
	virtual IObjectInfo* GetCurrObject();

	virtual bool IsLoaded(void) const;
	virtual void PrepareData();
	virtual void ReleaseData();

	void Destroy(void);

	typedef std::map<unsigned int, IObjectInfo*> ObjectTable_t;


	IProfilerHeapShotManager* m_pMgr;
	IHeapShot* m_pParent;   //指向所属HeapShot
	unsigned int m_offset; //当前HeapData在文件中的偏移
	bool		 m_loaded; //当前截面数据是否已经载入 
	ObjectTable_t m_objs;  //对象表
	ObjectTable_t::iterator m_currObj;
};

class HeapShotImpl : public IHeapShot
{
public:
	HeapShotImpl();
	virtual ~HeapShotImpl();

	virtual const char* GetFileName() const;

	virtual IHeapData* GetHeapDataByIndex(const unsigned int i) const;
	virtual unsigned int GetHeapDataCount() const;
	virtual IClassInfo* GetClassInfoByID(const unsigned int id) const;
	virtual unsigned int GetClassInfoCount() const;

	virtual void Update(void); 
	virtual void GetClassList(std::vector<IClassInfo*>& classes); 
	void Destroy();

	IProfilerHeapShotManager* m_pMgr;

	std::string m_filename; //HeapShot所属文件名
	unsigned int m_offset;  //最后一次更新HeapShot的文件偏移
	 
	typedef std::vector<IHeapData*> HeapDataList_t;
	HeapDataList_t m_heapDataList;

	typedef std::map<unsigned int, IClassInfo*> ClassInfoTable_t;
	ClassInfoTable_t m_classMap;

};

class ProfilerHeapShotManager : public IProfilerHeapShotManager
{
public:
	ProfilerHeapShotManager();
	virtual ~ProfilerHeapShotManager();

	virtual IHeapShot* CreateHeapShotFromFile(const char* filename);
	virtual unsigned int GetHeapShotCount() const;
	virtual IHeapShot* GetHeapShotByIndex( const unsigned int i) const;
	virtual void Clear();
	
	virtual void _UpdateHeapShot(IHeapShot* pHeapShot);
	virtual void _LoadHeapData(IHeapData* pHeapData);
	void Destroy();

	typedef std::vector<IHeapShot*> HeapShotList_t;
	HeapShotList_t m_heapShotList; 
	IProfilerFileReader* m_pFileReader;

};