# Style Guide For The OS

## C

Adapted from the linux kernel coding style guide.
https://www.kernel.org/doc/html/v4.10/process/coding-style.html

### Indentation

Indentation is 4 spaces.

    if (condition) 
        do_this();


### Line Length

Lines should be no longer than 80 columns.

Lines longer than 80 columns should be broken into multiple lines unless they
are strings. Child lines should be indented in a reasonable manner.

    many_parameter_function(
        parameter1, parameter2, parameter3, parameter4, parameter5
    );


### Braces

For all non-function blocks, the opening brace is placed on the same line as
the start of the statement and the closing brace is on a line by itself except
where followed by a continuation of the same statement.

    if (condition) {
        true_body...
    } else {
        false_body...
    }

The opening brace of a function block is placed on a line by itself. This is
to distinguish more clearly between the function header and the function
body. If the function parameters span multiple lines then the opening brace is 
placed on a line with the closing parenthesis.

    void my_func(
        parameters
    ) {
        code;
    }

Do not use braces unnecessarily when a single statement will do. The statement 
should be on its own line unless it is a guard clause such as return, break, 
continue, or ensuring a variable is valid (such as non-zero).

    if (!guard_condition) return;

    if (condition)
        do_this();
    else
        do_that();

The exception to this is when one branch of a conditional requires braces. In
such a case use braces for all branches.

    if (condition) {
        line1;
        line2;
    } else {
        line3;
    }


### Spaces

Use spaces around all keywords that are not used similarly to a function.

Use spaces around:

    if, switch, case, for, do, while

Avoid space after: 

    sizeof, typeof, alignor, __attribute__

Do not add space inside parentheses.

Do:
    function_call(a);

Do not do:
    function_call( a );

Place the space before '*' for pointer declarations. Place a space on both
sides of '*' for pointer declarations if the '*' does not immediately precede
the variable name;

    int *a;
    FILE * restrict stream;

Use one space on both sides of binary and ternary operators. Do not use a space
after unary operators and the prefix increment and decrement operators. Do not
use a space before the postfix increment and decrement operators. Do not use 
any space around the . and -> operators.

    a.b->c = --c + !d;


### Naming

Function names should use snake case:

    void my_func(parameters);

Variables and members should use snake case:

    my_variable = 0;

Struct types should use pascal case:

    typedef struct {
        some_member
    } SomeStruct;

Enums should use pascal case and members should be in all caps:

    enum SomeEnum {
        ITEM_1
    };

Constant macros should be all caps and function macros should use snake case:

    #define SOME_CONSTANT 0xFF
    #define double_it(a) (2*a)

Exceptions to this naming scheme are allowed in order to match the C standard.

### Includes

The header file for an implementation file should be listed first with a blank
row following. All other headers should be listed in alphabetical order.

    #include "vga_text.h"

    #include "io.h"
    #include <stdbool.h>

