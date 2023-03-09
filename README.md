# IPL Interpreter

IPL is a simple imperative language created for educational purposes in the [Introduction to Programming course](http://cgi.di.uoa.gr/~ip/). This
implementation uses some of the techniques described in the (amazing!) book [Crafting Interpreters](https://craftinginterpreters.com/).

## Usage

```Bash
# Compile the project
make

# Cleanup
make clean
```

## IPL Description

### Types

The language only supports integers and arrays of integers.

### Variables

Variables don't need to be declared before their use and they are implicitly initialized to `0` on their first use. Variable names
can contain at most 100 characters and they must start with a letter, followed by any number of letters, numbers or underscores.

### Constants

Only non-negative constants are allowed. Ofcourse, one can produce negative values by subtracting from 0, e.g. `-5 = 0 - 5`.

### Input

The built-in command `read <lvalue>` reads an integer value into `<lvalue>`, which can be either a variable or an array element.

### Output

The built-in commands `write <expr>` and `writeln <expr>` output the integer value `<expr>`: a constant, variable or array element.
The former outputs a trailing space, while the latter a newline. In case these are used without an argument, a single space or newline
character will be printed.

### Arithmetic Expressions

An arithmetic expression can contain at most two operands that can be either constants, variables or array elements.
The supported operators are `+`, `-`, `/`, `*` and `%`, having the same semantics as in C.

### Assignment

Integer values can be assigned to a variable or to an array element using the assignment statement: `<lvalue> = <expr>`.
Here, `<expr`> can be either a constant, a variable, an array element or an arithmetic expression.

### Conditions

Conditions follow the same format as the arithmetic expressions, but they can only be used in control-flow constructs like
if-else and while statements. The supported operators are `==`, `!=`, `<=`, `<`, `>=` and `>`, having the same semantics as in C.

### While loop

This is the only statement that can be used for creating loops in IPL:

```c
while <condition>
<tab> <statement1>
<tab> <statement2>
....
```

The language is indentation-sensitive, so tabs define blocks.

### Branching

Similar to the while loop statement, IPL provides an if-else statement (else clause is optional):

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

### Random Numbers

The built-in command `random <lvalue>` generates a random integer and stores it in `<lvalue>`.

### Comments

A comment in IPL starts with the # character and ends when a newline character is found.

### Command Line Arguments

The built-in command `argument size <lvalue>` stores the number of command line arguments in `<lvalue>`. The input
file is not counted as an argument. The built-in command `argument <expr> <lvalue>` stores an integer argument in
`<lvalue>`, where `<expr>` is either a constant, a variable or an array element and represents the argument index.
If the index is out-of-bounds, a runtime error is raised.

### Break and Continue

The built-in commands `break <n>` and `continue <n>` have the same semantics as break and continue in C when `<n> = 1`.
In all other cases, they jump `<n>` loops, where `<n>` is a positive integer. Below is an example:

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

Integer arrays can be created as `new <name>[<expr>]`, where `<expr>` is a non zero constant, variable or array
element representing the new array's dimension. Arrays and variables must have different names. All uninitialized
array elements are implicitly initialized to 0. The array's memory can be collected with the free statement, using
`free <name>`. An array element reference works just like in C: `<name>[<expr>]`, where `<expr>` can be either a
constant, a variable or an array element and an out-of-bounds index raises a runtime error. The built-in command
`size <name> <lvalue>` stores the size of the array referred to by `<name>` in `<lvalue>`.

### Whitespace

A command may contain an arbitrary number of spaces/tabs between variables, operators, etc. Tab characters in
particular are used to specify indentation for block statements.
