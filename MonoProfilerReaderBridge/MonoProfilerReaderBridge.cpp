// 这是主 DLL 文件。

#include "stdafx.h"

#include "MonoProfilerReaderBridge.h"
#include <stdio.h>
#include <vector>
#include "Util.h"


MonoProfilerReaderBridge::ClassInfo::~ClassInfo()
{
	m_pClassInfo = NULL;
}

unsigned int MonoProfilerReaderBridge::ClassInfo::GetID()
{
	return m_pClassInfo->GetID();
}

System::String^ MonoProfilerReaderBridge::ClassInfo::GetName()
{
	return gcnew System::String(m_pClassInfo->GetName());
}


MonoProfilerReaderBridge::ObjectInfo::~ObjectInfo()
{
	m_pObjectInfo = NULL;
}


unsigned int MonoProfilerReaderBridge::ObjectInfo::GetID()
{
	return m_pObjectInfo->GetID();
}

unsigned int MonoProfilerReaderBridge::ObjectInfo::GetClassID()
{
	return m_pObjectInfo->GetClassID();
}

unsigned int MonoProfilerReaderBridge::ObjectInfo::GetSize()
{
	return m_pObjectInfo->GetSize();
}

unsigned int MonoProfilerReaderBridge::ObjectInfo::GetRefObjCount()
{
	return m_pObjectInfo->GetRefObjCount();
}

unsigned int MonoProfilerReaderBridge::ObjectInfo::GetRefObjIDByIndex(const unsigned int i)
{
	return m_pObjectInfo->GetRefObjIDByIndex(i);
}

bool MonoProfilerReaderBridge::ObjectInfo::IsReachable()
{
	return m_pObjectInfo->IsReachable();
}

unsigned int MonoProfilerReaderBridge::HeapData::GetObjectCount()
{
	return m_pHeapData->GetObjectCount();
}

MonoProfilerReaderBridge::ObjectInfo^ MonoProfilerReaderBridge::HeapData::GetObjectByID(const unsigned int id)
{
	IObjectInfo* pObj = NULL;
	if (pObj = m_pHeapData->GetObjectByID(id))
	{
		if(!m_objs->ContainsKey(id))
		{
			m_objs[id] = gcnew ObjectInfo(pObj);
		} 
		return (ObjectInfo^)m_objs[id];
	}
	return nullptr;
}


MonoProfilerReaderBridge::HeapData::HeapData(IHeapData* pHeapData)
{
	m_pHeapData = pHeapData;
	m_objs = gcnew Hashtable();

}

MonoProfilerReaderBridge::HeapData::~HeapData()
{
	m_pHeapData = NULL; 
	m_objs->Clear();
	m_objs = nullptr; 
}


void MonoProfilerReaderBridge::HeapData::MoveFirstObject()
{
	m_pHeapData->MoveFirstObject();
}

void MonoProfilerReaderBridge::HeapData::MoveNextObject()
{
	m_pHeapData->MoveNextObject();
}

MonoProfilerReaderBridge::ObjectInfo^ MonoProfilerReaderBridge::HeapData::GetCurrObject()
{
	IObjectInfo* pObj = m_pHeapData->GetCurrObject();
	if (pObj)
	{
		if (!m_objs->ContainsKey(pObj->GetID()))
		{
			m_objs[pObj->GetID()] = gcnew ObjectInfo(pObj);
		}
		return (ObjectInfo^)m_objs[pObj->GetID()];
	}
	return nullptr;
}

bool MonoProfilerReaderBridge::HeapData::IsLoaded()
{
	return m_pHeapData->IsLoaded();
}

void MonoProfilerReaderBridge::HeapData::PrepareData()
{
	m_pHeapData->PrepareData();
}

void MonoProfilerReaderBridge::HeapData::ReleaseData()
{
	m_pHeapData->ReleaseData();
}


MonoProfilerReaderBridge::HeapShot::HeapShot(IHeapShot* pHeapShot)
{
	m_pHeapShot = pHeapShot;

	m_heapdatas = gcnew Hashtable();
	m_classes = gcnew Hashtable();

	//更新类列表
	_UpdateClassTable();
}

MonoProfilerReaderBridge::HeapShot::~HeapShot()
{
	m_pHeapShot = NULL;
 
	m_heapdatas->Clear();
	m_heapdatas = nullptr;

 
	m_classes->Clear();
	m_classes = nullptr;
}


