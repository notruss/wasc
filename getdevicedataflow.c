#include <mmdeviceapi.h>
#include <audioclient.h>

#include "wasc.h"

//
// func getdevicedataflow() gets device dataflow direction
// 
//
int getdevicedataflow(IMMDevice *pDevice, EDataFlow *pdataflow)
{
	int returnstatus= RETURN_OK;
	HRESULT hresult;
	IMMEndpoint *pIMMEndpoint= NULL;

	//
	// Get IMMEndpoint interface
	//	
	hresult= IMMDevice_QueryInterface(pDevice, &IID_IMMEndpoint, (void **) &pIMMEndpoint);
	if(hresult != S_OK)
	{
		returnstatus=RETURN_ERROR;
		goto return_error_IMMDevice_QueryInterface;
	}

	//
	// Get endpoint data flow direction
	//
	hresult= IMMEndpoint_GetDataFlow(pIMMEndpoint, pdataflow);
	if(hresult != S_OK)
	{
		returnstatus=RETURN_ERROR;
		goto return_error_IMMEndpoint_GetDataFlow;
	}

return_error_IMMEndpoint_GetDataFlow:

//
// Release IMMEndpoint interface
//
	IMMEndpoint_Release(pIMMEndpoint);

return_error_IMMDevice_QueryInterface:

	return(returnstatus);	
}