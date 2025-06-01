LIBS=-lole32 -lmmdevapi -lksguid
CFLAGS=-DCOBJMACROS -D_UNICODE -DUNICODE
LDFLAGS=-mconsole -municode -s

wasc.exe: getdevicedataflow.o getoptions.o getwavesubformatstr.o listdevices.o main.o writeall.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o : %.c
	 $(CC) -c $(CFLAGS) $< -o $@

clean:
	del /Q /F *.exe *.o