System::String^ MonoProfilerReaderBridge::HeapShot::GetFilePath()
{
	return gcnew System::String(m_pHeapShot->GetFileName());
}


MonoProfilerReaderBridge::HeapData^ MonoProfilerReaderBridge::HeapShot::GetHeapDataByIndex(const unsigned int i)
{
	IHeapData* pHeapData = m_pHeapShot->GetHeapDataByIndex(i);
	if (pHeapData)
	{
		unsigned int key = *((unsigned int*)(&pHeapData));
		if (!m_heapdatas->ContainsKey(key))
		{
			m_heapdatas[key] = gcnew HeapData(pHeapData);
		}
		return (HeapData^)m_heapdatas[key];
	}
	return nullptr;
}

unsigned int MonoProfilerReaderBridge::HeapShot::GetHeapDataCount()
{
	return m_pHeapShot->GetHeapDataCount();
}

MonoProfilerReaderBridge::ClassInfo^ MonoProfilerReaderBridge::HeapShot::GetClassInfoByID(const unsigned int id)
{
	IClassInfo* pClass = m_pHeapShot->GetClassInfoByID(id);
	if (pClass)
	{
		if (!m_classes->ContainsKey(pClass->GetID()))
		{
			m_classes[pClass->GetID()] = gcnew ClassInfo(pClass);
		}
		return (ClassInfo^)m_classes[pClass->GetID()];
	}
	return nullptr;
}

void MonoProfilerReaderBridge::HeapShot::_UpdateClassTable()
{
	std::vector<IClassInfo*> classList;
	m_pHeapShot->GetClassList(classList);

	auto itClass = classList.begin();
	while (itClass != classList.end())
	{
		IClassInfo* pClass = *itClass;
		if (!m_classes->ContainsKey(pClass->GetID()))
		{
			m_classes[pClass->GetID()] = gcnew ClassInfo(pClass);
		}
		++itClass;
	}
}

unsigned int MonoProfilerReaderBridge::HeapShot::GetClassInfoCount()
{
	return m_pHeapShot->GetClassInfoCount();
}

void MonoProfilerReaderBridge::HeapShot::Update()
{
	m_pHeapShot->Update(); 
	_UpdateClassTable();
}



 


MonoProfilerReaderBridge::ProfilerHeapShotManager::ProfilerHeapShotManager()
{
	m_pMgr = CreateProfilerHeapShotManager();
	m_heapshots = gcnew Hashtable();

}

MonoProfilerReaderBridge::ProfilerHeapShotManager::~ProfilerHeapShotManager()
{
	Clear();
	DestroyProfilerHeapShotManager(m_pMgr);
	m_heapshots = nullptr;
	m_pMgr = NULL;
}

MonoProfilerReaderBridge::HeapShot^ MonoProfilerReaderBridge::ProfilerHeapShotManager::CreateHeapShotFromFile(System::String^ filename)
{
	UMConverter con_filename(StringItem(filename).getBuf());
	IHeapShot* pHeapShot = m_pMgr->CreateHeapShotFromFile(con_filename.GetMBCS());
	if (pHeapShot)
	{
		unsigned int key = *((unsigned int*)(&pHeapShot));
		if (!m_heapshots->ContainsKey(key))
		{
			m_heapshots[key] = gcnew HeapShot(pHeapShot); 
		}
		return (HeapShot^)m_heapshots[key];
	}
	return nullptr;
}

unsigned int MonoProfilerReaderBridge::ProfilerHeapShotManager::GetHeapShotCount()
{
	return m_pMgr->GetHeapShotCount();
}

void MonoProfilerReaderBridge::ProfilerHeapShotManager::Clear()
{ 
 
	m_heapshots->Clear();
	m_pMgr->Clear();
}

MonoProfilerReaderBridge::HeapShot^ MonoProfilerReaderBridge::ProfilerHeapShotManager::GetHeapShotByIndex(const unsigned int i)
{
	IHeapShot* pHeapShot = m_pMgr->GetHeapShotByIndex(i);
	if (pHeapShot)
	{
		unsigned int key = *((unsigned int*)(&pHeapShot));
		if (m_heapshots->ContainsKey(key))
		{
			return (HeapShot^)m_heapshots[key];
		}
	}
	return nullptr;
}



 
