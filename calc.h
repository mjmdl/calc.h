#ifndef CALC_H
#define CALC_H

/**
 * Solves a mathematical expression.
 * @param expression to be solved.
 * @param result of the expression.
 * @returns 0 on success.
 */
int calc_solve(const char *expression, double *result);

#endif
#ifdef CALC_IMPLEMENTATION
#undef CALC_IMPLEMENTATION

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

enum calc__symbol {
	CALC__OPAREN   = '(',
	CALC__CPAREN   = ')',
	CALC__SUM      = '+',
	CALC__SUBTRACT = '-',
	CALC__MULTIPLY = '*',
	CALC__DIVIDE   = '/',
	CALC__NUMBER,
};

struct calc__token {
	enum calc__symbol symbol;
	double            value;
};

struct calc__stack {
	size_t              capacity;
	size_t              count;
	struct calc__token *tokens;
};

static void calc__destroy_stack(struct calc__stack *stack)
{
	if (stack->tokens != NULL) free(stack->tokens);
}

static int calc__create_stack(struct calc__stack *stack)
{
	size_t capacity = 10;
	size_t size     = capacity * sizeof stack->tokens[0];

	stack->tokens = (struct calc__token *)malloc(size);
	if (stack->tokens == NULL) return -1;

	stack->capacity = capacity;
	stack->count    = 0;
	return 0;
}

static int calc__stack_grow(struct calc__stack *stack)
{
	size_t  capacity = stack->capacity * 2;
	size_t  size     = capacity * sizeof stack->tokens[0];
	void   *memory   = (struct calc__token *)realloc(stack->tokens, size);
	if (memory == NULL) return -1;

	stack->capacity = capacity;
	stack->tokens   = memory;
	return 0;
}

static int calc__stack_push(struct calc__stack *stack, struct calc__token *token)
{
	if (stack->count >= stack->capacity) {
		if (calc__stack_grow(stack) != 0) return -1;
	}

	struct calc__token *new_token = stack->tokens + stack->count;
	new_token->symbol = token->symbol;
	new_token->value  = token->value;
	
	++stack->count;
	return 0;
}

static int calc__stack_pop(struct calc__stack *stack, struct calc__token *out_token)
{
	if (stack->count == 0) return -1;

	struct calc__token *token = stack->tokens + (stack->count - 1);
	out_token->symbol = token->symbol;
	out_token->value  = token->value;

	--stack->count;
	return 0;
}

static int calc__parse_number(const char **cursor, struct calc__stack *input)
{
	char *end;
	double value = strtod(*cursor, &end);
	*cursor = end;

	struct calc__token token = {.symbol = CALC__NUMBER, .value = value};
	if (calc__stack_push(input, &token) != 0) return -1;
	
	return 0;
}

static int calc__oper_order(enum calc__symbol oper)
{
	switch (oper) {
	case CALC__OPAREN:   return 3;
	case CALC__MULTIPLY:
	case CALC__DIVIDE:   return 2;
	case CALC__SUM:
	case CALC__SUBTRACT: return 1;
	default:             return -1;
	}
}

static bool calc__has_preceding_order(enum calc__symbol first, enum calc__symbol second)
{
	if (first == CALC__OPAREN || first == CALC__CPAREN) return false;
	return calc__oper_order(first) >= calc__oper_order(second);
}

static int calc__drain_preceding_opers(enum calc__symbol symbol, struct calc__stack *input, struct calc__stack *opers)
{
	if (opers->count == 0) return 0;

	while (opers->count > 0 && calc__has_preceding_order(opers->tokens[opers->count - 1].symbol, symbol)) {
		struct calc__token oper;
			
		if (calc__stack_pop(opers, &oper) != 0)  return -1;
		if (calc__stack_push(input, &oper) != 0) return -1;
	}
	
	return 0;
}

static int calc__drain_paren(struct calc__stack *input, struct calc__stack *opers)
{
	while (opers->count > 0) {
		struct calc__token oper;

		if (calc__stack_pop(opers, &oper) != 0)  return -1;
		if (oper.symbol == CALC__OPAREN)         return 0;
		if (calc__stack_push(input, &oper) != 0) return -1;
	}
	
	return -1;
}

