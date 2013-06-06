/*
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors: Tom Stellard <thomas.stellard@amd.com>
 *
 */

char *source =
"// This file is taken and modified from the public-domain poclbm project, and\n"
"// I have therefore decided to keep it public-domain.\n"
"// Modified version copyright 2011-2012 Con Kolivas\n"

"#ifdef VECTORS4\n"
"	typedef uint4 u;\n"
"#elif defined VECTORS2\n"
"	typedef uint2 u;\n"
"#else\n"
"	typedef uint u;\n"
"#endif\n"

"__constant uint K[64] = { \n"
"    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,\n"
"    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,\n"
"    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,\n"
"    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,\n"
"    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,\n"
"    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,\n"
"    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,\n"
"    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2\n"
"};\n"

"__constant uint ConstW[128] = {\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x80000000U, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000280U,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"

"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x80000000U, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000100U,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,\n"
"0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000\n"
"};\n"

"__constant uint H[8] = { \n"
"	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19\n"
"};\n"


"#ifdef BITALIGN\n"
"	#pragma OPENCL EXTENSION cl_amd_media_ops : enable\n"
"	#define rot(x, y) amd_bitalign(x, x, (uint)(32 - y))\n"

"// This part is not from the stock poclbm kernel. It's part of an optimization\n"
"// added in the Phoenix Miner.\n"

"// Some AMD devices have Vals[0] BFI_INT opcode, which behaves exactly like the\n"
"// SHA-256 Ch function, but provides it in exactly one instruction. If\n"
"// detected, use it for Ch. Otherwise, construct Ch out of simpler logical\n"
"// primitives.\n"

" #ifdef BFI_INT\n"
"	// Well, slight problem... It turns out BFI_INT isn't actually exposed to\n"
"	// OpenCL (or CAL IL for that matter) in any way. However, there is \n"
"	// a similar instruction, BYTE_ALIGN_INT, which is exposed to OpenCL via\n"
"	// amd_bytealign, takes the same inputs, and provides the same output. \n"
"	// We can use that as a placeholder for BFI_INT and have the application \n"
"	// patch it after compilation.\n"
"	\n"
"	// This is the BFI_INT function\n"
"	#define Ch(x, y, z) amd_bytealign(x,y,z)\n"
"	// Ma can also be implemented in terms of BFI_INT...\n"
"	#define Ma(z, x, y) amd_bytealign(z^x,y,x)\n"
" #else // BFI_INT\n"
"	// Later SDKs optimise this to BFI INT without patching and GCN\n"
"	// actually fails if manually patched with BFI_INT\n"

"	#define Ch(x, y, z) bitselect((u)z, (u)y, (u)x)\n"
"	#define Ma(x, y, z) bitselect((u)x, (u)y, (u)z ^ (u)x)\n"
"	#define rotr(x, y) amd_bitalign((u)x, (u)x, (u)y)\n"
" #endif\n"
"#else // BITALIGN\n"
"	#define Ch(x, y, z) (z ^ (x & (y ^ z)))\n"
"	#define Ma(x, y, z) ((x & z) | (y & (x | z)))\n"
"	#define rot(x, y) rotate(x, y)\n"
"	#define rotr(x, y) rotate(x, (32-y))\n"
"#endif\n"



"//Various intermediate calculations for each SHA round\n"
"#define s0(n) (S0(Vals[(0 + 128 - (n)) % 8]))\n"
"#define S0(n) (rot(n, 30u)^rot(n, 19u)^rot(n,10u))\n"

"#define s1(n) (S1(Vals[(4 + 128 - (n)) % 8]))\n"
"#define S1(n) (rot(n, 26u)^rot(n, 21u)^rot(n, 7u))\n"

"#define ch(n) Ch(Vals[(4 + 128 - (n)) % 8],Vals[(5 + 128 - (n)) % 8],Vals[(6 + 128 - (n)) % 8])\n"
"#define maj(n) Ma(Vals[(1 + 128 - (n)) % 8],Vals[(2 + 128 - (n)) % 8],Vals[(0 + 128 - (n)) % 8])\n"

"//t1 calc when W is already calculated\n"
"#define t1(n) K[(n) % 64] + Vals[(7 + 128 - (n)) % 8] +  W[(n)] + s1(n) + ch(n) \n"

"//t1 calc which calculates W\n"
"#define t1W(n) K[(n) % 64] + Vals[(7 + 128 - (n)) % 8] +  W(n) + s1(n) + ch(n)\n"

