#include "Headers.h"
#include "Misc_Func.h"
#include "Design_FileParse.h"
#include "FM_Part_Algorithm.h"



/***************************************************************************
* Function Name: main
* Description: Code execution begins here
****************************************************************************/
int main(int argc, char *argv[])
{
	FILE *cmd_fp = NULL;
	FM_RESULT result = FM_ERROR;
	unsigned long long timeStart = 0;
	unsigned long long timeEnd = 0;
	
#ifdef WIN32
	// Get starting timepoint 
	auto start = high_resolution_clock::now();
#endif
	//Start time
	timeStart = time(0);

	//Setting random funcction seed
	srand((unsigned int)time(0));

	//Read command line arguments
	if (2 != argc)
	{
		printf("\nError. %s file not provided", DESIGN_AUX_FILE);
		return 0;
	}

	cmd_fp = fopen(argv[1], "r");
	if (NULL == cmd_fp)
	{
		printf("\nError Could not open file: %s", DESIGN_AUX_FILE);
		return 0;
	}

	//Read input files
	result = Read_InputFiles(cmd_fp);
	if (FM_ERROR == result)
	{
		printf("\nError main(): Failed to read input files");
		return 0;
	}

	//FM Algorithm
	result = FM_Algorithm_Core();
	if (FM_ERROR == result)
	{
		printf("\nError main(): FM algorithm failed");
		return 0;
	}

	//Endtime
	timeEnd = time(0);

#ifdef WIN32
	// Get ending timepoint 
	auto stop = high_resolution_clock::now();

	auto duration = duration_cast<microseconds>(stop - start);

	printf("\nTime Taken: %d microseconds", duration);

#endif


	printf("\nTime Taken: %d seconds", timeEnd - timeStart);
	

	return 0;
}