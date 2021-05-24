#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>


typedef union {
	double real_val;

	uint64_t uint_val;
} real;

typedef enum {
PlusZero      = 0x00,
MinusZero     = 0x01,
PlusInf       = 0xF0,
MinusInf      = 0xF1,
PlusRegular   = 0x10,
MinusRegular  = 0x11,
PlusDenormal  = 0x20,
MinusDenormal = 0x21,
SignalingNaN  = 0x30,
QuietNaN      = 0x31
				    
} float_class_t;

extern float_class_t classify(double* value_ptr) { 
	real number;

	number.real_val = *value_ptr;
	uint64_t sign = number.uint_val >> 63;
	uint64_t e = (number.uint_val & (~0ULL >> 1)) >> 52;
	uint64_t m = (number.uint_val & (~0ULL >> 12));
/*
	printf("m: %ld\n", m);
	printf("e: %ld\n", e);
	printf("sign: %ld\n", sign);
	printf("input number: %f\n",number.real_val);
	printf("///////////\n");
*/
	if (e == 0 && m ==0 && sign == 0) {
		//printf("+zero\n");
		return PlusZero;
	}
	if (e == 0 && m == 0 && sign == 1) {
		//printf("-zero\n");
		return MinusZero;
	}
	if (e == 2047 &&  m == 0 && sign == 0) {
		//printf("+inf\n");
		return PlusInf;
	}
	if (e  == 2047 && m == 0 && sign == 1) {
		//printf("-inf\n");
		return MinusInf;
	}

	if (e == 2047 && m >> 51 == 0 && m!=0) {
		//printf("snan\n");
		return SignalingNaN;
	}
	if (e == 2047 && m >> 51 == 1 && m!=0) {
		//printf("qnan\n");
		return QuietNaN;
	}
	if (e == 0 && sign== 0) {
		//printf("+denorm\n");
		return PlusDenormal;
	}
	if (e == 0 && sign == 1){
		//printf("-denorm\n");
		return MinusDenormal;

	}
	if (sign == 1) {
		//printf("-norm\n");
		return MinusRegular;
	}
	// if (e != 0 && sign == 0 && m != 0) {
		//printf("+norm\n");
    return PlusRegular;
	// }

/*
	for (uint64_t i = 0; i < 64;++i) {
		printf("%lld", ((1ULL << (63 - i)) &  number.uint_val) >> (63 - i));
	}
	printf("\n");
*/

}
/*
int main() {
	double a = 1/-INFINITY;
	classify(&a);
	return 0;
}*/