"//Used for constant W Values (the compiler optimizes out zeros)\n"
"#define t1C(n) (K[(n) % 64]+ ConstW[(n)]) + Vals[(7 + 128 - (n)) % 8] + s1(n) + ch(n)\n"

"//t2 Calc\n"
"#define t2(n)  maj(n) + s0(n)\n"

"#define rotC(x,n) (x<<n | x >> (32-n))\n"

"//W calculation used for SHA round\n"
"#define W(n) (W[n] = P4(n) + P3(n) + P2(n) + P1(n))\n"



"//Partial W calculations (used for the begining where only some values are nonzero)\n"
"#define P1(n) ((rot(W[(n)-2],15u)^rot(W[(n)-2],13u)^((W[(n)-2])>>10U)))\n"
"#define P2(n) ((rot(W[(n)-15],25u)^rot(W[(n)-15],14u)^((W[(n)-15])>>3U)))\n"


"#define p1(x) ((rot(x,15u)^rot(x,13u)^((x)>>10U)))\n"
"#define p2(x) ((rot(x,25u)^rot(x,14u)^((x)>>3U)))\n"


"#define P3(n)  W[n-7]\n"
"#define P4(n)  W[n-16]\n"


"//Partial Calcs for constant W values\n"
"#define P1C(n) ((rotC(ConstW[(n)-2],15)^rotC(ConstW[(n)-2],13)^((ConstW[(n)-2])>>10U)))\n"
"#define P2C(n) ((rotC(ConstW[(n)-15],25)^rotC(ConstW[(n)-15],14)^((ConstW[(n)-15])>>3U)))\n"
"#define P3C(x)  ConstW[x-7]\n"
"#define P4C(x)  ConstW[x-16]\n"

"//SHA round with built in W calc\n"
"#define sharoundW(n) Barrier1(n);  Vals[(3 + 128 - (n)) % 8] += t1W(n); Vals[(7 + 128 - (n)) % 8] = t1W(n) + t2(n);  \n"

"//SHA round without W calc\n"
"#define sharound(n)  Barrier2(n); Vals[(3 + 128 - (n)) % 8] += t1(n); Vals[(7 + 128 - (n)) % 8] = t1(n) + t2(n);\n"

"//SHA round for constant W values\n"
"#define sharoundC(n)  Barrier3(n); Vals[(3 + 128 - (n)) % 8] += t1C(n); Vals[(7 + 128 - (n)) % 8] = t1C(n) + t2(n);\n"

"//The compiler is stupid... I put this in there only to stop the compiler from (de)optimizing the order\n"
"#define Barrier1(n) t1 = t1C((n+1))\n"
"#define Barrier2(n) t1 = t1C((n))\n"
"#define Barrier3(n) t1 = t1C((n))\n"

"//#define WORKSIZE 256\n"
"#define MAXBUFFERS (4095)\n"

"__kernel \n"
" __attribute__((reqd_work_group_size(WORKSIZE, 1, 1)))\n"
"void search(	const uint state0, const uint state1, const uint state2, const uint state3,\n"
"						const uint state4, const uint state5, const uint state6, const uint state7,\n"
"						const uint B1, const uint C1, const uint D1,\n"
"						const uint F1, const uint G1, const uint H1,\n"
"						const u base,\n"
"						const uint W16, const uint W17,\n"
"						const uint PreVal4, const uint PreVal0,\n"
"						const uint PreW18, const uint PreW19,\n"
"						const uint PreW31, const uint PreW32,\n"
"						\n"
"						volatile __global uint * output)\n"
"{\n"


"	u W[124];\n"
"	u Vals[8];\n"

"//Dummy Variable to prevent compiler from reordering between rounds\n"
"	u t1;\n"

"	//Vals[0]=state0;\n"
"	Vals[1]=B1;\n"
"	Vals[2]=C1;\n"
"	Vals[3]=D1;\n"
"	//Vals[4]=PreVal4;\n"
"	Vals[5]=F1;\n"
"	Vals[6]=G1;\n"
"	Vals[7]=H1;\n"

"	W[16] = W16;\n"
"	W[17] = W17;\n"

