#include <mmdeviceapi.h>
#include <mmreg.h>
#include <ksmedia.h>

#include "wasc.h"

//
// func getwavesubformatstr() returns pointer to const string associated with subformat GUID
// 
//
int getwavesubformatstr(GUID *pSubformatGUID, char **mbSubFormatStr)
{
	int returnstatus= RETURN_OK;
	int result;

	if(memcmp(pSubformatGUID, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(GUID)) == 0)
	{
		*mbSubFormatStr= "float";
	}
	else if(memcmp(pSubformatGUID, &KSDATAFORMAT_SUBTYPE_PCM, sizeof(GUID)) == 0)
	{
		*mbSubFormatStr= "PCM";
	}
	else
	{
		*mbSubFormatStr= "Unkn";
	}
	
	return(returnstatus);
}