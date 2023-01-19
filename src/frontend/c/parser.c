#include<frontend/c/parser.h>
#include<datastructures/stack.h>
#include<frontend/common/parser.h>
#include<frontend/c/preprocessor.h>
#include<backend/intermediate/intermediate.h>
#include<backend/intermediate/struct.h>
#if DEBUG && linux
#include<time.h>
#endif

typedef u8 prio;

/* The logically highest value of the "prio" type. */
#define __PRIO_MAX__ ((prio)-1)

/*
 * Operators ids are the same as those of "intermediate_kind" expect for
 * "OPENING_PAR" and "CLOSING_PAR" which are just temp anyway.
 */
typedef u8 operator;

#define OPENING_PAR 22
#define CLOSING_PAR 23

/* The logically highest value of the "operator" type. */
#define __OPERATOR_MAX__ ((operator)-1)

/* Stack of operators. Operators are in the pointers of the nodes. */
static stack operator_stack = { NULLPTR, sizeof(operator) };

static inline bool C_parse_operation(char*** token);
prio C_get_operator_prio(operator _operator);
operator C_get_operator(char* token);
static inline void C_set_operator_associativity(operator *_operator, \
bool* left_associative, bool* right_associative);

/*
 * This function reads in the C source file refrenced by "file_name" processes
 * and turns it into intermediates.
 */
void C_file_into_intermediate(char* file_name)
{
    // TODO: This should be done outside of this function, preprocessing should
    // be optional.
    /* Preprocessing. */
    vector file = C_preprocess_file(file_name);
    if (file.contents == NULLPTR) {
        printf("Failed to preprocess C file: %s", file_name);
        exit(-1);
    }

    /* Initial intermediate pass. */
    #if DEBUG && linux
        clock_t starting_time = clock();
    #endif

    // struct
        // types
    // typedef
        // struct
        // enum
    // type
        // variable definition
        // function
    // lvalue = rvalue
    // if
        // (rvalue) {
    // else
    // while
        // (rvalue) {
        // continue
        // break
    // for
        // (lvalue = rvalue; condition; on_loop) {
            // continue
            // break
    // switch
        // case X:
        // default:
    // label:
        // return

    char** current_token = vector_at(&file, 0, false);
    char** end_token = vector_at(&file, VECTOR_SIZE(file)-1, false);

    for (; current_token <= end_token;) {
        /*
         * This catches the first line in the file and the last while loop in
         * this file to make sure we are pointing to a valid token and/or right
         * after a ';'.
         */
        if (*current_token == NULLPTR || **current_token == ';') {
            current_token += 1;
            continue;
        }

        /* Variable/function definitions. */

        /* Equation parsing. */
        if (C_parse_operation(&current_token))
            return;

        /* Finds the ';'. */
        while (*current_token == NULLPTR || **current_token != ';') {
            current_token += 1;
        }
    }

    #if linux && DEBUG
        printf("Took %f ms to do the initial intermediate pass.\n", \
            (((float)clock() - starting_time) / CLOCKS_PER_SEC) * 1000.0f );
    #endif
}

/*
 * This function reads an operand from the file, adds it to the intermediates,
 * and returns the ending token of the operands so the pointer will have to be
 * incramented once to reach the next token that is not part of the operand.
 * Returns "__INTPTR_MAX__" if nothing was read.
 */
char** C_parse_operand(char** current_token)
{
    /* ASCII numbers */
    if (add_if_ascii_num(*current_token)) {
        pop_operand(false, false);
        return current_token;
    }

    /* Variables */
    if (!is_invalid_name(*current_token)
    && get_variable_symbol(*current_token, 0)) {
        intermediate _tmp_intermediate = \
        { VAR_ACCESS, (void*)get_variable_symbol(*current_token, 0)->hash };

        add_operand(_tmp_intermediate, false);

        pop_operand(false, false);
        return current_token;
    }

    // FUNC CALLS

    return (char**)__INTPTR_MAX__;
}

/*
 * This function takes in the starting index of an operation and sends the
 * contents into "backend/intermediate/intermediate.c" in RPN.
 */
