#include <wchar.h>
#include <windows.h>
#include <stdio.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <audioclient.h>

#include "wasc.h"

//
// func listdevices()- iterates through IMMDeviceCollection from device number 0
// till error or MAX_DEVICES riched
// returns RETURN_OK RETURN_ERROR
//

const char *streRender= "eRender";
const char *streCapture= "eCaptur";

int listdevices(IMMDeviceCollection *pCollection)
{
	int returnstatus= RETURN_OK;
	UINT nDevice= 0;
//
//
//
	while(nDevice < MAX_DEVICES)
	{
		IMMDevice *pDevice= NULL;
		HRESULT hresult= IMMDeviceCollection_Item(pCollection, nDevice, &pDevice);
		
		if(hresult != S_OK)
		{
			break;
		}

		//
		//work with IMMDevice instance
		//

		//
		// Get device ID unicode string
		//
		LPWSTR pwszID = NULL;
		
		hresult= IMMDevice_GetId(pDevice, &pwszID);
		if(hresult == S_OK)
		{
			char mbzID[256]; //multibyte string got from pwszID conversion
			mbzID[255]= 0; //preset the terminatig null character in case we have 255 chars lenght result
			if(wcstombs(mbzID, pwszID, sizeof(mbzID) - sizeof(mbzID[0])) == -1)
			{
				printf("{multibyte conversion error}");
			}
			else
			{
				printf("%s", mbzID);
			}
			CoTaskMemFree(pwszID);
		}
		else
		{
			printf("{IMMDevice_GetId() error}");
		}
		
		EDataFlow eDataFlow;
		if(getdevicedataflow(pDevice, &eDataFlow) == RETURN_OK)
		{
			const char *strDataflow;
			switch(eDataFlow)
			{
				case eRender:
					strDataflow= streRender;
					break;
				case eCapture:
					strDataflow= streCapture;
					break;
				default:
					strDataflow= "UnknVal";
			}
			printf(" %s", strDataflow);
		}
		else
		{
			printf(" error__");
		}



		// Get device friendly name
		//
		IPropertyStore *pProperties= NULL;
		hresult= IMMDevice_OpenPropertyStore(pDevice, STGM_READ, &pProperties);
		if(hresult == S_OK)
		{
			PROPVARIANT propvariant;
			
			PropVariantInit(&propvariant);
			hresult= IPropertyStore_GetValue(pProperties, &PKEY_Device_FriendlyName, &propvariant);
			if(hresult == S_OK || hresult == INPLACE_S_TRUNCATED)
			{
				char mbzDevice_FriendlyName[256]; //multibyte string got from propvariant.pwszVal conversion
				mbzDevice_FriendlyName[255]= 0; //preset the terminatig null character in case we have 255 chars lenght result
				if(wcstombs(mbzDevice_FriendlyName, propvariant.pwszVal, sizeof(mbzDevice_FriendlyName) - sizeof(mbzDevice_FriendlyName[0])) == -1)
				{
					printf(" \"multibyte conversion error\"\n");
				}
				else
				{
					printf(" \"%s\"\n", mbzDevice_FriendlyName);
				}
			}
			PropVariantClear(&propvariant);
			IPropertyStore_Release(pProperties);

		}
		else
		{
			printf(" \"\"\n");
		}

		//
		// Get AudioClient interface by IMMDevice::Activate
		//
		IAudioClient *pAudioClient= NULL;
		hresult= IMMDevice_Activate(pDevice, &IID_IAudioClient, CLSCTX_ALL, NULL, (void **) &pAudioClient);
		if(hresult != S_OK)
		{
			LOGE("IMMDevice_Activate()= 0x%x", hresult);
			goto loop_error_IMMDevice_Activate;
		}

		//
		// Get Windows audio server internal mixing format for device
		//
		WAVEFORMATEX *pWaveFormatEx= NULL;
		hresult= IAudioClient_GetMixFormat(pAudioClient, &pWaveFormatEx);
		if(hresult != S_OK)
		{
			LOGE("IAudioClient_GetMixFormat()= 0x%x", hresult);
			goto loop_error_IAudioClient_GetMixFormat;
		}
		
		//
		// Print basic wave format info
		//
		printf("\\-%s(0x%x), %d b/sa, %d ch, %d B/frame, %d sa/s", (pWaveFormatEx->wFormatTag == WAVE_FORMAT_PCM) ? "PCM" : ( (pWaveFormatEx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) ? "EXT" : "?"), pWaveFormatEx->wFormatTag, pWaveFormatEx->wBitsPerSample, pWaveFormatEx->nChannels, pWaveFormatEx->nBlockAlign, pWaveFormatEx->nSamplesPerSec );

		//
		// Print more info if we got WAVEFORMATEXTENSIBLE
		//
		if(pWaveFormatEx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		{
			PWAVEFORMATEXTENSIBLE pWFExtensible= (PWAVEFORMATEXTENSIBLE) pWaveFormatEx;
			char *pSubFormatStr;
			
			getwavesubformatstr(&pWFExtensible->SubFormat, &pSubFormatStr);
			printf(", %s, %d valid b/sa, ch_mask=0x%x", pSubFormatStr, pWFExtensible->Samples.wValidBitsPerSample, pWFExtensible->dwChannelMask);
		}
		printf(", %d Kib/s\n", (pWaveFormatEx->nAvgBytesPerSec * 8) / 1024);

		CoTaskMemFree(pWaveFormatEx);

		loop_error_IAudioClient_GetMixFormat:

		//
		// Release IAudioClient
		//
		IAudioClient_Release(pAudioClient);

		loop_error_IMMDevice_Activate:

		//
		//release current IMMDevice instance from IMMDeviceCollection
		//
		IMMDevice_Release(pDevice);

		//next device
		nDevice= nDevice + 1;
	}


error_IMMDeviceCollection_Item:
	//nothing

	return(returnstatus);
}
