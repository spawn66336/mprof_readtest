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

	//当前截面是否已经加载
	virtual bool IsLoaded(void) const = 0;
	//准备数据
	virtual void PrepareData(void) = 0;
	//清空载入的数据
	virtual void ReleaseData(void) = 0;
};

class IHeapShot
{
public:
	IHeapShot(){}
	virtual ~IHeapShot(){}

	//获取HeapShot文件名
	virtual const char* GetFileName() const = 0;

	virtual IHeapData* GetHeapDataByIndex( const unsigned int i ) const = 0;
	virtual unsigned int GetHeapDataCount() const = 0;
	virtual IClassInfo* GetClassInfoByID( const unsigned int id) const = 0;
	virtual unsigned int GetClassInfoCount() const = 0;
	 
	// 更新当期那HeapShot
	//   
	// {注意事项}
	// 此操作会更新HeapShot的类信息，若发现新的HeapData数据创建
	// 新的HeapData实例对象。
	virtual void Update(void) = 0;

	// 获取类列表
	//
	// {参数}
	// classes [in,out]: 获取类列表容器
	//  
	// {注意事项}
	// 返回的类列表中的类实例不可在外部删除，列表
	// 中实例的生存周期由HeapShot来管理
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
	//清空截面数据
	virtual void Clear() = 0;

	//更新HeapShot
	virtual void _UpdateHeapShot(IHeapShot* pHeapShot) = 0;
	//载入HeapData
	virtual void _LoadHeapData(IHeapData* pHeapData) = 0;
};
 
 
MPR_API  IProfilerHeapShotManager*  CreateProfilerHeapShotManager();
MPR_API void  DestroyProfilerHeapShotManager(IProfilerHeapShotManager* pMgr);

 