#include "fixed_point.h"
#include <stdio.h>

#define SHIFT 1000000.0

void print_fixed(int16_t raw, int16_t q) {

    double x = (double)raw / (1 << q);

    int64_t truncate = (int64_t)(x * SHIFT);
    double res = (double)(truncate / SHIFT);

    printf("%.6f", res);
}

int16_t add_fixed(int16_t a, int16_t b) {
    return (int16_t)(a + b);
}

int16_t subtract_fixed(int16_t a, int16_t b) {
    return (int16_t)(a - b);
}

int16_t multiply_fixed(int16_t a, int16_t b, int16_t q) {
    // TODO: Implement fixed-point multiply:
       //- Use a wider type for intermediate multiplication (e.g., int32_t or int64_t).

    int16_t raw_out = 0;

    raw_out = (int16_t)(((int32_t)(a*b))/(1 << q));

    return raw_out;
}

void eval_poly_ax2_minus_bx_plus_c_fixed(int16_t x, int16_t a, int16_t b, int16_t c, int16_t q) {
    /* TODO:
       Evaluate: y = a*x^2 - b*x + c
    */
   

    printf("the polynomial output for a=");
    print_fixed(a, q);
    printf(", b=");
    print_fixed(b, q);
    printf(", c=");
    print_fixed(c, q);
    printf(" is ");
    /* TODO: print y once computed */

    long double y = 0;
    y = add_fixed(multiply_fixed(a,multiply_fixed(x,x,q),q),subtract_fixed(c, multiply_fixed(b,x,q)));
    
    print_fixed(y, q);

}

/*
int main()
{
    eval_poly_ax2_minus_bx_plus_c_fixed(200,1002,1200,9012,12);
    return 0;
}
*/
