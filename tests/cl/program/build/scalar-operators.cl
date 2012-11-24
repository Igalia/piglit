/*!
[config]
name: Scalar operators
clc_version_min: 10
!*/

kernel void test(global int* out) {
	int a;
	int b;

	a+b;
	a-b;
	a*b;
	a/b;
	a%b;
	+a;
	-a;
	++a;
	--a;
	a>b;
	a<b;
	a>=b;
	a<=b;
	a==b;
	a!=b;
	a&b;
	a|b;
	a^b;
	~a;
	a&&b;
	a||b;
	!a;
	a ? a : b;
	a<<b;
	a>>b;
	sizeof(int);
	a+b, b+a;
	&a;
	*(&a);
	a=b;

	a+=b;
	a-=b;
	a*=b;
	a/=b;
	a%=b;
	a&=b;
	a|=b;
	a^=b;
	a<<=b;
	a>>=b;
	a+=a,b;
}
