#include <stdio.h>

#define CALC_IMPLEMENTATION
#include "calc.h"

int main(void)
{
	const char *expr = "1 / 10 + (2 * 100.2 + 28 + 0.06) / 2 + 3 * 102.12";
	double result = 0.0;
	calc_solve(expr, &result);
	printf("%s = %.2f\n", expr, result);
	return 0;
}
