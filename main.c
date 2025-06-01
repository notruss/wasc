#include <wchar.h>
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <audioclient.h>

#include "wasc.h"

#define EXIT_OK 0
#define EXIT_ERROR 1


int wmain(int argc, wchar_t **argv)
{
	unsigned int exitstatus= EXIT_OK;
	HRESULT hresult;

	if(setmode(STDOUT_FILENO, O_BINARY) == -1 )
	{
		exitstatus= EXIT_ERROR;
		LOGE("_setmode() failed");
		goto exit_error_setmode;
	}
//
// Get command line options
//
	CMDLINEOPTIONS cmdlineoptions;
	cmdlineoptions.flags= 0;
	if(getoptions(argc, argv, &cmdlineoptions) != RETURN_OK)
	{
		exitstatus= EXIT_ERROR;
		LOGE("getoptions() != RETURN_OK");
		goto exit_error;
	}
	
//	
// COM Initialization
//
	hresult=CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(hresult != S_OK)
	{
		exitstatus= EXIT_ERROR;
		LOGE("0x%X %d", hresult, hresult);
		goto exit_error_CoInitializeEx ;
	}

//
// Get MMDeviceEnumerator instance
//
	IMMDeviceEnumerator *pEnumerator= NULL;
	hresult=CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void **) &pEnumerator);
	if(hresult != S_OK)
	{
		exitstatus= EXIT_ERROR;
		LOGE("0x%X %d", hresult, hresult);
		goto exit_error_CoCreateInstance_MMDeviceEnumerator ;
		
	}
	
//
// List devices and exit from main if none cmdline parameters
//
	if( argc == 1)
	{
		IMMDeviceCollection *pCollection= NULL;
		hresult= IMMDeviceEnumerator_EnumAudioEndpoints(pEnumerator, eAll, DEVICE_STATE_ACTIVE, &pCollection);
		if(hresult != S_OK)
		{
			exitstatus= EXIT_ERROR;
			LOGE("IMMDeviceEnumerator_EnumAudioEndpoints()= 0x%x", hresult);
			goto exit_error_MMDeviceEnumerator_EnumAudioEndpoints;
		}
		//
		// List devices in IMMDeviceCollection
		//
		if(listdevices(pCollection) != RETURN_OK)
		{
			LOGE("listdevices() != RETURN_OK");
		}
		IMMDeviceCollection_Release(pCollection);
		goto exit_listonly;
	}

//
// Get particular IMMDevice interface by endpoint id wchar string from cmdline
//
	IMMDevice *pDevice= NULL;
	hresult= IMMDeviceEnumerator_GetDevice(pEnumerator, argv[1], &pDevice);
	if(hresult != S_OK)
	{
		exitstatus= EXIT_ERROR;
		LOGE("IMMDeviceEnumerator_GetDevice()= 0x%x", hresult);
		goto exit_error_MMDeviceEnumerator_GetDevice;
	}

//
// Get AudioClient interface by IMMDevice::Activate
//
	IAudioClient *pAudioClient= NULL;
	hresult= IMMDevice_Activate(pDevice, &IID_IAudioClient, CLSCTX_ALL, NULL, (void **) &pAudioClient);
	if(hresult != S_OK)
	{
		exitstatus= EXIT_ERROR;
		LOGE("IMMDevice_Activate()= 0x%x", hresult);
		goto exit_error_IMMDevice_Activate;
	}

//
// Get Windows audio server internal mixing format for device
//
	WAVEFORMATEX *pWaveFormatEx= NULL;
	hresult= IAudioClient_GetMixFormat(pAudioClient, &pWaveFormatEx);
	if(hresult != S_OK)
	{
		LOGE("IAudioClient_GetMixFormat()= 0x%x", hresult);
		goto exit_error_IAudioClient_GetMixFormat;
	}


//
// Initialize audio session with IAudioClient::Initialize
//
	hresult= IAudioClient_Initialize(pAudioClient, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, pWaveFormatEx, NULL);
	if(hresult != S_OK)
	{
		exitstatus= EXIT_ERROR;
		LOGE("IAudioClient_Initialize()= 0x%x", hresult);
		goto exit_error_IAudioClient_Initialize;
	}

//
// Get windows audio server allocated buffer size
//
	UINT32 NumBufferFrames;
	int bufferdurationms;
	hresult= IAudioClient_GetBufferSize(pAudioClient, &NumBufferFrames);
	if(hresult != S_OK)
	{
		exitstatus= EXIT_ERROR;
		LOGE("IAudioClient_GetBufferSize()= 0x%x", hresult);
		goto exit_error_IAudioClient_GetBufferSize;
	}
	bufferdurationms= (NumBufferFrames * 1000) / pWaveFormatEx->nSamplesPerSec;
	
