# IPL Interpreter

For this project, I've implemented an interpreter for [IPL](https://github.com/GeorgeSittas/ipl-interpreter/blob/main/specification.pdf),
a small, imperative language created for educational purposes in the Introduction to Programming course ([k04](http://cgi.di.uoa.gr/~ip/)),
using some of the techniques I learned from the book [Crafting Interpreters](https://craftinginterpreters.com/). The language is briefly
described as follows:

### Types

The language only supports integers, as well as arrays of integers.

### Variables

Since there are only integer variables in IPL, they don't need to be declared before their use, and each uninitialized variable is
implicitly initialized to 0 on its first use. Additionally, variable names can't consist of more than 100 characters and they must start
with a letter (upper or lower case), (possibly) followed by a string that contains letters, digits and underscores (in regex terms:
`Identifier := [a-zA-Z][a-zA-Z0-9_]*`).

### Constants

Only non-negative integer constants are allowed in IPL, so for example -5 is disallowed while 6 is allowed in the program text.
Ofcourse, one can produce negative values with the help of subtraction (eg. -5 = 0 - 5).

### Input

The built-in command `read <lvalue>` is provided for reading integer values into `<lvalue>`, which can be either a variable or
an array element.

### Output

The built-in commands `write <expr>` and `writeln <expr>` are provided for outputing integer values, where `<expr>` can be either
a constant, a variable or an array element. The former outputs a trailing space, while the latter a newline. These can also be
used without an argument, in which case a single space or newline character will be printed.

### Arithmetic Expressions

The language only supports binary arithmetic expressions, where each of the two operands can be either a constant, a variable or an
element array. The supported operators are: +, -, /, * and %, and they have the same semantics as in C.

### Assignment

One can assign an integer value to a variable or to an array element by using the assignment statement, which follows the
syntax `<lvalue> = <expr>`. Here, `<expr`> can be either a constant, a variable, an array element or an arithmetic expression.

### Conditions

Conditions follow the same format as the arithmetic expressions, but they can only be used in control-flow constructs like
if-else and while statements (see below). The supported operators are: ==, !=, <=, <, >= and >, and they have the same semantics
as in C.

### While loop

This is the only statement that can be used for creating loops in IPL, and its syntax is as follows:
```c
while <condition>
<tab> <statement1>
<tab> <statement2>
....
```
That is, the loop's body is a block of statements that are indented by exactly one more tab to the right.

### Branching

Similar to the while loop statement, IPL provides a branching if-else statement, which has similar syntax:
```c
if <condition>
<tab> <if_statement1>
<tab> <if_statement2>
....
else
<tab> <else_statement1>
<tab> <else_statement2>
....
```
Note here, that the else clause is optional, so one can omit it if he desires.

### Random Numbers

The built-in command `random <lvalue>` is provided for generating randon integers, which are stored in `<lvalue>`
(either a variable or an array statement).

### Comments

A comment in IPL starts with the # character and ends when a newline character is found. Multi-line comments are
not supported.

### Command Line Arguments

The built-in command `argument size <lvalue>` can be used in order to store the number of arguments passed to an
IPL program in `<lvalue>` (either a variable or an array element). The input file is not counted as an argument.
Additionally, one can use the built-in command `argument <expr> <lvalue>` to store an integer argument in `<lvalue>`
(same as above), where `<expr>` is either a constant, a variable or an array element and represents the argument
index. If the index is out-of-bounds, a runtime error is raised.

### Break and Continue

The built-in commands `break <n>` and `continue <n>` have the same semantics as break and continue in C, if `<n> = 1`,
otherwise they "go up" `<n>` loops, where `<n>` is a positive integer. The following example will probably help to
understand them better:

```c
while a < 5
  a = a + 1
  write a
  if a == 3
    break
  while b < 20
    b = b + 1
    write b
    c = b % 5
    if c == 0
      continue 2
writeln
```

`Output: 1 1 2 3 4 5 2 6 7 8 9 10 3`

### Arrays

Integer arrays can be created as `new <name>[<expr>]`, where `<name>` is the array's identifier and `<expr>` can be either
a constant, a variable or an array element (different than zero, in any of these cases), representing the array's
dimension. Arrays and variables must have different names. All uninitialized array elements are implicitly
initialized to 0, similar to how variables work. The array's memory can be collected with the free statement, as in
`free <name>`, where `<name>` is the identifier of the array-to-be-deallocated. An array element reference works just like
in C: `<name>[<expr>]`, where `<expr>` can be either a constant, a variable or an array element (again, an out-of-bounds
index will raise a runtime error). Finally, one can use the built-in command `size <name> <lvalue>` to store the size of
the array referred to by `<name>` in `<lvalue>`, which can be either a variable or an array element.

## Compilation

```Bash
make       // produces the executable "ipli"
make clean // clean the directory of object files & executable
```

## Extra Notes

The interpreter hasn't been thoroughly tested, so it's very likely that it contains bugs. Feel free to create an issue
if you find an interesting case or even contribute to the project by creating a pull request. The following assumptions
have been made:

- A command can contain an arbitrary number of whitespace (spaces or tabs) between variables, operators, etc.
- The indentation of a command is determined by the number of tabs at the beginning of each line.

## To-Do List

- [ ] Create a test-suite for the interpreter & automate testing.
- [ ] Add support for the verbose mode
