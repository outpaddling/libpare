############################################################################
# Installed targets

LIB     = libpare.a
HEADERS = pare.h
MAN     = pare.3

############################################################################
# List object files that comprise BIN.

OBJS =  pare_comp.o \
	pare_free.o \
	pare_match.o

############################################################################
# Compile, link, and install options

# Install in ../local, unless defined by the parent Makefile, the
# environment, or a command line option such as PREFIX=/opt/local.
LOCALBASE   ?= ../local
PREFIX      ?= ${LOCALBASE}

# Allow caller to override either MANPREFIX or MANDIR
MANPREFIX   ?= ${PREFIX}
MANDIR      ?= ${MANPREFIX}/man

############################################################################
# Build flags
# Override with "make CC=gcc", "make CC=icc", etc.
# Do not add non-portable options (such as -Wall) using +=

# Portable defaults.  Can be overridden by mk.conf or command line.
CC          ?= gcc
CFLAGS      ?= -Wall -g

CXX         ?= g++
CXXFLAGS    ?= -Wall -g

CPP         ?= cpp

AR          ?= ar
RANLIB      ?= ranlib

INCLUDES    += -isystem ${LOCALBASE}/include
CFLAGS      += ${INCLUDES}

############################################################################
# Assume first command in PATH.  Override with full pathnames if necessary.
# E.g. "make INSTALL=/usr/local/bin/ginstall"
# Do not place flags here (e.g. RM = rm -f).  Just provide the command
# and let each usage dictate the flags.

MKDIR   ?= mkdir
INSTALL ?= install
LN      ?= ln
RM      ?= rm
PRINTF  ?= printf

############################################################################
# Archive rules

all:    ${LIB}

${LIB}: ${OBJS}
	${AR} r ${LIB} ${OBJS}
	${RANLIB} ${LIB}

############################################################################
# Include dependencies generated by "make depend", if they exist.
# These rules explicitly list dependencies for each object file.
# See "depend" target below.  If Makefile.depend does not exist, use
# generic source compile rules.  These have some limitations, so you
# may prefer to create explicit rules for each target file.  This can
# be done automatically using "cpp -M" or "cpp -MM".  Run "man cpp"
# for more information, or see the "depend" target below.

# Rules generated by "make depend"
include Makefile.depend

############################################################################
# Self-generate dependencies the old-fashioned way

depend:
	rm -f Makefile.depend
	for file in *.c; do \
	    ${CPP} ${INCLUDES} -MM $${file} >> Makefile.depend; \
	    ${PRINTF} "\t\$${CC} -c \$${CFLAGS} $${file}\n\n" >> Makefile.depend; \
	done

############################################################################
# Remove generated files (objs and nroff output from man pages)

clean:
	rm -f ${OBJS} ${LIB} *.nr

# Keep backup files during normal clean, but provide an option to remove them
realclean: clean
	rm -f .*.bak *.bak *.BAK *.gmon core *.core

############################################################################
# Install all target files (binaries, libraries, docs, etc.)

install: all
	${MKDIR} -p ${DESTDIR}${PREFIX}/lib ${DESTDIR}${PREFIX}/include ${DESTDIR}${MANDIR}/man3
	${INSTALL} -m 0444 ${LIB} ${DESTDIR}${PREFIX}/lib
	for file in ${HEADERS}; do \
	    ${INSTALL} -m 0444 $${file} ${DESTDIR}${PREFIX}/include; \
	done
	${INSTALL} -m 0444 ${MAN} ${DESTDIR}${MANDIR}/man3
