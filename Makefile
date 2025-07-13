CSRC=getdevicedataflow.c getoptions.c getwavesubformatstr.c listdevices.c main.c writeall.c
CHDR=wasc.h

LIBS=-lole32 -lmmdevapi -lksguid
CFLAGS=-DCOBJMACROS -D_UNICODE -DUNICODE
LDFLAGS=-mconsole -municode -s

wasc.exe: $(CSRC:.c=.o)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c $(CHDR)
	$(CC) -c $(CFLAGS) -o $@ $(filter-out $(CHDR), $^)

clean:
	del /Q /F *.exe *.o