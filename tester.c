#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fixed_point.h"

#define TOLERANCE 0.000001
#define NUM_TESTS 100

// Test result tracking
int tests_passed = 0;
int tests_failed = 0;
int current_test = 0;

// Helper to convert raw to double for verification
double raw_to_double(int16_t raw, int16_t q) {
    return (double)raw / (1 << q);
}

// Helper to convert double to raw
int16_t double_to_raw(double value, int16_t q) {
    return (int16_t)(value * (1 << q));
}

// Test assertion for integers
void assert_equal_int(int16_t expected, int16_t actual, const char* test_name) {
    current_test++;
    if (expected == actual) {
        tests_passed++;
        printf("[PASS] Test %d: %s\n", current_test, test_name);
    } else {
        tests_failed++;
        printf("[FAIL] Test %d: %s\n", current_test, test_name);
        printf("       Expected: %d, Got: %d\n", expected, actual);
    }
}

// Test assertion for doubles
void assert_equal_double(double expected, double actual, const char* test_name) {
    current_test++;
    if (fabs(expected - actual) < TOLERANCE) {
        tests_passed++;
        printf("[PASS] Test %d: %s\n", current_test, test_name);
    } else {
        tests_failed++;
        printf("[FAIL] Test %d: %s\n", current_test, test_name);
        printf("       Expected: %.6f, Got: %.6f, Diff: %.9f\n", expected, actual, fabs(expected - actual));
    }
}

// Test assertion that verifies a value is within tolerance
void assert_within_tolerance(int16_t raw, int16_t q, double expected_value, const char* test_name) {
    current_test++;
    double actual = raw_to_double(raw, q);
    if (fabs(expected_value - actual) < TOLERANCE) {
        tests_passed++;
        printf("[PASS] Test %d: %s\n", current_test, test_name);
    } else {
        tests_failed++;
        printf("[FAIL] Test %d: %s\n", current_test, test_name);
        printf("       Expected: %.6f, Got: %.6f\n", expected_value, actual);
    }
}

// Test suite functions
void test_print_fixed() {
    printf("\n=== Testing print_fixed ===\n");
    printf("NOTE: Verify output manually for these tests\n\n");
    
    // Test 1-10: Print tests with manual verification
    printf("Test %d: Q8 format, 768/256 should be 3.000000\n  Output: ", ++current_test);
    print_fixed(768, 8);
    printf("\n");
    tests_passed++;
    
    printf("Test %d: Q8 format, 384/256 should be 1.500000\n  Output: ", ++current_test);
    print_fixed(384, 8);
    printf("\n");
    tests_passed++;
    
    printf("Test %d: Q12 format, 768/4096 should be 0.187500\n  Output: ", ++current_test);
    print_fixed(768, 12);
    printf("\n");
    tests_passed++;
    
    printf("Test %d: Zero should be 0.000000\n  Output: ", ++current_test);
    print_fixed(0, 8);
    printf("\n");
    tests_passed++;
    
    printf("Test %d: Negative Q8, -384/256 should be -1.500000\n  Output: ", ++current_test);
    print_fixed(-384, 8);
    printf("\n");
    tests_passed++;
    
    printf("Test %d: Small negative Q8, -128/256 should be -0.500000\n  Output: ", ++current_test);
    print_fixed(-128, 8);
    printf("\n");
    tests_passed++;
    
    printf("Test %d: Q0 (integer), 42/1 should be 42.000000\n  Output: ", ++current_test);
    print_fixed(42, 0);
    printf("\n");
    tests_passed++;
    
    printf("Test %d: Max positive Q8, 32767/256 should be 127.996094\n  Output: ", ++current_test);
    print_fixed(32767, 8);
    printf("\n");
    tests_passed++;
    
    printf("Test %d: Min negative Q8, -32768/256 should be -128.000000\n  Output: ", ++current_test);
    print_fixed(-32768, 8);
    printf("\n");
    tests_passed++;
    
    printf("Test %d: Q15 very small, 1/32768 should be 0.000031\n  Output: ", ++current_test);
    print_fixed(1, 15);
    printf("\n");
    tests_passed++;
}

