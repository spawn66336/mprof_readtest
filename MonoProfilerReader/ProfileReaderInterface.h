#pragma once

#define MPR_API  __declspec(dllexport)
 
#include <vector>

class IClassInfo
{
public:
	IClassInfo(){}
	virtual ~IClassInfo(){}

	virtual unsigned int GetID()const = 0;
	virtual const char* GetName() const = 0;
};

class IObjectInfo
{
public:
	IObjectInfo(){}
	virtual ~IObjectInfo(){}

	virtual unsigned int GetID() const = 0;
	virtual unsigned int GetClassID()const = 0;
	virtual unsigned int GetSize()const = 0;
	virtual unsigned int GetRefObjCount()const = 0;
	virtual unsigned int GetRefObjIDByIndex( const unsigned int i) const = 0;
	virtual bool IsReachable()const = 0;
};

class IHeapData
{
public:
	IHeapData(){}
	virtual ~IHeapData(){}

	virtual unsigned int GetObjectCount() const = 0; 
	virtual IObjectInfo* GetObjectByID( const unsigned int id ) const = 0;

	virtual void MoveFirstObject() = 0;
	virtual void MoveNextObject() = 0;
	virtual IObjectInfo* GetCurrObject() = 0;

	//��ǰ�����Ƿ��Ѿ�����
	virtual bool IsLoaded(void) const = 0;
	//׼������
	virtual void PrepareData(void) = 0;
	//������������
	virtual void ReleaseData(void) = 0;
};

class IHeapShot
{
public:
	IHeapShot(){}
	virtual ~IHeapShot(){}

	//��ȡHeapShot�ļ���
	virtual const char* GetFileName() const = 0;

	virtual IHeapData* GetHeapDataByIndex( const unsigned int i ) const = 0;
	virtual unsigned int GetHeapDataCount() const = 0;
	virtual IClassInfo* GetClassInfoByID( const unsigned int id) const = 0;
	virtual unsigned int GetClassInfoCount() const = 0;
	 
	// ���µ�����HeapShot
	//   
	// {ע������}
	// �˲��������HeapShot������Ϣ���������µ�HeapData���ݴ���
	// �µ�HeapDataʵ������
	virtual void Update(void) = 0;

	// ��ȡ���б�
	//
	// {����}
	// classes [in,out]: ��ȡ���б�����
	//  
	// {ע������}
	// ���ص����б��е���ʵ���������ⲿɾ�����б�
	// ��ʵ��������������HeapShot������
	// 
	virtual void GetClassList(std::vector<IClassInfo*>& classes) = 0;
	
};

class IProfilerHeapShotManager
{
public:
	IProfilerHeapShotManager(){}
	virtual ~IProfilerHeapShotManager(){}

	virtual IHeapShot* CreateHeapShotFromFile(const char* filename) = 0;
	virtual unsigned int GetHeapShotCount() const = 0;
	virtual IHeapShot* GetHeapShotByIndex(const unsigned int i) const = 0;
	//��ս�������
	virtual void Clear() = 0;

	//����HeapShot
	virtual void _UpdateHeapShot(IHeapShot* pHeapShot) = 0;
	//����HeapData
	virtual void _LoadHeapData(IHeapData* pHeapData) = 0;
};
 
 
MPR_API  IProfilerHeapShotManager*  CreateProfilerHeapShotManager();
MPR_API void  DestroyProfilerHeapShotManager(IProfilerHeapShotManager* pMgr);

 