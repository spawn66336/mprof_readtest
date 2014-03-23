#include <stdio.h>
#include <string>
#include <iostream> 
#include <vector>
#include "ProfileReaderInterface.h"

using namespace std;
 

void OutputHeapShotToFile(const std::string& filename, IHeapShot* pHeapShot)
{
	FILE* pOutFile = NULL;
	fopen_s(&pOutFile, filename.c_str(), "wb");
	if (pOutFile)
	{
		for (int i = 0; i < pHeapShot->GetHeapDataCount(); i++)
		{
			IHeapData* pHeapData = pHeapShot->GetHeapDataByIndex(i);

			fprintf(pOutFile, "************************HeapData******************\n");

			pHeapData->MoveFirstObject();
			IObjectInfo* pObj = NULL;
			while (pObj = pHeapData->GetCurrObject())
			{
				IClassInfo* pClass =
				pHeapShot->GetClassInfoByID(pObj->GetClassID());
				
				fprintf(pOutFile, "%-50s\t\t[%p] refs=%d, size=%d\n" ,pClass->GetName(), pObj->GetID() ,pObj->GetRefObjCount(), pObj->GetSize());

				for (int j = 0; j < pObj->GetRefObjCount(); j++)
				{
					fprintf(pOutFile,"\t\t\t[%p]\n", pObj->GetRefObjIDByIndex(j));
				}
				pHeapData->MoveNextObject();
			} 
			fprintf(pOutFile, "****************************************************\n");
		}
		fclose(pOutFile);
	}
}
 
void main()
{
	IProfilerHeapShotManager* pHeapShotMgr =  CreateProfilerHeapShotManager();

	IHeapShot* pHeapShot = 
	pHeapShotMgr->CreateHeapShotFromFile("MonoProfiler_Test.mprof");
	OutputHeapShotToFile("HeapShotReport.txt", pHeapShot);
	DestroyProfilerHeapShotManager(pHeapShotMgr);
	pHeapShotMgr = NULL;
}