static int calc__parse_oper(enum calc__symbol symbol, struct calc__stack *input, struct calc__stack *opers)
{
	if (symbol == CALC__CPAREN) {
		if (calc__drain_paren(input, opers) != 0) return -1;
		return 0;
	}

	if (calc__drain_preceding_opers(symbol, input, opers) != 0) return -1;
	
	struct calc__token new_oper = {.symbol = symbol};
	if (calc__stack_push(opers, &new_oper) != 0) return -1;
	
	return 0;
}

static bool calc__is_digit(char chr)
{
	return '0' <= chr && chr <= '9';
}

static bool calc__is_oper(char chr)
{
	return chr == '(' ||
	       chr == ')' ||
	       chr == '+' ||
	       chr == '-' ||
	       chr == '*' ||
	       chr == '/';
}

static bool calc__is_whitespace(char chr)
{
	return chr == ' '  ||
	       chr == '\t' ||
	       chr == '\n';
}

static int calc__parse(const char *expr, struct calc__stack *input)
{
	struct calc__stack opers;
	if (calc__create_stack(&opers) != 0) goto fault;

	for (const char *cursor = expr; *cursor != '\0';) {
		if (calc__is_whitespace(*cursor)) { ++cursor; continue; }
		
		if (calc__is_digit(*cursor)) {
			if (calc__parse_number(&cursor, input) != 0) goto fault;
			continue;
		}

		if (calc__is_oper(*cursor)) {
			if (calc__parse_oper((enum calc__symbol)*cursor, input, &opers) != 0) goto fault;
			++cursor;
			continue;
		}

		return -1;
	}

	while (opers.count > 0) {
		struct calc__token oper;
		
		if (calc__stack_pop(&opers, &oper) != 0) goto fault;
		if (calc__stack_push(input, &oper) != 0) goto fault;
	}

	calc__destroy_stack(&opers);
	return 0;

fault:
	calc__destroy_stack(&opers);
	return -1;
}

static int calc__evaluate(const struct calc__stack *input, double *result)
{
	struct calc__stack output;
	if (calc__create_stack(&output) != 0) goto fault;

	for (size_t i = 0; i < input->count; ++i) {
		struct calc__token *token = input->tokens + i;

		if (token->symbol == CALC__NUMBER) {
			calc__stack_push(&output, token);
			continue;
		}

		struct calc__token left = {0}, right = {0};
		if (calc__stack_pop(&output, &right) != 0) goto fault;
		if (calc__stack_pop(&output, &left) != 0) goto fault;

		struct calc__token new_token = {.symbol = CALC__NUMBER};
		switch (token->symbol) {
		case CALC__SUM:      new_token.value = left.value + right.value; break;
		case CALC__SUBTRACT: new_token.value = left.value - right.value; break;
		case CALC__MULTIPLY: new_token.value = left.value * right.value; break;
		case CALC__DIVIDE:
			if (right.value == 0.0) goto fault;
			new_token.value = left.value / right.value;
			break;
		default: goto fault;
		}
		
		if (calc__stack_push(&output, &new_token) != 0) goto fault;
	}

	if (output.count != 1) goto fault;
	struct calc__token last = {0};
	if (calc__stack_pop(&output, &last) != 0) goto fault;
	*result = last.value;
	
	calc__destroy_stack(&output);
	return 0;

fault:
	calc__destroy_stack(&output);
	return -1;
}

int calc_solve(const char *expression, double *result)
{
	if (expression == NULL || result == NULL) return 1;
	
	struct calc__stack input = {0};
	if (calc__create_stack(&input) != 0) return -1;

	if (calc__parse(expression, &input) != 0) goto fault;
	if (calc__evaluate(&input, result) != 0)  goto fault;
		
	calc__destroy_stack(&input);
	return 0;

fault:
	calc__destroy_stack(&input);
	return -1;
}

#endif
