#include<frontend/c/parser.h>
#include<datastructures/stack.h>
#include<frontend/common/parser.h>
#include<backend/intermediate/intermediate.h>

typedef u8 prio;

/* The logically highest value of the "prio" type. */
#define __PRIO_MAX__ ((prio)-1)

/*
 * Operators are "intermediate_kind" expect for "OPENING_PAR" and "CLOSING_PAR"
 * which are just temp anyway.
 */
typedef u8 operator;

#define OPENING_PAR 20
#define CLOSING_PAR 21


/* The logically highest value of the "operator" type. */
#define __OPERATOR_MAX__ ((operator)-1)

/* Stack of operators. Operators are in the pointers of the nodes. */
static stack operator_stack = { NULLPTR, sizeof(operator) };

static inline bool C_parse_operation(vector* file, u32 index);
static inline prio C_get_operator_prio(operator _operator);
static inline operator C_get_operator(char* token);

/*
 * This function reads an operand from the file, adds it to the intermediates,
 * and returns the ending index of the operands so the index will have to be
 * incramented once to reach the next token that is not part of the operand.
 */
u32 C_parse_operand(vector* file, u32 starting_index)
{
    char* first_token = vector_at(file, starting_index, false);

    /* ASCII numbers */
    if (add_if_ascii_num(first_token))
        return starting_index;

    /* Variables */
    if (!is_invalid_name(first_token)
    && get_variable_symbol(first_token, 0)) {
        intermediate _tmp_intermediate = \
        { VAR_ACCESS, (void*)get_variable_symbol(first_token, 0)->hash };

        add_operand(_tmp_intermediate, false);
        return starting_index;
    }

    // FUNC CALLS
    // CASTS
    // VARIABLES
}

/*
 * This function takes in the starting index of an operation and sends the
 * contents into "backend/intermediate/intermediate.c" in RPN.
 */
static inline bool C_parse_operation(vector* file, u32 starting_index)
{
    /*
    - Parse the equation:
        - If the next operator <= the presendence of the last, add the last
            to the output and the current on to the stack.

        - If an opening parethesis is hit add it to the stack.

        - If the last operator was a parenthesis add the current operator to
            the output.

        - If the current operator is a closing parenthesis or the end of the
            operation add all the operators to the output.

        3 2 - 2 4 * 2 / -
        1 2 4 * 2 / -
        1 8 2 / -
        1 4 -
    */

    char** starting_token = vector_at(file, starting_index, false);

    for (char** current_token = starting_token ;; current_token++) {
        if (*current_token == NULLPTR)
            continue;
        if (*current_token == '\n')
            break;

        /* Reading operator. */
        operator current_operator = C_get_operator(current_token);
        if (current_operator == __OPERATOR_MAX__)
            printf("Expected to find an operator but found %s.\n", \
                current_token);

        /* Adding the operator to the output or stack. */
        if (stack_size(&operator_stack) == 0
        || C_get_operator_prio(current_operator) >= \
        C_get_operator_prio((operator)stack_top(&operator_stack)))
            process_operation(current_operator);
        else
            stack_push(&operator_stack, (void*)current_operator);

        /* Reading the operand. */
        C_parse_operand(file, starting_index);
    }
    while (stack_size(&operator_stack))
        process_operation((operator)stack_pop(&operator_stack));
}

/*
 * This either returns the operator of the token or __OPERATOR_MAX__ if it is
 * not a valid operator. Negation and subtraction need to be done seperate of
 * this.
 */
static inline operator C_get_operator(char* token)
{
    #define U16_CHAR(a,b) a | (b << 8)
    if (strlen(token) == 1) {
        switch(*token)
        {
        case '!':
            return NOT;
        case '~':
            return COMPLEMENT;
        case '+':
            return ADD;
        case '-':
            return SUB;
        case '*':
            return MUL;
        case '/':
            return DIV;
        case '&':
            return AND;
        case '^':
            return XOR;
        case '|':
            return OR;
        case '%':
            return MOD;
        case '>':
            return GREATER_THAN;
        case '<':
            return LESS_THAN;
        case '(':
            return OPENING_PAR;
        case ')':
            return CLOSING_PAR;
        }
    } else if (strlen(token) == 2) {
        switch(*(u16*)token)
        {
        case U16_CHAR('<','<'):
            return LSL;
        case U16_CHAR('>','>'):
            return LSR;
        case U16_CHAR('=','='):
            return IS_EQUAL;
        case U16_CHAR('!','='):
            return NOT_EQUAL;
        case U16_CHAR('>','='):
            return GREATER_THAN_EQUAL;
        case U16_CHAR('<','='):
            return LESS_THAN_EQUAL;
        }
    }
    return __OPERATOR_MAX__;
    #undef U16_CHAR
}

/* This returns the priority of the inputed operator. */
static inline prio C_get_operator_prio(operator _operator)
{
    switch(_operator)
    {
    case IS_EQUAL:
    case NOT_EQUAL:
        return 0;
    case GREATER_THAN:
    case GREATER_THAN_EQUAL:
    case LESS_THAN:
    case LESS_THAN_EQUAL:
        return 1;
    case OR:
    case XOR:
    case AND:
        return 2;
    case LSL:
    case LSR:
        return 3;
    case INC:
    case DEC:
    case MEM_LOCATION:
    case MEM_ACCESS:
    case ADD:
    case SUB:
    case COMPLEMENT:
    case NOT:
        return 4;
    case MUL:
    case DIV:
    case MOD:
        return 5;
    return __PRIO_MAX__;
    }
}

#undef CLOSING_PAR
#undef OPENING_PAR
#undef __PRIO_MAX__