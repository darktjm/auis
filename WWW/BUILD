#!/bin/sh
#			Build all WWW Code for this platform
#
#	Figure out what sort of unix this is
#	(NeXT machines don't have uname!)

# When BUILDing a SOCKSified httpd:
# Set this to the location of your ready-built SOCKS library
# export SOCKSLIB=/xxxx/yyyy/libsocks.a

## Use this setting to enable SOCKS:
# export SOCKS_FLAGS=-DSOCKS
## ..OR this setting to enable both SOCKS _and_ client access control:
# export SOCKS_FLAGS="-DSOCKS -DCLIENT_CONTROL"
## Note that cern_httpd's normal access control can be used instead.

UNAME=""
test -e /usr/bin/uname &&	UNAME=`/usr/bin/uname`
test -e /bin/uname &&		UNAME=`/bin/uname`
test -e /usr/apollo/bin &&	UNAME=`ver sys5.3 /bin/uname`
test -e /usr/bin/ncrm &&	UNAME=ncr
test -e /usr/bin/swconfig &&	UNAME=sco
if [ -z "$UNAME" ]; then
    if [ -r /NextApps ]; then
        if hostinfo | grep I386 >/dev/null; then
            UNAME=next-386
        else
            UNAME=next
        fi
    fi
fi
#
export UNAME

# For apollo, must use bsd mode. Also, WWW_MACH not inherited through make!
case $UNAME in
	next)		WWW_MACH=next ;;
	next-386)	WWW_MACH=next-386 ;;
	ncr)		WWW_MACH=ncr ;;
	sco)		WWW_MACH=sco; PATH="${PATH}:/usr/local/bin" ;;
	HP-UX)		WWW_MACH=snake ;;
	IRIX)		WWW_MACH=sgi ;;
	SunOS)
		arch=`arch`
		revision=`uname -r`
		if [ "$revision" ~ 5.* ]; then
			WWW_MACH=${arch}-sol2
		else
			WWW_MACH=${arch}
		fi ;;
	ULTRIX)		WWW_MACH=decstation ;;
	AIX)    	WWW_MACH=rs6000 ;;
	OSF1)   	WWW_MACH=osf1 ;;
	BSD/386)	WWW_MACH=unix ;;
	Linux)		WWW_MACH=linux ;;
	A/UX)		WWW_MACH=aux ;;
	SMP_DC.SOx)	WWW_MACH=pyramid ;;
	uts)
		# differentiate between UTS 2 (SVR3/BSD) and UTS 4 (SVR4)
		version=`uname -v`
		if [ "$version" = 2.* ]; then
			WWW_MACH=uts2
		else
			WWW_MACH=uts4
		fi ;;
esac
#
# ISC 3.0 (How can I tell I'm running on ISC 3.0 ???)
#
# WWW_MACH=isc3.0
# WWW_MACH=isc3.0		# Lauren

#
# DELL Unix (How can I tell I'm running on DELL????)
#
# WWW_MACH=dell

#
# Unisys Unix (How can I tell I'm running on Unisys????)
#
# WWW_MACH=unisys

if [ -z "$WWW_MACH" ]; then
	echo
	echo "Please edit BUILD file to include your machine OS"
	echo "and mail differences back to libwww@info.cern.ch"
	echo
	echo "If you are BUILDing for:"
	echo "	- ISC 3.0"
	echo "	- DELL Unix SVR4"
	echo "	- Unisys Unix SVR4"
	echo "just uncomment the corresponding lines in the BUILD script."
	echo
	exit 99
fi
export WWW_MACH
echo "________________________________________________________________"
echo "WWW build for machine type:                            " $WWW_MACH

#	Now go do build

#	We don't want SHELL set to something funny to screw up make
cd All/Implementation; SHELL=/bin/sh make $1
stat=$?
echo
echo "WWW build for " $WWW_MACH  " done. status = " $stat
exit $stat
