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
:m_pMgr(0),
 m_pParent(0),
 m_offset(0),
 m_loaded(false)
{

}

HeapDataImpl::~HeapDataImpl()
{
	m_pMgr = NULL;
	m_pParent = NULL;
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
	ReleaseData();
}

bool HeapDataImpl::IsLoaded(void) const
{
	return m_loaded;
}

void HeapDataImpl::PrepareData()
{
	if (m_pMgr)
	{
		m_pMgr->_LoadHeapData(this);
		m_loaded = true;
	} 
}

void HeapDataImpl::ReleaseData()
{
	auto itObj = m_objs.begin();
	while (itObj != m_objs.end())
	{
		delete itObj->second;
		itObj->second = NULL;
		++itObj;
	}
	m_objs.clear();
	m_loaded = false;
}

HeapShotImpl::HeapShotImpl() :
m_pMgr(0),
m_offset(0)
{
	
}

HeapShotImpl::~HeapShotImpl()
{
	this->Destroy();
}


const char* HeapShotImpl::GetFileName() const
{
	return m_filename.c_str();
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

void HeapShotImpl::GetClassList(std::vector<IClassInfo*>& classes)
{
	classes.clear();
	auto itClass = m_classMap.begin();
	while (itClass != m_classMap.end())
	{
		classes.push_back((*itClass).second);
		++itClass;
	}
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

void HeapShotImpl::Update()
{
	if (m_pMgr)
	{
		m_pMgr->_UpdateHeapShot(this);
	}
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
	HeapShotImpl* pNewHeapShot = new HeapShotImpl; 
	if (pNewHeapShot)
	{
		pNewHeapShot->m_pMgr = this;
		pNewHeapShot->m_filename = filename;
		pNewHeapShot->m_offset = 0;
		pNewHeapShot->Update(); 

		m_heapShotList.push_back(pNewHeapShot);
		return pNewHeapShot;
	}else{
		printf("创建HeapShot对象失败！");
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

void ProfilerHeapShotManager::Clear()
{
	auto itHeapShot = m_heapShotList.begin();
	while (itHeapShot != m_heapShotList.end())
	{
		delete *itHeapShot;
		*itHeapShot = NULL;
		++itHeapShot;
	}
	m_heapShotList.clear();
}

void ProfilerHeapShotManager::Destroy()
{
	//清空所有内存截面数据
	Clear();

	if (m_pFileReader)
	{
		delete m_pFileReader;
		m_pFileReader = NULL;
	}

}

void ProfilerHeapShotManager::_UpdateHeapShot(IHeapShot* pHeapShot)
{
	if (NULL == pHeapShot)
		return;

	HeapShotImpl* pHeapShotImpl = (HeapShotImpl*)pHeapShot;
	
	std::vector<ClassParseInfo> classes;
	std::vector<HeapDataParseInfo> heapDataInfos;

	pHeapShotImpl->m_offset =  m_pFileReader->ParseHeapShotFromFile(
		pHeapShotImpl->m_filename.c_str(), 
		pHeapShotImpl->m_offset,
		classes,heapDataInfos);

	//记录类信息
	auto itClass = classes.begin();
	while (itClass != classes.end())
	{
		ClassInfoImpl* pNewClass = new ClassInfoImpl;
		pNewClass->m_id = itClass->m_id;
		pNewClass->m_name = itClass->m_name;
		pHeapShotImpl->m_classMap.insert(std::make_pair(pNewClass->m_id, pNewClass));
		++itClass;
	}

	//只生成新的堆截面对象，并将偏移信息记录在对象中
	//并不实际读取数据，待后续实际使用堆截面数据时再
	//载入
	auto itHeapDataInfo = heapDataInfos.begin();
	while (itHeapDataInfo != heapDataInfos.end())
	{
		HeapDataImpl* pNewHeapData = new HeapDataImpl;

		pNewHeapData->m_pMgr = this;
		pNewHeapData->m_pParent = pHeapShot;
		pNewHeapData->m_loaded = false;
		pNewHeapData->m_offset = itHeapDataInfo->m_offset;

		pHeapShotImpl->m_heapDataList.push_back(pNewHeapData);
		++itHeapDataInfo;
	}

}

void ProfilerHeapShotManager::_LoadHeapData(IHeapData* pHeapData)
{
	if (NULL == pHeapData)
		return;

	HeapDataImpl* pHeapDataImpl = (HeapDataImpl*)pHeapData;

	//若当前堆截面数据已经加载，则直接返回
	if (pHeapDataImpl->m_loaded)
		return;
	
	//加载截面对象数据
	HeapDataInfo heapDataInfo;
	if (m_pFileReader->LoadHeapData(
		pHeapDataImpl->m_pParent->GetFileName(), 
		pHeapDataImpl->m_offset, heapDataInfo))
	{
		auto itObj = heapDataInfo.objs.begin();
		while (itObj != heapDataInfo.objs.end())
		{
			ObjectInfoImpl* pNewObj = new ObjectInfoImpl;
			pNewObj->m_classID = itObj->class_id;
			pNewObj->m_id = itObj->obj;
			pNewObj->m_size = itObj->size;
			pNewObj->m_refObjs = itObj->refs;
			pNewObj->m_isReachable = true;

			pHeapDataImpl->m_objs.insert(std::make_pair(pNewObj->m_id ,pNewObj));
			++itObj;
		}
	}
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