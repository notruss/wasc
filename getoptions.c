#include <stdio.h>


#include "wasc.h"

int getoptions(int argc, wchar_t **argv, CMDLINEOPTIONS *cmdlineoptions)
{
	for(int i= 1; i < argc; i++)
	{
		if(wcscmp(L"--poll", argv[i]) == 0)
		{
			if((i + 1) == argc)
			{
				cmdlineoptions->errorarg= i;
				return(RETURN_ERROR); //"--poll" requires an argument
			}
			i= i + 1;			//one step to next command line argument
			errno= 0;		//need to set now for afterwards checking
			wchar_t *tailptr= 0;	//see wcstol() man
			cmdlineoptions->pollinterval= wcstol(argv[i], &tailptr, 10);

			if(errno || wcslen(tailptr) != 0)
			{
				cmdlineoptions->errorarg= i;
				return(RETURN_ERROR);
			}
			
			//we processed --poll and it's argument
			continue;
		}
		
		//
		// seems like we have an ordinary string argv so consider it is device id or error if we already have one
		//
		if(cmdlineoptions->deviceidargv)
		{
			cmdlineoptions->errorarg= i;
			return(RETURN_ERROR);
		}
		cmdlineoptions->deviceidargv=argv[i];
	}
	return(RETURN_OK);
}
