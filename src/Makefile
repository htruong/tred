obj = vgatxt.obj vnfont.obj vncvt.obj utf8.obj surface.obj draw.obj mouse.obj kbd.obj textbuf.obj ime.obj editor.obj menu.obj main.obj
bin = tred.exe

CC = wcl
CFLAGS = -0 -ml -otexan -s -d0

.ERASE

# The object list is longer than a DOS command line, so link from a response
# file. wlink's default DOS stack is only 2 KB; give the editor some headroom.
$(bin): $(obj)
    %create tred.lnk
    @for %i in ($(obj)) do @%append tred.lnk file %i
    wlink system dos name $@ option quiet option stack=16k @tred.lnk

# .AUTODEPEND rebuilds an object when a header it includes changes (Watcom
# records those dependencies in each object file)
.c.obj: .AUTODEPEND
    $(CC) -bt=dos -fo=$@ $(CFLAGS) -c $<

clean: .symbolic
    rm *.obj
    rm $(bin)
