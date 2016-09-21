#ifndef _abits_H_
#define _abits_H_
/* Copyright 1995 Carnegie Mellon University All rights reserved. */

/** \addtogroup libbasics
 * @{ */
// The number of bits in the smallest addressable unit (the character).
#ifndef ABITS_BITS_PER_CHAR
#define ABITS_BITS_PER_CHAR 8
#endif

class abits;
// A proxy class used by the abits and abitarray classes to provide a modifiable
// representation of a bit.  The user should never have to worry about
// whether one of these is being used.  (and an abit cannot be constructed
// except by the abits class.)
class abit {
    friend class abits;
  protected:
    unsigned char *thebyte;
    unsigned char thebit;
    inline abit(unsigned char *the_byte, unsigned char the_bit) {
	thebyte=the_byte;
	thebit=the_bit;
    }
  public:
    // return the status of the bit.
    inline operator int() {
	return (*thebyte&(1<<thebit))?TRUE:FALSE;
    }
    // set the status of the bit.
    abit &operator=(int val) {
	if(val) *thebyte|=(1<<thebit);
	else *thebyte&=~(1<<thebit);
	return *this;
    }
};

#define ABITS_CHARS_FOR_BITS(N) (((N)+ABITS_BITS_PER_CHAR-1)/ABITS_BITS_PER_CHAR)

// abits is essentially a pointer into an array of bits.
// Any array of unsigned chars may be used to provide the storage
// or the abitarray class can be used to manage the storage.
//  The operators *, postfix ++, postfix --,  +, -, =,  and []
// behave as for normal pointers.  The rest of the operators
// are not defined at this time.
class abits : private abit {
  public:
    inline abits(unsigned char *the_byte, unsigned char the_bit) : abit(the_byte, the_bit) {
    }
    inline abits() : abit(NULL, 0) {
    }
    inline abit &operator*() {
	return *this;
    }
    inline int operator==(const abits &src) {
	return (src.thebit==thebit) && (src.thebyte==thebyte);
    }
    inline abits &operator++(int) {
	thebit++;
	if(thebit>=ABITS_BITS_PER_CHAR) {
	    thebyte++;
	    thebit=0;
	}
	return *this;
    }
    inline abits &operator--(int) {
	if(thebit==0) {
	    thebyte--;
	    thebit=ABITS_BITS_PER_CHAR-1;
	} else thebit--;
	return *this;
    }
    inline abits operator+(int plus) {
	abits res=*this;
	if(plus>=0) {
	    res.thebyte+=plus/ABITS_BITS_PER_CHAR;
	    res.thebit+=plus%ABITS_BITS_PER_CHAR;
	    if(res.thebit>=ABITS_BITS_PER_CHAR) {
		res.thebyte++;
		res.thebit-=ABITS_BITS_PER_CHAR;
	    }
	} else {
	    res.thebyte-=(-plus)/ABITS_BITS_PER_CHAR;
	    size_t leftovers=(-plus)%ABITS_BITS_PER_CHAR;
	    if(leftovers>res.thebit) {
		res.thebit=ABITS_BITS_PER_CHAR-(leftovers-res.thebit);
		res.thebyte--;
	    } else res.thebit-=leftovers;
	}
	return res;
    }
    inline abits operator-(int minus) {
	abits res=*this;
	if(minus<0) {
	    res.thebyte+=(-minus)/ABITS_BITS_PER_CHAR;
	    res.thebit+=(-minus)%ABITS_BITS_PER_CHAR;
	    if(res.thebit>=ABITS_BITS_PER_CHAR) {
		res.thebyte++;
		res.thebit-=ABITS_BITS_PER_CHAR;
	    }
	} else {
	    res.thebyte-=(minus)/ABITS_BITS_PER_CHAR;
	    size_t leftovers=(minus)%ABITS_BITS_PER_CHAR;
	    if(leftovers>res.thebit) {
		res.thebit=ABITS_BITS_PER_CHAR-(leftovers-res.thebit);
		res.thebyte--;
	    } else res.thebit-=leftovers;
	}
	return res;
    }
	
    inline abits &operator=(const abits &src) {
	thebyte=src.thebyte;
	thebit=src.thebit;
	return *this;
    }
    inline abit operator[](size_t i) {
	unsigned char *tby=thebyte+(i+thebit)/ABITS_BITS_PER_CHAR;
	unsigned char tbi=(i+thebit)%ABITS_BITS_PER_CHAR;
	return abit(tby, tbi);
    }
    inline unsigned char *GetTheByte() const {
	return (thebyte);
    }
    inline unsigned char GetTheBit() const {
	return (thebit);
    }
    inline void SetTheByte(unsigned char *tb) {
	thebyte=tb;
    }
    inline void SetTheBit(unsigned char tb) {
	thebit=tb;
    }    
};

// abitarray provides a resizable array of bits.
// abitarray foo(N) allocates an array of N bits.
// abitarray bar(foo) constructs bar with a copy of the contents of the array in foo.
// foo=bar copies the contents of the array in bar into foo.
// Resize(N) resizes the array to accommodate N bits.  Any bits after the Nth
// are lost.  If the array is grown, the values of the new bits are not defined.
// abits bits=foo, sets bits to be a pointer into the foo array.
// foo[i] returns an abit representing the ith bit. e.g.:
// foo[i]=TRUE; sets the ith bit.
// boolean bit=foo[i]; initializes bit with the value of the ith bit.
//  printf("ith:%d\n", (int)foo[i]); prints the ith bit, the cast
// is needed because with stdarg functions like printf the
// compiler has no other way to know that the int representation
// is the desired one.
// foo.GetN() returns the number of bits in the array.

class abitarray {
    abits bits;
    size_t n;
  public:
    inline abitarray(size_t N) : bits((unsigned char *)malloc(ABITS_CHARS_FOR_BITS(N)),0) {
	n=N;
    }
    inline abitarray(const abitarray &src) : bits((unsigned char *)malloc(ABITS_CHARS_FOR_BITS(src.GetN())), src.bits.GetTheBit()) {
	if(bits.GetTheByte() && src.bits.GetTheByte()) {
	    memcpy(bits.GetTheByte(), src.bits.GetTheByte(), ABITS_CHARS_FOR_BITS(src.GetN()));
	}
	n=src.n;
    }
    inline abitarray &operator=(const abitarray &src) {
	abits local((unsigned char *)malloc(ABITS_CHARS_FOR_BITS(src.GetN())), src.bits.GetTheBit());
	if(local.GetTheByte() && src.bits.GetTheByte()) {
	    memcpy(local.GetTheByte(), src.bits.GetTheByte(), ABITS_CHARS_FOR_BITS(src.GetN()));
	}
	if(bits.GetTheByte()) free(bits.GetTheByte());
	bits=local;
	n=src.n;
	return *this;
    }
    inline void Resize(size_t N) {
	if(n!=N) {
	    unsigned char *res=(unsigned char *)realloc(bits.GetTheByte(), ABITS_CHARS_FOR_BITS(N));
	    bits.SetTheByte(res);
	    n=N;
	}
    }
    inline ~abitarray() {
	if(bits.GetTheByte()) free(bits.GetTheByte());
    }
    operator abits &() {
	return bits;
    }
    abit operator[](size_t i) {
	return bits[i];
    }
    size_t GetN() const{
	return n;
    }
};
/** @} */
#endif