static inline bool C_parse_operation(char*** token)
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

    char** starting_token = *token;

    /* If the operand is operating on the right operand. */
    bool right_associative;

    /* If the operand is operating on the left operand. */
    bool left_associative;

    /* If we have output and operation yet or not. */
    bool have_operated = false;

    /* The operator to be evaluated. */
    operator to_operate = __OPERATOR_MAX__;

    for (char** current_token = starting_token ;; current_token += 1) {
        if (*current_token == NULLPTR)
            continue;
        if (**current_token == '\n')
            break;

        left_associative = false;
        right_associative = false;
        to_operate = __OPERATOR_MAX__;

        /* Adding the operators in between parenthesis. */
        if (**current_token == ')') {
            while (stack_size(&operator_stack)) {
                operator tmp_operator = (operator)stack_pop(&operator_stack);
                if (tmp_operator == OPENING_PAR)
                    goto C_parse_operation_continue_label;
                process_operation(tmp_operator);
            }
            send_error("Uneven parenthesis");
        }

        /* Checking if this is the inital '('. */
        if (**current_token == '(') {
            stack_push(&operator_stack, OPENING_PAR);
            goto C_parse_operation_continue_label;
        }

        /* Reading the first/left operand. */
        char** left_operand_pos = C_parse_operand(current_token);
        if (left_operand_pos == __INTPTR_MAX__) {
            right_associative = true;
        } else {
            current_token = left_operand_pos;
            do {
                current_token += 1;
                if (**current_token == '\n')
                    goto C_parse_operation_pop_operations_label;
                if (**current_token == '(') {
                    stack_push(&operator_stack, OPENING_PAR);
                    goto C_parse_operation_continue_label;
                }
            } while (*current_token == NULLPTR);
        }

        /* Reading the operator. */
        operator current_operator = C_get_operator(*current_token);
        if (current_operator == __OPERATOR_MAX__) {
            printf("C operation parsing is still being tested. The operation be at the top of the file and after the operation nothing else will be processed.\n");
            printf("Expected to find an operator but found: %s.\n", \
                *current_token);
            exit(-1);
        }

        /* Adding the operator to the output or stack. */
        if (stack_size(&operator_stack) != 0
        && C_get_operator_prio(current_operator) >= \
        C_get_operator_prio((operator)stack_top(&operator_stack))) {
            to_operate = current_operator;
            C_set_operator_associativity(&to_operate, &left_associative, \
                &right_associative);
        } else {
            stack_push(&operator_stack, (void*)current_operator);
        }
        have_operated = true;

        if (left_associative)
            goto C_parse_operation_no_right_operand_label;

        do {
            current_token += 1;
            if (**current_token == '\n')
                send_error("Expected another operand before new line");
            if (**current_token == '(') {
                stack_push(&operator_stack, OPENING_PAR);
                goto C_parse_operation_no_right_operand_label;
            }
        } while (*current_token == NULLPTR);

        /* Reading the right operand. */
        char** right_operand_pos = C_parse_operand(current_token);
        if (right_operand_pos == __INTPTR_MAX__)
            left_associative = true;
        else
            current_token = right_operand_pos;

        C_parse_operation_no_right_operand_label: ;
        // TODO: This isn't the best error handling in the world.
        /* Checks if an expected operand is missing. */
        if (left_associative && right_associative)
            send_error("Unexpected operation");

        /* Processing the operation. */
        if (to_operate != __OPERATOR_MAX__)
            process_operation(to_operate);

        C_parse_operation_continue_label: ;
    }

    C_parse_operation_pop_operations_label: ;

    while (stack_size(&operator_stack))
        process_operation((operator)stack_pop(&operator_stack));

    return have_operated;

    #undef CLOSE_OPERAND_SLICE
    #undef OPEN_OPERAND_SLICE
    #undef INC_AND_CHECK_IF_TOKEN_VALID
}

/*
 * This sets the the associative flags and changes subtraction to negation where
 * it's needed.
 */
static inline void C_set_operator_associativity(operator *_operator, \
bool* left_associative, bool* right_associative)
{
    switch (*_operator)
    {
    case COMPLEMENT:
        *right_associative = true;
        return;
    case NOT:
        *right_associative = true;
        return;
    case SUB:
        if (*right_associative)
            *_operator = NEG;
        return;
    default:
        return;
    }
}

/*
 * This either returns the operator of the token or __OPERATOR_MAX__ if it is
 * not a valid operator. Negation and subtraction need to be done seperate of
 * this.
 */
operator C_get_operator(char* token)
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
prio C_get_operator_prio(operator _operator)
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
    default:
        return __PRIO_MAX__;
    }
}

#undef CLOSING_PAR
#undef OPENING_PAR
#undef __PRIO_MAX__
#undef __OPERATOR_MAX__