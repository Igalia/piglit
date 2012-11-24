/*!
[config]
name: Vector operators
clc_version_min: 10
!*/

kernel void test(global int* out) {
	int4 a;
	int4 b;

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
	sizeof(int4);
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
