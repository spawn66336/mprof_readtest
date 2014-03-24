#include <stdio.h>
#include <conio.h>

void take_heapshot(void)
{
	FILE* pFile = NULL;
	unsigned int hs_request = 1; 
	fopen_s(&pFile, "heapshotctrl", "wb");
	if (pFile)
	{
		fwrite(&hs_request, sizeof(hs_request), 1, pFile);
		fclose(pFile);  
	}//if(pFile) 
}

void main()
{
	printf("按\"h\"拍摄快照!\n按\"q\"退出程序!\n");

	int c = 0;
	while (c = _getch())
	{
		if (c == 'h'){
			printf("HeapShot!\n");
			take_heapshot();
		}
		else if (c == 'q'){
			printf("程序退出！\n");
			break;
		}
	}
}