#pragma once

class IHeapShot;


typedef enum{
	PROFILER_FILE_LOGGING_READER = 1
}ProfilerFileReaderType;

class IProfilerFileReader
{
public:
	IProfilerFileReader(){}
	virtual ~IProfilerFileReader(){}

	virtual IHeapShot* CreateHeapShotFromFile(const char* filename) = 0; 
	static IProfilerFileReader* CreateInstance(const ProfilerFileReaderType type);
};

