#include "fixed_point.h"
#include <stdio.h>
#include <stdint.h>

#define SHIFT 1000000.0

void print_fixed(int16_t raw, int16_t q) {
    // Interpret the raw value as a real number based on q bits
    double x = (double)raw / (1 << q);
    
    // Print with exactly 6 decimal places
    // The %.6f format specifier handles formatting automatically
    printf("%.6f", x);
}

int16_t add_fixed(int16_t a, int16_t b) {
    // Inputs share the same q [cite: 152, 156]
    return (int16_t)(a + b);
}

int16_t subtract_fixed(int16_t a, int16_t b) {
    // Inputs share the same q [cite: 158]
    return (int16_t)(a - b);
}

int16_t multiply_fixed(int16_t a, int16_t b, int16_t q) {
    // Use wider type for intermediate product to avoid overflow 
    int64_t raw_temp = (int64_t)a * (int64_t)b;
    
    // Shift right by q to return to the original Q-format 
    return (int16_t)(raw_temp >> q);
}

void eval_poly_ax2_minus_bx_plus_c_fixed(int16_t x, int16_t a, int16_t b, int16_t c, int16_t q) {
    // Evaluate y = a*x^2 - b*x + c [cite: 166]
    // Step 1: x^2
    int16_t x_squared = multiply_fixed(x, x, q);
    
    // Step 2: a * x^2
    int16_t a_x_squared = multiply_fixed(a, x_squared, q);
    
    // Step 3: b * x
    int16_t b_x = multiply_fixed(b, x, q);
    
    // Step 4: (a*x^2) - (b*x)
    int16_t term1 = subtract_fixed(a_x_squared, b_x);
    
    // Step 5: final y = term1 + c
    int16_t y = add_fixed(term1, c);

    // Print the required message exactly [cite: 168, 169]
    printf("the polynomial output for a=");
    print_fixed(a, q);
    printf(", b=");
    print_fixed(b, q);
    printf(", c=");
    print_fixed(c, q);
    printf(" is ");
    print_fixed(y, q);
    printf("\n");
}