"#ifdef VECTORS4\n"
"	//Less dependencies to get both the local id and group id and then add them\n"
"	W[3] = base + (uint)(get_local_id(0)) * 4u + (uint)(get_group_id(0)) * (WORKSIZE * 4u);\n"
"	uint r = rot(W[3].x,25u)^rot(W[3].x,14u)^((W[3].x)>>3U);\n"
"	//Since only the 2 LSB is opposite between the nonces, we can save an instruction by flipping the 4 bits in W18 rather than the 1 bit in W3\n"
"	W[18] = PreW18 + (u){r, r ^ 0x2004000U, r ^ 0x4008000U, r ^ 0x600C000U};\n"
"#elif defined VECTORS2\n"
"	W[3] = base + (uint)(get_local_id(0)) * 2u + (uint)(get_group_id(0)) * (WORKSIZE * 2u);\n"
"	uint r = rot(W[3].x,25u)^rot(W[3].x,14u)^((W[3].x)>>3U);\n"
"	W[18] = PreW18 + (u){r, r ^ 0x2004000U};\n"
"#else\n"
"	W[3] = base + get_local_id(0) + get_group_id(0) * (WORKSIZE);\n"
"	u r = rot(W[3],25u)^rot(W[3],14u)^((W[3])>>3U);\n"
"	W[18] = PreW18 + r;\n"
"#endif\n"
"	//the order of the W calcs and Rounds is like this because the compiler needs help finding how to order the instructions\n"



"	Vals[4] = PreVal4 + W[3];\n"
"	Vals[0] = PreVal0 + W[3];\n"

"	sharoundC(4);\n"
"	W[19] = PreW19 + W[3];\n"
"	sharoundC(5);\n"
"	W[20] = P4C(20) + P1(20);\n"
"	sharoundC(6);\n"
"	W[21] = P1(21);\n"
"	sharoundC(7);\n"
"	W[22] = P3C(22) + P1(22);\n"
"	sharoundC(8);\n"
"	W[23] = W[16] + P1(23);\n"
"	sharoundC(9);\n"
"	W[24] = W[17] + P1(24);\n"
"	sharoundC(10);\n"
"	W[25] = P1(25) + P3(25);\n"
"	W[26] = P1(26) + P3(26);\n"
"	sharoundC(11);\n"
"	W[27] = P1(27) + P3(27);\n"
"	W[28] = P1(28) + P3(28);\n"
"	sharoundC(12);\n"
"	W[29] = P1(29) + P3(29);\n"
"	sharoundC(13);\n"
"	W[30] = P1(30) + P2C(30) + P3(30);\n"
"	W[31] = PreW31 + (P1(31) + P3(31));\n"
"	sharoundC(14);\n"
"	W[32] = PreW32 + (P1(32) + P3(32));\n"
"	sharoundC(15);\n"
"	sharound(16);\n"
"	sharound(17);\n"
"	sharound(18);\n"
"	sharound(19);\n"
"	sharound(20);\n"
"	sharound(21);\n"
"	sharound(22);\n"
"	sharound(23);\n"
"	sharound(24);\n"
"	sharound(25);\n"
"	sharound(26);\n"
"	sharound(27);\n"
"	sharound(28);\n"
"	sharound(29);\n"
"	sharound(30);\n"
"	sharound(31);\n"
"	sharound(32);\n"
"	sharoundW(33);\n"
"	sharoundW(34);\n"
"	sharoundW(35);\n"
"	sharoundW(36);\n"
"	sharoundW(37);	\n"
"	sharoundW(38);\n"
"	sharoundW(39);\n"
"	sharoundW(40);\n"
"	sharoundW(41);\n"
"	sharoundW(42);\n"
"	sharoundW(43);\n"
"	sharoundW(44);\n"
"	sharoundW(45);\n"
"	sharoundW(46);\n"
"	sharoundW(47);\n"
"	sharoundW(48);\n"
"	sharoundW(49);\n"
"	sharoundW(50);\n"
"	sharoundW(51);\n"
"	sharoundW(52);\n"
"	sharoundW(53);\n"
"	sharoundW(54);\n"
"	sharoundW(55);\n"
"	sharoundW(56);\n"
"	sharoundW(57);\n"
"	sharoundW(58);\n"
"	sharoundW(59);\n"
"	sharoundW(60);\n"
"	sharoundW(61);\n"
"	sharoundW(62);\n"
"	sharoundW(63);\n"

"	W[64]=state0+Vals[0];\n"
"	W[65]=state1+Vals[1];\n"
"	W[66]=state2+Vals[2];\n"
"	W[67]=state3+Vals[3];\n"
"	W[68]=state4+Vals[4];\n"
"	W[69]=state5+Vals[5];\n"
"	W[70]=state6+Vals[6];\n"
"	W[71]=state7+Vals[7];\n"

