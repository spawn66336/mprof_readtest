#include "ProfileReaderInterfaceImpl.h"

ClassInfoImpl::ClassInfoImpl():
m_id(0)
{

}

ClassInfoImpl::~ClassInfoImpl()
{
}

unsigned int ClassInfoImpl::GetID() const
{
	return m_id;
}

const char* ClassInfoImpl::GetName() const
{
	return m_name.c_str();
}

ObjectInfoImpl::ObjectInfoImpl():
m_id(0),
m_classID(0),
m_size(0),
m_isReachable(false)
{

}

ObjectInfoImpl::~ObjectInfoImpl()
{

}

unsigned int ObjectInfoImpl::GetID() const
{
	return m_id;
}

unsigned int ObjectInfoImpl::GetClassID() const
{
	return m_classID;
}

unsigned int ObjectInfoImpl::GetSize() const
{
	return m_size;
}

unsigned int ObjectInfoImpl::GetRefObjCount() const
{
	return m_refObjs.size();
}

unsigned int ObjectInfoImpl::GetRefObjIDByIndex(const unsigned int i) const
{
	if (i >= m_refObjs.size())
	{
		return 0;
	}
	return m_refObjs.at(i);
}

bool ObjectInfoImpl::IsReachable() const
{
	return m_isReachable;
}

HeapDataImpl::HeapDataImpl()
{

}

HeapDataImpl::~HeapDataImpl()
{
	this->Destroy();
}

unsigned int HeapDataImpl::GetObjectCount() const
{
	return m_objs.size();
}

 

IObjectInfo* HeapDataImpl::GetObjectByID( const unsigned int id ) const
{
	auto findObj = m_objs.find(id);
	if (findObj != m_objs.end())
	{
		return findObj->second;
	}
	return NULL;
}

void HeapDataImpl::MoveFirstObject()
{
	m_currObj = m_objs.begin();
}

void HeapDataImpl::MoveNextObject()
{
	if (m_currObj != m_objs.end())
	{
		++m_currObj;
	}
}

IObjectInfo* HeapDataImpl::GetCurrObject()
{
	if (m_currObj != m_objs.end())
	{
		return m_currObj->second;
	}
	return NULL;
}

void HeapDataImpl::Destroy(void)
{
	auto itObj = m_objs.begin();
	while (itObj != m_objs.end())
	{
		delete itObj->second;
		itObj->second = NULL;
		++itObj;
	}
	m_objs.clear();
}

HeapShotImpl::HeapShotImpl()
{

}

HeapShotImpl::~HeapShotImpl()
{
	this->Destroy();
}

IHeapData* HeapShotImpl::GetHeapDataByIndex(const unsigned int i) const
{
	if(i >= m_heapDataList.size())
	{
		return NULL;
	}
	return m_heapDataList.at(i);
}

unsigned int HeapShotImpl::GetHeapDataCount() const
{
	return m_heapDataList.size();
}

IClassInfo* HeapShotImpl::GetClassInfoByID(const unsigned int id) const
{
	auto itClass = m_classMap.find(id);
	if (itClass != m_classMap.end())
	{
		return itClass->second;
	}
	return NULL;
}

unsigned int HeapShotImpl::GetClassInfoCount() const
{
	return m_classMap.size();
}

void HeapShotImpl::Destroy()
{
	auto itHeapData = m_heapDataList.begin();
	while (itHeapData != m_heapDataList.end())
	{
		delete *itHeapData;
		*itHeapData = NULL;
		++itHeapData;
	}
	m_heapDataList.clear();

	auto itClass = m_classMap.begin();
	while (itClass != m_classMap.end())
	{
		delete itClass->second;
		itClass->second = NULL;
		++itClass;
	}
	m_classMap.clear();
}

ProfilerHeapShotManager::ProfilerHeapShotManager():
m_pFileReader(NULL)
{
	m_pFileReader = IProfilerFileReader::CreateInstance(PROFILER_FILE_LOGGING_READER);
}

ProfilerHeapShotManager::~ProfilerHeapShotManager()
{
	this->Destroy();
}

IHeapShot* ProfilerHeapShotManager::CreateHeapShotFromFile(const char* filename)
{
	IHeapShot* pNewHeapShot = m_pFileReader->CreateHeapShotFromFile(filename);
	
	if (pNewHeapShot)
	{
		m_heapShotList.push_back(pNewHeapShot);
		return pNewHeapShot;
	}
	else{
		printf("无法打开文件：%s\n", filename);
	}
	return NULL;
}

unsigned int ProfilerHeapShotManager::GetHeapShotCount() const
{
	return m_heapShotList.size();
}

IHeapShot* ProfilerHeapShotManager::GetHeapShotByIndex(const unsigned int i) const
{
	if (i >= m_heapShotList.size())
	{
		return NULL;
	}
	return m_heapShotList.at(i);
}

void ProfilerHeapShotManager::Destroy()
{
	if (m_pFileReader)
	{
		delete m_pFileReader;
		m_pFileReader = NULL;
	}

	auto itHeapShot = m_heapShotList.begin();
	while (itHeapShot != m_heapShotList.end())
	{
		delete *itHeapShot;
		*itHeapShot = NULL;
		++itHeapShot;
	}
	m_heapShotList.clear();
}


IProfilerHeapShotManager* CreateProfilerHeapShotManager()
{
	return new ProfilerHeapShotManager;
}

void DestroyProfilerHeapShotManager(IProfilerHeapShotManager* pMgr)
{
	if (pMgr)
	{
		delete pMgr;
	}
}