/**************************************************************************
 *
 * Copyright 2012 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef MERSENNE_HPP
#define MERSENNE_HPP

#include <string>
#include <stdio.h>
#include <stdint.h>

/**
 * Variant of Mersenne Twister which can be skipped to any point in time.
 *
 * Instead of producing a new state table by mutating the previous table it initiates
 * again using a seed which is the last random value of the previous state
 */

class Mersenne
{
public:
   static const uint32_t n = 624;
   static const uint32_t m = 397;
   static const uint32_t b32 = 1 << 31;
   static const uint32_t rand_max = 0xFFFFFFFF;

public:
   Mersenne()
   {
   }

   Mersenne(unsigned int seed)
   {
      init(seed);
   }

   unsigned int value()
   {
      uint32_t x = mState[mIndex++];
      x ^= (x >> 11);
      x ^= (x << 7) & 0x9D2C5680;
      x ^= (x << 15) & 0xEFC60000;
      x ^= (x >> 18);

      if (mIndex == n){
         init(x);
         return value();
      }

      return x;
   }

   unsigned int max()
   {
      return 0xFFFFFFFF;
   }

   std::string state()
   {
      char buffer[32];
      sprintf(buffer, "%08x%03d", mSeed, mIndex);
      return buffer;
   }

   void setState(const std::string& state){
      uint32_t seed, index;
      sscanf(state.c_str(), "%08x%03d", &seed, &index);

      init(seed);
      mIndex = index;
   }

   void init(uint32_t seed)
   {
      mIndex = 0;
      mSeed = seed;

      /* Standard MT initialiser */
      mState[0] = seed;

      for (uint32_t i = 1; i < n; ++i)
         mState[i] = 1812433253 * (mState[i - 1] ^ (mState[i - 1] >> 30)) + i;

      /* Standard MT number generator, split into parts to avoid having to do % n */
      for (uint32_t i = 0; i < (n - m); ++i)
         mState[i] = mState[i + m] ^ twist(mState[i], mState[i + 1]);

      for (uint32_t i = n - m; i < (n - 1); ++i)
         mState[i] = mState[i + m - n] ^ twist(mState[i], mState[i + 1]);

      mState[n - 1] = mState[m - 1] ^ twist(mState[n - 1], mState[0]);
   }

private:
   inline uint32_t twist(uint32_t a, uint32_t b)
   {
      return (((a & b32) | (b & ~b32)) >> 1) ^ ((b & 1) ? 0x9908B0DF : 0);
   }

private:
   uint32_t mSeed;
   uint32_t mIndex;
   uint32_t mState[n];
};

#endif // MERSENNE_HPP