//
// Initialize buffer with silence- calloc() zeroes buffer before returning the pointer.
//
	unsigned int silencebuffersize= pWaveFormatEx->nBlockAlign * NumBufferFrames;
	BYTE *psilencebuffer= calloc(silencebuffersize, 1);
	if(psilencebuffer == 0)
	{
		exitstatus= EXIT_ERROR;
		LOGE("calloc()=0, errno=%d", errno);
		goto exit_error_calloc;
	}
	

//
// Get IAudioCaptureClient interface
//
	IAudioCaptureClient *pAudioCaptureClient= NULL;
	hresult= IAudioClient_GetService(pAudioClient, &IID_IAudioCaptureClient, (void **) &pAudioCaptureClient);
	if(hresult != S_OK)
	{
		exitstatus= EXIT_ERROR;
		LOGE("IAudioClient_GetService()= 0x%x", hresult);
		goto exit_error_IAudioClient_GetService;
	}

//
// Start stream
//
	hresult= IAudioClient_Start(pAudioClient);
	if(hresult != S_OK)
	{
		exitstatus= EXIT_ERROR;
		LOGE("IAudioClient_Start()= 0x%x", hresult);
		goto exit_error_IAudioClient_Start;
	}
	
	// sleep interval between reading input in ms
	int sleepdurationms= bufferdurationms / 3;

//
// Reading audiodata loop
//

	for(;;)
	{
		BYTE *pData;
		UINT32 NumFramesToRead;
		DWORD dwFlags;
		
		Sleep(sleepdurationms);
		for(;;)
		{
			hresult= IAudioCaptureClient_GetBuffer(pAudioCaptureClient, &pData, &NumFramesToRead, &dwFlags, NULL, NULL);
			if(hresult == S_OK)
			{
				// got data packet, write it out or write out the silence buffer if corresponding flag is set

				if(writeall(STDOUT_FILENO, (dwFlags & AUDCLNT_BUFFERFLAGS_SILENT) ? psilencebuffer : pData, pWaveFormatEx->nBlockAlign * NumFramesToRead) == RETURN_ERROR)
				{
					exitstatus= EXIT_ERROR;
					LOGE("writeall()= RETURN_ERROR");
					goto exit_error_writeall;
				}
				hresult= IAudioCaptureClient_ReleaseBuffer(pAudioCaptureClient, NumFramesToRead);
				if(hresult != S_OK)
				{
					exitstatus= EXIT_ERROR;
					LOGE("IAudioCaptureClient_ReleaseBuffer()= 0x%x", hresult);
					goto exit_error_IAudioCaptureClient_ReleaseBuffer;
				}
				if(dwFlags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY)
				{
					LOGE("AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY");
				}
				if(dwFlags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR)
				{
					LOGE("AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR");
				}

				continue;
			}
			if(hresult == AUDCLNT_S_BUFFER_EMPTY)
			{
				//nothing left to fetch
				break;
			}

			//
			// Any other hresult considered an error
			exitstatus= EXIT_ERROR;
			LOGE("IAudioCaptureClient_GetBuffer()= 0x%x", hresult);
			goto exit_error_IAudioCaptureClient_GetBuffer;

		}
		
	}


exit_error_IAudioCaptureClient_ReleaseBuffer:
exit_error_writeall:
exit_error_IAudioCaptureClient_GetBuffer:

//
// Stop stream
//
	hresult= IAudioClient_Stop(pAudioClient);
	if(hresult != S_OK && hresult != S_FALSE)
	{
		LOGE("IAudioClient_Stop()= 0x%x", hresult);
	}
	
	

exit_error_IAudioClient_Start:

//
// Release IAudioCaptureClient interface
//
	IAudioCaptureClient_Release(pAudioCaptureClient);

//
// Free silence buffer memory
//
	free(psilencebuffer);


exit_error_IAudioClient_GetService:
exit_error_calloc:
exit_error_IAudioClient_GetBufferSize:
exit_error_IAudioClient_Initialize:

//
// Free pWaveFormatEx mem
//
	CoTaskMemFree(pWaveFormatEx);	

exit_error_IAudioClient_GetMixFormat:

//
// Release IAudioClient
//
	IAudioClient_Release(pAudioClient);

exit_error_IMMDevice_Activate:

//
// Release IMMDevice interface
//
	IMMDevice_Release(pDevice);


exit_error_MMDeviceEnumerator_GetDevice:
exit_error_MMDeviceEnumerator_EnumAudioEndpoints:
exit_error_listdevices:
exit_listonly:

//
// Release IMMDeviceEnumerator
//
	hresult= IMMDeviceEnumerator_Release(pEnumerator);

exit_error_CoCreateInstance_MMDeviceEnumerator:

exit_error_CoInitializeEx:
	CoUninitialize();

exit_error:
exit_error_setmode:

	return(exitstatus);
}