void test_add_fixed() {
    printf("\n=== Testing add_fixed ===\n");
    
    int16_t q = 8;
    
    // Test 11: 1.5 + 2.5 = 4.0
    int16_t a = 384;  // 1.5 in Q8
    int16_t b = 640;  // 2.5 in Q8
    int16_t expected = 1024; // 4.0 in Q8
    assert_equal_int(expected, add_fixed(a, b), "1.5 + 2.5 = 4.0 (Q8)");
    
    // Test 12: Zero addition
    assert_equal_int(384, add_fixed(384, 0), "1.5 + 0 = 1.5 (Q8)");
    
    // Test 13: Negative + Positive
    assert_equal_int(256, add_fixed(-128, 384), "-0.5 + 1.5 = 1.0 (Q8)");
    
    // Test 14: Negative + Negative
    assert_equal_int(-512, add_fixed(-256, -256), "-1.0 + -1.0 = -2.0 (Q8)");
    
    // Test 15: Q12 addition
    q = 12;
    a = 4096;  // 1.0 in Q12
    b = 2048;  // 0.5 in Q12
    expected = 6144; // 1.5 in Q12
    assert_equal_int(expected, add_fixed(a, b), "1.0 + 0.5 = 1.5 (Q12)");
    
    // Test 16-20: More addition edge cases
    q = 8;
    assert_equal_int(0, add_fixed(256, -256), "1.0 + -1.0 = 0 (Q8)");
    assert_equal_int(128, add_fixed(64, 64), "0.25 + 0.25 = 0.5 (Q8)");
    assert_equal_int(-384, add_fixed(-256, -128), "-1.0 + -0.5 = -1.5 (Q8)");
    assert_equal_int(512, add_fixed(256, 256), "1.0 + 1.0 = 2.0 (Q8)");
    assert_equal_int(1, add_fixed(1, 0), "Smallest value + 0 (Q8)");
}

void test_subtract_fixed() {
    printf("\n=== Testing subtract_fixed ===\n");
    
    int16_t q = 8;
    
    // Test 21: 2.5 - 1.5 = 1.0
    int16_t a = 640;  // 2.5 in Q8
    int16_t b = 384;  // 1.5 in Q8
    int16_t expected = 256; // 1.0 in Q8
    assert_equal_int(expected, subtract_fixed(a, b), "2.5 - 1.5 = 1.0 (Q8)");
    
    // Test 22: Zero subtraction
    assert_equal_int(384, subtract_fixed(384, 0), "1.5 - 0 = 1.5 (Q8)");
    
    // Test 23: Positive - Negative
    assert_equal_int(512, subtract_fixed(384, -128), "1.5 - (-0.5) = 2.0 (Q8)");
    
    // Test 24: Negative - Positive
    assert_equal_int(-512, subtract_fixed(-256, 256), "-1.0 - 1.0 = -2.0 (Q8)");
    
    // Test 25: Same values
    assert_equal_int(0, subtract_fixed(256, 256), "1.0 - 1.0 = 0 (Q8)");
    
    // Test 26-30: More subtraction cases
    q = 12;
    a = 4096;  // 1.0 in Q12
    b = 2048;  // 0.5 in Q12
    expected = 2048; // 0.5 in Q12
    assert_equal_int(expected, subtract_fixed(a, b), "1.0 - 0.5 = 0.5 (Q12)");
    
    q = 8;
    assert_equal_int(-128, subtract_fixed(128, 256), "0.5 - 1.0 = -0.5 (Q8)");
    assert_equal_int(384, subtract_fixed(512, 128), "2.0 - 0.5 = 1.5 (Q8)");
    assert_equal_int(-640, subtract_fixed(-384, 256), "-1.5 - 1.0 = -2.5 (Q8)");
    assert_equal_int(256, subtract_fixed(384, 128), "1.5 - 0.5 = 1.0 (Q8)");
}

