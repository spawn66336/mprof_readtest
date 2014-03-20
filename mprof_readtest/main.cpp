#include <stdio.h>
#include <string>
#include <iostream>
#include "ProfilerReaderUtil.h"
#include <vector>

using namespace std;

 
typedef struct 
{
	unsigned int version;
	string			mono_name;
	unsigned int	flags;
	 unsigned __int64 start_counter;
	 unsigned __int64 start_time; 
}Profile_Intro_Header_t;

void PrintHeapShotSummary(FILE* pFile ,Profile_Block* pBlock)
{
	if (!pBlock) return;

	if (pBlock->code == MONO_PROFILER_FILE_BLOCK_KIND_HEAP_SUMMARY)
	{ 
		fprintf(pFile, "***********************HeapShot Summary***********************\n");
		Profile_Heapshot_Summary* pSumBlock = (Profile_Heapshot_Summary*)(pBlock);
		fprintf(pFile ,"%-50s %-20s %-20s %-20s %-20s\n", 
			"类名", "可达实例数", "可达实例总大小", "不可达实例数", "不可达实例总大小"
			);
		auto itItem = pSumBlock->items.begin();
		while (itItem != pSumBlock->items.end())
		{ 
			auto itClassInfo = g_class_mapping.find(itItem->class_id);
			if (itClassInfo != g_class_mapping.end())
			{
				std::string className = itClassInfo->second.class_name;
				fprintf(pFile,"%-50s %-20d %-20d %-20d %-20d\n", 
					className.c_str(), 
					itItem->reachable_insts, 
					itItem->reachable_bytes, 
					itItem->unreachable_insts, 
					itItem->unreachable_bytes);
			}
			else{
				fprintf(pFile,"未知的classID=%d\n", itItem->class_id);
			}
			++itItem;
		} 
		fprintf(pFile,"**********************************************************\n");
	}
}

void main()
{
	vector<Profile_Block*> blocks;

	FILE* pFile = fopen("HelloWorld.mprof", "rb"); 
	Profile_Block* pBlock = NULL;
	while (pBlock = ProfilerReaderUtil::ReadBlock(pFile))
	{ 
		blocks.push_back(pBlock); 
	}
	fclose(pFile);

	 
	FILE* pOutputReport = NULL;
	pOutputReport = fopen("Heapshot.txt", "wb");
	auto itBlock = blocks.begin();
	while (itBlock != blocks.end())
	{
		if ((*itBlock)->code == MONO_PROFILER_FILE_BLOCK_KIND_HEAP_SUMMARY )
		{
			PrintHeapShotSummary(pOutputReport,*itBlock);
		} 
		++itBlock;
	}
	fclose(pOutputReport);
	pOutputReport = NULL;

	 itBlock = blocks.begin();
	while (itBlock != blocks.end())
	{
		delete *itBlock;
		++itBlock;
	}
	blocks.clear();
}