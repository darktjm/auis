#
# Makefile for the FX (file exchange) server
#
# Copyright 1989, 1990 by the Massachusetts Institute of Technology.
#
# For copying and distribution information, please see the file
# <mit-copyright.h>.
#

# Client-server protocol, error tables kept in separate directories
PROTDIR=../protocol
ETSDIR=../ets

INCLUDES = -I../rpc3.9/usr/include -I../include
LIBS = -L../rpc3.9/usr/lib -lkrb -ldes -lss -lcom_err -lrpclib

CFLAGS = -g ${INCLUDES} -DKERBEROS

.SUFFIXES: .ct .c .o .h

.ct.o:
	rm -f $*.o $*.c
	mk_cmds $*.ct

all: fxtest

FXTESTOBJS = \
	${ETSDIR}/fxserver_err.o \
	${ETSDIR}/rpc_err.o \
	${PROTDIR}/fx_prot_xdr.o \
	${PROTDIR}/fx_prot_clnt.o \
	commands.o \
	fxtest.o

FXTESTSRCS = \
	fxtest.c

fxtest: ${FXTESTOBJS}
	cc -o fxtest ${FXTESTOBJS} ${LIBS}

clean:
	rm -f *.o core fxtest fx_prot_clnt.c fx_prot_xdr.c