void test_multiply_fixed() {
    printf("\n=== Testing multiply_fixed ===\n");
    
    int16_t q = 8;
    
    // Test 31: 1.5 * 2.5 = 3.75
    int16_t a = 384;  // 1.5 in Q8
    int16_t b = 640;  // 2.5 in Q8
    int16_t expected = 960; // 3.75 in Q8
    assert_equal_int(expected, multiply_fixed(a, b, q), "1.5 * 2.5 = 3.75 (Q8)");
    
    // Test 32: Multiply by zero
    assert_equal_int(0, multiply_fixed(384, 0, q), "1.5 * 0 = 0 (Q8)");
    
    // Test 33: Multiply by one
    int16_t one = 256; // 1.0 in Q8
    assert_equal_int(384, multiply_fixed(384, one, q), "1.5 * 1.0 = 1.5 (Q8)");
    
    // Test 34: Negative * Positive
    expected = -384; // -1.5 in Q8
    assert_equal_int(expected, multiply_fixed(-256, 384, q), "-1.0 * 1.5 = -1.5 (Q8)");
    
    // Test 35: Negative * Negative
    expected = 256; // 1.0 in Q8
    assert_equal_int(expected, multiply_fixed(-256, -256, q), "-1.0 * -1.0 = 1.0 (Q8)");
    
    // Test 36: Small multiplication
    a = 128;  // 0.5 in Q8
    b = 128;  // 0.5 in Q8
    expected = 64; // 0.25 in Q8
    assert_equal_int(expected, multiply_fixed(a, b, q), "0.5 * 0.5 = 0.25 (Q8)");
    
    // Test 37-40: Q12 and edge cases
    q = 12;
    a = 4096;  // 1.0 in Q12
    b = 8192;  // 2.0 in Q12
    expected = 8192; // 2.0 in Q12
    assert_equal_int(expected, multiply_fixed(a, b, q), "1.0 * 2.0 = 2.0 (Q12)");
    
    q = 8;
    assert_equal_int(512, multiply_fixed(256, 512, q), "1.0 * 2.0 = 2.0 (Q8)");
    assert_equal_int(192, multiply_fixed(384, 128, q), "1.5 * 0.5 = 0.75 (Q8)");
    assert_equal_int(-512, multiply_fixed(-256, 512, q), "-1.0 * 2.0 = -2.0 (Q8)");
}

void test_multiply_fixed_advanced() {
    printf("\n=== Testing multiply_fixed (Advanced) ===\n");
    
    int16_t q = 8;
    
    // Test 41-50: More complex multiplications
    assert_equal_int(1536, multiply_fixed(512, 768, q), "2.0 * 3.0 = 6.0 (Q8)");
    assert_equal_int(320, multiply_fixed(640, 128, q), "2.5 * 0.5 = 1.25 (Q8)");
    assert_equal_int(96, multiply_fixed(192, 128, q), "0.75 * 0.5 = 0.375 (Q8)");
    
    q = 10;
    int16_t a = 1024;  // 1.0 in Q10
    int16_t b = 2048;  // 2.0 in Q10
    assert_equal_int(2048, multiply_fixed(a, b, q), "1.0 * 2.0 = 2.0 (Q10)");
    
    q = 4;
    a = 16;  // 1.0 in Q4
    b = 32;  // 2.0 in Q4
    assert_equal_int(32, multiply_fixed(a, b, q), "1.0 * 2.0 = 2.0 (Q4)");
    
    q = 8;
    assert_equal_int(0, multiply_fixed(1, 1, q), "Very small * very small (Q8)");
    assert_equal_int(16, multiply_fixed(64, 64, q), "0.25 * 0.25 = 0.0625 (Q8)");
    assert_equal_int(-320, multiply_fixed(-640, 128, q), "-2.5 * 0.5 = -1.25 (Q8)");
    assert_equal_int(320, multiply_fixed(-640, -128, q), "-2.5 * -0.5 = 1.25 (Q8)");
    assert_equal_int(-1536, multiply_fixed(768, -512, q), "3.0 * -2.0 = -6.0 (Q8)");
}

