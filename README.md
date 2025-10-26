# calc.h

Single-header math solver library.

## Usage

* Include `"calc.h"` wherever you need it.
* Define `CALC_IMPLEMENTATION` in **exactly one source file**, right before `#include "calc.h"`.
* Use `calc_solve()` to evaluate math expressions.

## Example

See [example.c](./example.c).

## Features

* Basic arithmetic: `+ - * /`
* Parentheses precedence

## ToDo

* Unary operators (`-`, `!`, etc.)
* Constants (`pi`, `e`, `i`, etc.)
* Functions (`sqrt`, `cos`, `sin`, etc.)
* User variables (e.g., `x`, `y`, `z`)
* Support for other brackets (`[]`, `{}`)
* Ternary operator (`?:`)
* Error codes
