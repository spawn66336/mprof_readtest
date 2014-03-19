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

void main()
{
	FILE* pFile = fopen("profiler-log.mprof", "rb"); 

	vector<Profile_Block_t*> blocks; 
	while (1)
	{
		Profile_Block_t* pNewBlock = new Profile_Block_t;
		if(!ProfilerReaderUtil::ReadBlock(pFile, *pNewBlock))
		{
			delete pNewBlock;
			break;
		}

		cout << MonoProfilerFileBlockKindMap(pNewBlock->head.code) << " size=" << pNewBlock->head.size << endl;
		blocks.push_back(pNewBlock);
	} 

	fclose(pFile);
	pFile = NULL;

	auto itDel = blocks.begin();
	while (itDel != blocks.end())
	{
		delete *itDel;
		*itDel = NULL;
		++itDel;
	}
	blocks.clear();
}