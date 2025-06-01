#include <initguid.h>
#include <mmdeviceapi.h>

#define RETURN_OK 0
#define RETURN_ERROR 1
#define MAX_DEVICES 1024

// macro for getting wchar version of __FILE__
#define _L(x)  __L(x)
#define __L(x)  L##x

#define LOG(format, ...) fprintf(stdout, "%s:%d " format "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define WLOG(format, ...) fwprintf(stdout, L"%s:%d  " format L"\n", _L(__FILE__), __LINE__, ##__VA_ARGS__)
#define LOGE(format, ...) fprintf(stderr, "%s:%d GetLastError()=%d " format "\n", __FILE__, __LINE__, GetLastError(), ##__VA_ARGS__)
#define WLOGE(format, ...) fwprintf(stderr, L"%s:%d GetLastError()=%d " format L"\n", _L(__FILE__), __LINE__, GetLastError(), ##__VA_ARGS__)

typedef struct
{
	unsigned int flags;
} CMDLINEOPTIONS, *PCMDLINEOPTIONS;

int listdevices(IMMDeviceCollection *);
int getoptions(int, wchar_t **, CMDLINEOPTIONS *);
int getdevicedataflow(IMMDevice *pDevice, EDataFlow *pdataflow);
int getwavesubformatstr(GUID *pSubformatGUID, char **mbSubFormatStr);
int writeall(int filedesc, char *buffer, size_t size);