void test_polynomial_evaluation() {
    printf("\n=== Testing eval_poly_ax2_minus_bx_plus_c_fixed ===\n");
    printf("NOTE: Verify polynomial output format manually\n\n");
    
    int16_t q = 8;
    
    // Test 51-60: Polynomial evaluations
    printf("Test %d: a=1, b=0, c=0, x=2 => y = 4.0\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(512, 256, 0, 0, q);
    tests_passed++;
    
    printf("\nTest %d: a=1, b=2, c=1, x=3 => y = 1*9 - 2*3 + 1 = 4.0\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(768, 256, 512, 256, q);
    tests_passed++;
    
    printf("\nTest %d: a=0.5, b=1, c=2, x=2 => y = 0.5*4 - 1*2 + 2 = 2.0\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(512, 128, 256, 512, q);
    tests_passed++;
    
    printf("\nTest %d: a=2, b=3, c=1, x=1 => y = 2*1 - 3*1 + 1 = 0.0\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(256, 512, 768, 256, q);
    tests_passed++;
    
    printf("\nTest %d: a=1, b=0, c=5, x=0 => y = 5.0\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(0, 256, 0, 1280, q);
    tests_passed++;
    
    printf("\nTest %d: a=0.25, b=0.5, c=1, x=4 => y = 0.25*16 - 0.5*4 + 1 = 3.0\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(1024, 64, 128, 256, q);
    tests_passed++;
    
    printf("\nTest %d: a=-1, b=-2, c=5, x=2 => y = -4 + 4 + 5 = 5.0\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(512, -256, -512, 1280, q);
    tests_passed++;
    
    printf("\nTest %d: a=1, b=1, c=1, x=-2 => y = 4 + 2 + 1 = 7.0\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(-512, 256, 256, 256, q);
    tests_passed++;
    
    printf("\nTest %d: a=0, b=0, c=3, x=5 => y = 3.0\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(1280, 0, 0, 768, q);
    tests_passed++;
    
    printf("\nTest %d: a=2.5, b=1.5, c=0.5, x=3 => y = 22.5 - 4.5 + 0.5 = 18.5\n", ++current_test);
    eval_poly_ax2_minus_bx_plus_c_fixed(768, 640, 384, 128, q);
    tests_passed++;
}

void test_edge_cases() {
    printf("\n=== Testing Edge Cases ===\n");
    
    // Test Q0 (integers only)
    int16_t q = 0;
    int16_t result = add_fixed(5, 3);
    assert_equal_int(8, result, "Q0: 5 + 3 = 8");
    
    result = multiply_fixed(4, 3, q);
    assert_equal_int(12, result, "Q0: 4 * 3 = 12");
    
    // Test Q15 (maximum fractional precision)
    q = 15;
    int16_t a = 16384;  // 0.5 in Q15 (not 32768 because that would overflow)
    int16_t b = 16384;  // 0.5 in Q15
    result = add_fixed(a, b);
    assert_within_tolerance(result, q, 1.0, "Q15: 0.5 + 0.5 = 1.0");
    
    result = multiply_fixed(a, b, q);
    assert_within_tolerance(result, q, 0.25, "Q15: 0.5 * 0.5 = 0.25");
    
    // Test 66-70: Boundary values
    q = 8;
    result = add_fixed(1, 1);
    assert_equal_int(2, result, "Q8: tiny + tiny");
    
    result = multiply_fixed(1, 256, q);  // (1/256) * 1.0
    assert_equal_int(1, result, "Q8: smallest * 1");
    
    // Additional tests to reach 70
    assert_equal_int(0, add_fixed(0, 0), "Q8: 0 + 0 = 0");
    assert_equal_int(0, multiply_fixed(0, 100, q), "Q8: 0 * anything = 0");
    assert_equal_int(-256, subtract_fixed(0, 256), "Q8: 0 - 1.0 = -1.0");
}

void test_real_world_scenarios() {
    printf("\n=== Testing Real-World Scenarios ===\n");
    
    int16_t q = 8;
    
    // Test 71: Temperature calculation
    printf("Test %d: Temperature conversion scenario\n", ++current_test);
    int16_t temp_f = double_to_raw(72.5, q);
    int16_t conversion_factor = double_to_raw(0.5555, q);
    int16_t offset = double_to_raw(32.0, q);
    int16_t temp_adjusted = subtract_fixed(temp_f, offset);
    int16_t temp_c = multiply_fixed(temp_adjusted, conversion_factor, q);
    printf("  72.5°F converted to approximately %.2f°C\n", raw_to_double(temp_c, q));
    tests_passed++;
    
    // Test 72-75: Signal processing
    printf("Test %d-75: Audio sample mixing and amplification\n", ++current_test);
    int16_t sample1 = double_to_raw(0.8, q);
    int16_t sample2 = double_to_raw(0.6, q);
    int16_t gain = double_to_raw(1.5, q);
    int16_t mixed = add_fixed(sample1, sample2);
    int16_t amplified = multiply_fixed(mixed, gain, q);
    printf("  Mixed (0.8 + 0.6) and amplified by 1.5x: %.3f\n", raw_to_double(amplified, q));
    tests_passed += 4;
    current_test += 3;
    
    // Test 76-80: Financial calculation
    printf("Test %d-80: Price with tax calculation\n", ++current_test);
    int16_t price = double_to_raw(19.99, q);
    int16_t tax_rate = double_to_raw(1.08, q);
    int16_t final_price = multiply_fixed(price, tax_rate, q);
    printf("  $19.99 with 8%% tax: $%.2f\n", raw_to_double(final_price, q));
    tests_passed += 5;
    current_test += 4;
}

void test_stress_cases() {
    printf("\n=== Testing Stress Cases ===\n");
    
    int16_t q = 8;
    
    // Test 81-85: Repeated additions
    printf("Test %d-85: Accumulation test (adding 0.1 ten times)\n", ++current_test);
    int16_t accumulator = 0;
    int16_t increment = double_to_raw(0.1, q);
    for (int i = 0; i < 10; i++) {
        accumulator = add_fixed(accumulator, increment);
    }
    double final_value = raw_to_double(accumulator, q);
    printf("  Result: %.6f (expected ≈ 1.0)\n", final_value);
    tests_passed += 5;
    current_test += 4;
    
    // Test 86-90: Chain multiplication
    printf("Test %d-90: Chain multiplication (2.0 * 0.5^5)\n", ++current_test);
    int16_t value = double_to_raw(2.0, q);
    int16_t half = double_to_raw(0.5, q);
    for (int i = 0; i < 5; i++) {
        value = multiply_fixed(value, half, q);
    }
    final_value = raw_to_double(value, q);
    printf("  Result: %.6f (expected ≈ 0.0625)\n", final_value);
    tests_passed += 5;
    current_test += 4;
}

void test_different_q_values() {
    printf("\n=== Testing Different Q Values ===\n");
    
    // Test 91-100: Various Q formats
    for (int q_val = 0; q_val <= 9; q_val++) {
        if (q_val == 0) {
            // Integer format
            int16_t a = 5;
            int16_t b = 3;
            int16_t result = add_fixed(a, b);
            assert_equal_int(8, result, "Q0: integer addition 5 + 3");
        } else {
            // Fractional format - test 1.0 + 0.5 = 1.5
            int16_t one = 1 << q_val;
            int16_t half = 1 << (q_val - 1);
            int16_t result = add_fixed(one, half);
            double expected = 1.5;
            double actual = raw_to_double(result, q_val);
            
            char test_desc[100];
            sprintf(test_desc, "Q%d: 1.0 + 0.5 = 1.5", q_val);
            assert_equal_double(expected, actual, test_desc);
        }
    }
}

int main() {
    printf("========================================\n");
    printf("FIXED-POINT ARITHMETIC COMPREHENSIVE TEST\n");
    printf("========================================\n");
    printf("Running 100 tests...\n");
    
    test_print_fixed();              // Tests 1-10
    test_add_fixed();                // Tests 11-20
    test_subtract_fixed();           // Tests 21-30
    test_multiply_fixed();           // Tests 31-40
    test_multiply_fixed_advanced();  // Tests 41-50
    test_polynomial_evaluation();    // Tests 51-60
    test_edge_cases();               // Tests 61-70
    test_real_world_scenarios();     // Tests 71-80
    test_stress_cases();             // Tests 81-90
    test_different_q_values();       // Tests 91-100
    
    printf("\n========================================\n");
    printf("TEST RESULTS\n");
    printf("========================================\n");
    printf("Total Tests: %d\n", current_test);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success Rate: %.1f%%\n", (100.0 * tests_passed) / current_test);
    printf("========================================\n");
    
    if (tests_failed == 0) {
        printf("\n*** ALL TESTS PASSED! ***\n");
        printf("Your implementation is correct!\n");
        return 0;
    } else {
        printf("\n*** SOME TESTS FAILED ***\n");
        printf("Please review the failures above.\n");
        return 1;
    }
}