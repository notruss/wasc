#include <unistd.h>

#include "wasc.h"

int writeall(int filedesc, char *buffer, size_t size)
{
	int returnstatus= RETURN_OK;
	size_t bytesleft= size;
	size_t byteswritten= 0;
	char *buffercursor= buffer;
	while(bytesleft != 0)
	{
		byteswritten= write(filedesc, buffercursor, bytesleft);
		if(byteswritten == -1)
		{
			if(errno == EINTR)
			{
				//just call write() again
				continue;
			}
			//we've got a problem
			returnstatus= RETURN_ERROR;
			goto exit_error_write;
		}
		bytesleft= bytesleft - byteswritten;
		buffercursor= buffercursor + byteswritten;
	}
	
exit_error_write:
	
	return(returnstatus);
}