"	Vals[0]=H[0];\n"
"	Vals[1]=H[1];\n"
"	Vals[2]=H[2];\n"
"	Vals[3]=H[3];\n"
"	Vals[4]=H[4];\n"
"	Vals[5]=H[5];\n"
"	Vals[6]=H[6];\n"
"	Vals[7]=H[7];\n"

"	//sharound(64 + 0);\n"
"	const u Temp = (0xb0edbdd0U + K[0]) +  W[64];\n"
"	Vals[7] = Temp + 0x08909ae5U;\n"
"	Vals[3] = 0xa54ff53aU + Temp;\n"
"	\n"
"#define P124(n) P2(n) + P1(n) + P4(n)\n"


"	W[64 + 16] = + P2(64 + 16) + P4(64 + 16);\n"
"	sharound(64 + 1);\n"
"	W[64 + 17] = P1C(64 + 17) + P2(64 + 17) + P4(64 + 17);\n"
"	sharound(64 + 2);\n"
"	W[64 + 18] = P124(64 + 18);\n"
"	sharound(64 + 3);\n"
"	W[64 + 19] = P124(64 + 19);\n"
"	sharound(64 + 4);\n"
"	W[64 + 20] = P124(64 + 20);\n"
"	sharound(64 + 5);\n"
"	W[64 + 21] = P124(64 + 21);\n"
"	sharound(64 + 6);\n"
"	W[64 + 22] = P4(64 + 22) + P3C(64 + 22) + P2(64 + 22) + P1(64 + 22);\n"
"	sharound(64 + 7);\n"
"	W[64 + 23] = P4(64 + 23) + P3(64 + 23) + P2C(64 + 23) + P1(64 + 23);\n"
"	sharoundC(64 + 8);\n"
"	W[64 + 24] =   P1(64 + 24) + P4C(64 + 24) + P3(64 + 24);\n"
"	sharoundC(64 + 9);\n"
"	W[64 + 25] = P3(64 + 25) + P1(64 + 25);\n"
"	sharoundC(64 + 10);\n"
"	W[64 + 26] = P3(64 + 26) + P1(64 + 26);\n"
"	sharoundC(64 + 11);\n"
"	W[64 + 27] = P3(64 + 27) + P1(64 + 27);\n"
"	sharoundC(64 + 12);\n"
"	W[64 + 28] = P3(64 + 28) + P1(64 + 28);\n"
"	sharoundC(64 + 13);\n"
"	W[64 + 29] = P1(64 + 29) + P3(64 + 29);\n"
"	W[64 + 30] = P3(64 + 30) + P2C(64 + 30) + P1(64 + 30);\n"
"	sharoundC(64 + 14);\n"
"	W[64 + 31] = P4C(64 + 31) + P3(64 + 31) + P2(64 + 31) + P1(64 + 31);\n"
"	sharoundC(64 + 15);\n"
"	sharound(64 + 16);\n"
"	sharound(64 + 17);\n"
"	sharound(64 + 18);\n"
"	sharound(64 + 19);\n"
"	sharound(64 + 20);\n"
"	sharound(64 + 21);\n"
"	sharound(64 + 22);\n"
"	sharound(64 + 23);\n"
"	sharound(64 + 24);\n"
"	sharound(64 + 25);\n"
"	sharound(64 + 26);\n"
"	sharound(64 + 27);\n"
"	sharound(64 + 28);\n"
"	sharound(64 + 29);\n"
"	sharound(64 + 30);\n"
"	sharound(64 + 31);\n"
"	sharoundW(64 + 32);\n"
"	sharoundW(64 + 33);\n"
"	sharoundW(64 + 34);\n"
"	sharoundW(64 + 35);\n"
"	sharoundW(64 + 36);\n"
"	sharoundW(64 + 37);\n"
"	sharoundW(64 + 38);\n"
"	sharoundW(64 + 39);\n"
"	sharoundW(64 + 40);\n"
"	sharoundW(64 + 41);\n"
"	sharoundW(64 + 42);\n"
"	sharoundW(64 + 43);\n"
"	sharoundW(64 + 44);\n"
"	sharoundW(64 + 45);\n"
"	sharoundW(64 + 46);\n"
"	sharoundW(64 + 47);\n"
"	sharoundW(64 + 48);\n"
"	sharoundW(64 + 49);\n"
"	sharoundW(64 + 50);\n"
"	sharoundW(64 + 51);\n"
"	sharoundW(64 + 52);\n"
"	sharoundW(64 + 53);\n"
"	sharoundW(64 + 54);\n"
"	sharoundW(64 + 55);\n"
"	sharoundW(64 + 56);\n"
"	sharoundW(64 + 57);\n"
"	sharoundW(64 + 58);\n"

