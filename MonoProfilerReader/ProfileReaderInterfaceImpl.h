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

	void Destroy(void);

	typedef std::map<unsigned int, IObjectInfo*> ObjectTable_t;
	ObjectTable_t m_objs;
	ObjectTable_t::iterator m_currObj;
};

class HeapShotImpl : public IHeapShot
{
public:
	HeapShotImpl();
	virtual ~HeapShotImpl();

	virtual IHeapData* GetHeapDataByIndex(const unsigned int i) const;
	virtual unsigned int GetHeapDataCount() const;
	virtual IClassInfo* GetClassInfoByID(const unsigned int id) const;
	virtual unsigned int GetClassInfoCount() const;

	void Destroy();

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
	
	void Destroy();

	typedef std::vector<IHeapShot*> HeapShotList_t;
	HeapShotList_t m_heapShotList; 
	IProfilerFileReader* m_pFileReader;

};