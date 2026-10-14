#ifndef PORT_H
#define PORT_H

// Lets the pure modules (utf8, textbuf, ime) build on a host compiler for tests
#ifndef __WATCOMC__
#define __far
#define __huge
#endif

#endif // PORT_H