"	W[117] += W[108] + Vals[3] + Vals[7] + P2(124) + P1(124) + Ch((Vals[0] + Vals[4]) + (K[59] + W(59+64)) + s1(64+59)+ ch(59+64),Vals[1],Vals[2]) -\n"
"		(-(K[60] + H[7]) - S1((Vals[0] + Vals[4]) + (K[59] + W(59+64))  + s1(64+59)+ ch(59+64)));\n"

"#define FOUND (0x0F)\n"
"#define SETFOUND(Xnonce) output[output[FOUND]++] = Xnonce\n"

"#ifdef VECTORS4\n"
"	bool result = W[117].x & W[117].y & W[117].z & W[117].w;\n"
"	if (!result) {\n"
"		if (!W[117].x)\n"
"			SETFOUND(W[3].x);\n"
"		if (!W[117].y)\n"
"			SETFOUND(W[3].y);\n"
"		if (!W[117].z)\n"
"			SETFOUND(W[3].z);\n"
"		if (!W[117].w)\n"
"			SETFOUND(W[3].w);\n"
"	}\n"
"#elif defined VECTORS2\n"
"	bool result = W[117].x & W[117].y;\n"
"	if (!result) {\n"
"		if (!W[117].x)\n"
"			SETFOUND(W[3].x);\n"
"		if (!W[117].y)\n"
"			SETFOUND(W[3].y);\n"
"	}\n"
"#else\n"
"	if (!W[117])\n"
"		SETFOUND(W[3]);\n"
"#endif\n"
"}\n";

#include "piglit-util.h"
#include "piglit-framework-cl-program.h"

#define EXPECTED_NONCE 1322941352

PIGLIT_CL_PROGRAM_TEST_CONFIG_BEGIN

	config.name = "Bitcoin phatk kernel";
	config.program_source = source;
	config.kernel_name = "search";
	config.build_options = "-D WORKSIZE=256";
	config.run_per_device = true;

PIGLIT_CL_PROGRAM_TEST_CONFIG_END

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_program_test_config* config,
               const struct piglit_cl_program_test_env* env)
{
	unsigned i;
	enum piglit_result result = PIGLIT_PASS;

	size_t global_size = 32768;
	size_t local_size = 256;
	unsigned data[16];

	cl_mem buffer = NULL;

	static const unsigned args[] = {
		451767447,
		953606276,
		3289282709,
		1043157133,
		3980456748,
		2761019621,
		3674259583,
		4041779413,
		1887291860,
		3618940936,
		4220496293,
		2530934726,
		204837088,
		3991048658,
		1322909696,
		2820830137,
		1604534223,
		3318230743,
		591787567,
		1627450019,
		151637226,
		4206519064,
		47933991,
	};

	/* Create CL objects */
	memset(data, 0, sizeof(data));
	buffer = piglit_cl_create_buffer(env->context, CL_MEM_WRITE_ONLY,
					sizeof(data));
	/* Initialize the buffer */
	piglit_cl_write_whole_buffer(env->context->command_queues[0],
							buffer, data);

	/* Set the kernel args */
	for (i = 0; i < 23; i++) {
		piglit_cl_set_kernel_arg(env->kernel, i, 4, &args[i]);
	}
	piglit_cl_set_kernel_buffer_arg(env->kernel, 23, &buffer);

	piglit_cl_execute_ND_range_kernel(env->context->command_queues[0],
					env->kernel, 1,
					&global_size, &local_size);
	piglit_cl_read_whole_buffer(env->context->command_queues[0], buffer, data);

	if (data[0] != EXPECTED_NONCE) {
		fprintf(stderr, "output[0] expected %u, but got %u\n",
			EXPECTED_NONCE, data[0]);
		result = PIGLIT_FAIL;
	}

	for (i = 1; i < 15; i++) {
		if (data[i] != 0) {
			fprintf(stderr, "output[%u] expected %u, but got %u\n",
				i, 0, data[i]);
			result = PIGLIT_FAIL;
		}
	}

	if (data[15] != 1) {
		fprintf(stderr, "output[15] expected 1, but got %u\n",
			data[15]);
		result = PIGLIT_FAIL;
	}

	return result;
}
