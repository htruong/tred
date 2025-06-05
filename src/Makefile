obj = screen.obj vscreen.obj vncvt.obj vgatxt.obj main.obj
bin = vicat.exe

CC = wcl
CFLAGS = -3 -fpi87 -fp3 -os -d0 -mh
LD = wcl

$(bin): $(obj)
    $(LD) $(CFLAGS) -l=dos -fe=$@ $(obj)

.c.obj:
    $(CC) -bt=dos -fo=$@ $(CFLAGS) -c $<

clean: .symbolic
    rm *.obj
    rm $(bin)