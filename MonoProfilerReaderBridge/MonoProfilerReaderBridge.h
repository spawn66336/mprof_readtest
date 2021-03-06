// MonoProfilerReaderBridge.h

#pragma once
#include "ProfileReaderInterface.h"

using namespace System;
using namespace System::Collections;

namespace MonoProfilerReaderBridge {

	public ref class ClassInfo
	{
	public:
		ClassInfo(IClassInfo* pClassInfo){ m_pClassInfo = pClassInfo; }
		~ClassInfo();
		 unsigned int GetID();
		 System::String^ GetName();

	protected:
		IClassInfo* m_pClassInfo;
	};

	public ref class ObjectInfo
	{
	public:
		ObjectInfo(IObjectInfo* pObjInfo){ m_pObjectInfo = pObjInfo; }
		~ObjectInfo();

		 unsigned int GetID();
		 unsigned int GetClassID();
		 unsigned int GetSize();
		 unsigned int GetRefObjCount();
		 unsigned int GetRefObjIDByIndex(const unsigned int i);
		 bool IsReachable();

	protected:
		IObjectInfo* m_pObjectInfo;
	};

	public ref class HeapData
	{
	public:
		HeapData(IHeapData* pHeapData);
		~HeapData();

		 unsigned int GetObjectCount();
		 ObjectInfo^ GetObjectByID(const unsigned int id);

		 void MoveFirstObject();
		 void MoveNextObject();
		 ObjectInfo^ GetCurrObject();

		 bool IsLoaded();
		 void PrepareData();
		 void ReleaseData();

	protected:
		IHeapData* m_pHeapData;
		Hashtable^ m_objs;
		
	};

	public ref class HeapShot
	{
	public:
		 HeapShot(IHeapShot* pHeapShot);
		 ~HeapShot();

		 System::String^ GetFilePath();
		 HeapData^ GetHeapDataByIndex(const unsigned int i);
		 unsigned int GetHeapDataCount() ;
		 ClassInfo^ GetClassInfoByID(const unsigned int id);
		 unsigned int GetClassInfoCount() ;

		 void Update();
		 Hashtable^ GetClassTable(){ return m_classes;  }
	  
	protected:
		//更新类列表
		void _UpdateClassTable();
	protected:
		IHeapShot* m_pHeapShot;
		Hashtable^ m_heapdatas;
		Hashtable^ m_classes;
	};

	public ref class ProfilerHeapShotManager
	{
	public:
		ProfilerHeapShotManager();
		~ProfilerHeapShotManager();

		 HeapShot^ CreateHeapShotFromFile(System::String^ filename);
		 unsigned int GetHeapShotCount();
		 HeapShot^ GetHeapShotByIndex(const unsigned int i);
		 void Clear();
	protected:
		IProfilerHeapShotManager* m_pMgr;
		Hashtable^ m_heapshots;
	};

 
}
