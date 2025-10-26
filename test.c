#include <stdio.h>
#include <assert.h>
#include <math.h>

#define CALC_IMPLEMENTATION
#include "calc.h"

static void test(const char *expr, double expected)
{
	double result;
	int ret = calc_solve(expr, &result);
	if (ret != 0) {
		printf("FAIL: \"%s\": error %d\n", expr, ret);
		return;
	}
    
	if (fabs(result - expected) > 1e-9) {
		printf("FAIL: \"%s\": got %f, expected %f\n", expr, result, expected);
	} else {
		printf("PASS: \"%s\" = %f\n", expr, result);
	}
}

static void test_error(const char *expr)
{
	double result;
	int ret = calc_solve(expr, &result);

	if (ret == 0) {
		printf("FAIL: \"%s\": expected error, got %f\n", expr, result);
	} else {
		printf("PASS: \"%s\": correctly returned %d\n", expr, ret);
	}
}

int main(void)
{
	/* Basic arithmetic */
	test("1 + 2", 3.0);
	test("5 - 2", 3.0);
	test("3 * 4", 12.0);
	test("10 / 2", 5.0);

	/* Precedence */
	test("2 + 3 * 4", 14.0);
	test("2 * 3 + 4", 10.0);
	test("10 - 3 - 2", 5.0);

	/* Parentheses */
	test("(2 + 3) * 4", 20.0);
	test("10 / (5 - 3)", 5.0);

	/* Unary (not supported yet) */
	/* test("-5 + 3", -2.0); */
	/* test("+5 - 2", 3.0); */

	/* Complex */
	test("(2 - 1) / (1 + 4 * 2 - 5)", 0.25);
	test("3 + 4 * 2 / (1 - 5)", 1.0);

	/* Edge cases */
	test_error("5 / 0");
	test_error("2 + +");
	test_error("3 *");
	test_error(")1+2(");

	return 0;
}
