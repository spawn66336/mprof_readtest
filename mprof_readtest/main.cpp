#include <stdio.h>
#include <string>
#include <iostream> 
#include <vector>
#include <map>
#include "ProfileReaderInterface.h"

using namespace std;
 

void OutputHeapShotToFile(const std::string& filename, IHeapShot* pHeapShot)
{
	FILE* pOutFile = NULL;
	fopen_s(&pOutFile, filename.c_str(), "wb");
	if (pOutFile)
	{
		std::map<unsigned int,IClassInfo*> classMapping;
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

				auto itFindClass = classMapping.find(pObj->GetClassID());
				if (itFindClass == classMapping.end())
				{
					classMapping.insert(std::make_pair(pObj->GetClassID(), pClass));
				}
				
				fprintf(pOutFile, "%-50s\t\t[%p] refs=%d, size=%d\n" ,pClass->GetName(), pObj->GetID() ,pObj->GetRefObjCount(), pObj->GetSize());

				for (int j = 0; j < pObj->GetRefObjCount(); j++)
				{
					fprintf(pOutFile,"\t\t\t[%p]\n", pObj->GetRefObjIDByIndex(j));
				}
				pHeapData->MoveNextObject();
			} 
			fprintf(pOutFile, "****************************************************\n");
		}


		fprintf(pOutFile, "***********************ClassMapping**********************\n");

		auto itClass = classMapping.begin();
		while (itClass != classMapping.end())
		{
			IClassInfo* pClass = itClass->second;
			fprintf(pOutFile,"[%d]\t\t%s\n", pClass->GetID(), pClass->GetName());
			++itClass;
		}
		 
		fprintf(pOutFile, "*********************************************************\n");

		fclose(pOutFile);
	}
}
 
void main()
{
	IProfilerHeapShotManager* pHeapShotMgr =  CreateProfilerHeapShotManager();

	IHeapShot* pHeapShot = 
	pHeapShotMgr->CreateHeapShotFromFile("HeapShot_2014-4-16_13-2-0-365.mprof");
	OutputHeapShotToFile("HeapShotReport.txt", pHeapShot);
	DestroyProfilerHeapShotManager(pHeapShotMgr);
	pHeapShotMgr = NULL;
}