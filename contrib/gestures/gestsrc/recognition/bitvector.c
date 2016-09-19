
/***********************************************************************

bitvector.c - some routines for dealing with bitvectors

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

 **********************************************************************/

/*LINTLIBRARY*/

#include "util.h"
#undef	BITS_PER_VECTOR
#define BITS_PER_VECTOR	128
#include "bitvector.h"

int
bitcount(int max, BitVector bv)
{
	int i, count;

	for(count = i = 0; i < max; i++)
		if(IS_SET(i, bv))
			count++;
	return count;
}

char *
BitVectorToString(int max, BitVector bv)
{
	char *string = recog_tempstring();
	int i;

	for(i = 0; i < max; i++)
		string[i] = IS_SET(i, bv) ? (i % 10) + '0' : '-' ;
	string[i] = '\0';
	return string;
}


void
StringToBitVector(char *string, int max, BitVector bv)
{
	int i;

	if((int)strlen(string) != max)
		recog_error("StringToBitVector: strlen(%s)=%d != %d",
			string, (int)strlen(string), max);

	for(i = 0; i < max; i++)
		if(string[i] != '-')
			BIT_SET(i, bv);
		else
			BIT_CLEAR(i, bv);		
}


void SetBitVector(BitVector v)
{
	int nints = INTS_PER_VECTOR;

	while(--nints >= 0)
		*v++ = -1;
}


void ClearBitVector(int nints, BitVector v)
{

	while(--nints >= 0)
		*v++ = 0;
}


void AssignBitVector(int nints, BitVector v1, BitVector v2)
{

	while(--nints >= 0)
		*v1++ = *v2++;
}

int
BitVectorDeQ(int max, BitVector v)
{
	int i;
	for(i = 0; i < max; i++)
		if(IS_SET(i, v)) {
			BIT_CLEAR(i, v);
			return i;
		}
	return -1;

}

int *
BitVectorOr(int *v, int *v1, int *v2, int ipv)
{
	int *vv = v;
	do
		*vv++ = *v1++ | *v2++;
	while(--ipv > 0);
	return v;
}

int *
BitVectorAnd(int *v, int *v1, int *v2, int ipv)
{
	int *vv = v;
	do
		*vv++ = *v1++ & *v2++;
	while(--ipv > 0);
	return v;
}

int
BitVectorNoBitsSet(int *v, int ipv)
{
	do
		if(*v++) return 0;
	while(--ipv > 0);
	return 1;
}
