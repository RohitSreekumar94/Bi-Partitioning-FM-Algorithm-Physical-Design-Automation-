#include "Headers.h"
#include "Misc_Func.h"

using namespace std;


/***************************************************************************
* Function Name: GetSizeofFile
* Description: Function to get the size of the file
****************************************************************************/
unsigned int GetSizeofFile(FILE *fp)
{
	unsigned int fileSize = 0;

	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);	// seek to end of file
		fileSize = (unsigned int)ftell(fp);		// get current file pointer
		fseek(fp, 0, SEEK_SET);	// seek back to beginning of file
	}
	else
	{
		Log_Message(LOG_ERROR,"GetSizeofFile() :: Invalid file pointer");
		return 0;
	}

	return fileSize;
}

