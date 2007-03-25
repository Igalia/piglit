// BEGIN_COPYRIGHT
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT




// rand.h:  Simple random sequence generation utilities.

// We provide these to eliminate dependencies on the operating
// system's random number generator.  This makes it possible to
// compare results for a given graphics device running under different
// operating systems.

// Based on Numerical Recipes, 2d ed., p. 284.


#ifndef __rand_h__
#define __rand_h__


namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// RandomBase:  Quick-and-dirty linear congruential generator that serves as
// a base for other random-sequence classes.
///////////////////////////////////////////////////////////////////////////////
class RandomBase {
	unsigned int i;
    public:
	inline RandomBase(unsigned int seed): i(seed) { }
	inline RandomBase(): i(1) { }
	inline unsigned int next() {
		i = 1664525 * i + 1013904223;
		return i;
	}
}; // class RandomBase

///////////////////////////////////////////////////////////////////////////////
// RandomBits:  Returns a given number of random bits (expressed as an unsigned
// int, so the maximum portable number of bits is 32).
///////////////////////////////////////////////////////////////////////////////
class RandomBits: public RandomBase {
	unsigned int shift;
    public:
    	inline RandomBits(unsigned int bits, unsigned int seed):
	    RandomBase(seed), shift(32 - bits) { }
	inline RandomBits(unsigned int bits):
	    RandomBase(), shift(32 - bits) { }
	inline unsigned int next() { return RandomBase::next() >> shift; }
}; // class RandomBits

///////////////////////////////////////////////////////////////////////////////
// RandomSignedBits:  Returns a given number of random bits (expressed as a
// signed int, so the maximum portable number of bits is 32 including sign).
///////////////////////////////////////////////////////////////////////////////
class RandomSignedBits: public RandomBase {
	unsigned int shift;
    public:
    	inline RandomSignedBits(unsigned int bits, unsigned int seed):
	    RandomBase(seed), shift(32 - bits) { }
	inline RandomSignedBits(unsigned int bits):
	    RandomBase(), shift(32 - bits) { }
	inline int next() {
		return static_cast<int>(RandomBase::next()) >> shift;
	}
}; // class RandomSignedBits

///////////////////////////////////////////////////////////////////////////////
// RandomDouble:  Returns a random floating-point value in the closed
// interval [0.0, 1.0].
///////////////////////////////////////////////////////////////////////////////
class RandomDouble: public RandomBase {
    public:
	inline RandomDouble(unsigned int seed):  RandomBase(seed) { }
	inline RandomDouble(): RandomBase() { }
	inline double next() {
		return static_cast<double>(RandomBase::next()) / 4294967295.0;
	}
}; // class RandomDouble

///////////////////////////////////////////////////////////////////////////////
// RandomBitsDouble:  Returns a random floating-point value in the closed
// interval [0.0, 1.0], but with possible values limited by a generator
// returning a specific number of bits.
///////////////////////////////////////////////////////////////////////////////
class RandomBitsDouble: public RandomBits {
	double scale;
    public:
	inline RandomBitsDouble(unsigned int bits, unsigned int seed):
	    RandomBits(bits, seed) { scale = (1 << bits) - 1.0; }
	inline RandomBitsDouble(unsigned int bits):
	    RandomBits(bits) { scale = (1 << bits) - 1.0; }
	inline double next() {
		return static_cast<double>(RandomBits::next()) / scale;
	}
}; // class RandomBitsDouble

} // namespace GLEAN

#endif // __rand_h__
