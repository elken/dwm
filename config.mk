# dwm version
VERSION = 6.1

# Customize below to fit your system

# paths
PREFIX = /usr
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/include/X11
X11LIB = /usr/lib/X11

# Xinerama, comment if you don't want it
# XINERAMALIBS  = -lXinerama
# XINERAMAFLAGS = -DXINERAMA

# includes and libs
INCS = -I${X11INC} -I/usr/include/freetype2
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} -lfontconfig -lXft

# flags
CFLAGS   = -g -std=c11 -pedantic -Wall -DVERSION=\"${VERSION}\" -Wno-deprecated-declarations -Os ${INCS} ${CPPFLAGS} -DGRUVBOX 
LDFLAGS  = -s ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = clang
