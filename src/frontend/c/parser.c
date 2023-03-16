#include<frontend/c/parser.h>
#include<datastructures/stack.h>
#include<frontend/common/parser.h>
#include<frontend/c/preprocessor.h>
#include<intermediate/intermediate.h>
#include<intermediate/struct.h>

/* This represents the priority of an operator. Higher value, higher prio. */
typedef u8 prio;

/* The logically highest value of the "prio" type. */
#define __PRIO_MAX__ ((prio)-1)

/* Stack of operators. Operators are in the pointers of the links. */
static stack operator_stack = { NULLPTR };

prio C_get_operator_prio(operator _operator);
operator C_get_operator(char* token);
char** C_parse_operand(char** current_token);
void C_parse_inital_syntax_tree(char*** _token);
static inline void C_set_operator_associativity(operator *_operator, \
bool* left_associative, bool* right_associative);

/*
 * This function reads in the C source file referenced by "file_name" processes
 * and turns it into intermediates.
 */
void C_file_into_intermediates(char* file_name)
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
    START_PROFILING("the initial intermediate pass", "compile file");

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
        C_parse_inital_syntax_tree(&current_token);

        /* Finds the ';'. */
        while (*current_token == NULLPTR && **current_token != ';')
            current_token += 1;
    }

    END_PROFILING("the initial intermediate pass", true);
}

/* This function parses the inital syntax tree of C source code. */
void C_parse_inital_syntax_tree(char*** _token)
{
    /* Equation parsing. */
    if (C_parse_operation(_token, (void (*)(operator))process_operation, \
    C_parse_operand, NULLPTR))
        return;
}

// TODO: Why does these functions use __INTPTR_MAX__ instead of NULLPTR???
/*
 * This function reads an operand from the file, adds it to the intermediates,
 * and returns the ending token of the operands so the pointer will have to be
 * incremented once to reach the next token that is not part of the operand.
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
    if (!is_invalid_name(*current_token) \
    && get_variable_symbol(*current_token, 0)) {
        intermediate _tmp_intermediate = { .type = VAR_ACCESS, \
        .ptr = (void*)(size_t)(get_variable_symbol(*current_token, 0)->hash) };

        add_operand(_tmp_intermediate, false);
        pop_operand(false, false);

        return current_token;
    }

    // TODO: FUNC CALLS

    return (char**)__INTPTR_MAX__;
}

/*
 * This function takes in the starting index of an operation and passes the
 * "operator" type of every operator into the inputted "operator_func".
 * The parsed operators are passed into the inputted "operand_func". After
 * reaching "ending_token" this will stop parsing the equation if "ending_token"
 * is a NULLPTR nothing is done.
 */
bool C_parse_operation(char*** token, void (*operator_func)(operator), \
char** (*operand_func)(char**), char** ending_token)
{
    /*
     * If the ending token isn't set this ensures the current token never gets
     * to a value over the maximum ptr counter.
     */
    if (ending_token == NULLPTR)
        ending_token = (char**)__INTPTR_MAX__;

    /*
    - Parse the equation:
        - If the next operator <= the presendence of the last, add the last
            to the output and the current on to the stack.

        - If an opening parethesis is hit add it to the stack.

        - If the last operator was a parenthesis add the current operator to
            the output.

        - If the current operator is a closing parenthesis or the end of the
            operation add all the operators to the output.
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

    char** current_token;

    for (current_token = starting_token ;; current_token += 1) {
        if (*current_token == NULLPTR)
            continue;
        if (**current_token == '\n')
            break;
        if (current_token >= ending_token)
            break;

        left_associative = false;
        right_associative = false;
        to_operate = __OPERATOR_MAX__;

        /* Adding the operators in between parenthesis. */
        if (**current_token == ')') {
            while (stack_size(&operator_stack)) {
                operator tmp_operator = (size_t)stack_pop(&operator_stack);
                if (tmp_operator == OPENING_PAR)
                    goto C_parse_operation_continue_label;
                (*operator_func)(tmp_operator);
            }
            send_error("Uneven parenthesis");
        }

        // TODO: This checking if the operation is done should be done a lot
        // better some way

        /* Checking if this is the inital '('. */
        if (**current_token == '(') {
            stack_push(&operator_stack, (void*)OPENING_PAR);
            goto C_parse_operation_continue_label;
        }

        /* Checking if this is the inital '\n'. */
        if (**current_token == '\n')
            goto C_parse_operation_pop_operations_label;

        /* Reading the first/left operand. */
        char** left_operand_pos = (*operand_func)(current_token);
        if (left_operand_pos == (void*)__INTPTR_MAX__) {
            right_associative = true;
        } else {
            current_token = left_operand_pos;
            while (true) {
                current_token += 1;
                if (*current_token == NULLPTR)
                    continue;
                if (**current_token == '\n' || current_token >= ending_token)
                    goto C_parse_operation_pop_operations_label;
                if (**current_token == '(') {
                    stack_push(&operator_stack, (void*)OPENING_PAR);
                    goto C_parse_operation_continue_label;
                }
                break;
            }
        }

        /* Reading the operator. */
        operator current_operator = C_get_operator(*current_token);
        if (current_operator == __OPERATOR_MAX__) {
            char _char = **current_token;
            if (_char == '\n' || _char == ';' || _char == '\0' \
            || current_token >= ending_token)
                goto C_parse_operation_pop_operations_label;
            printf("C operation parsing is still being tested. The operation be at the top of the file and after the operation nothing else will be processed.\n");
            printf("Expected to find an operator but found: %s.\n", \
                *current_token);
            exit(-1);
        }

        // TODO: This doesn't handle the case of the operator being either the
        // one after an '(' or just the first operator's priority and it would
        // be added last even though the other operators have a lower priority.

        /* Adding the operator to the output or stack. */
        if (!STACK_IS_EMPTY(operator_stack)
        && C_get_operator_prio(current_operator) >= \
        C_get_operator_prio((operator)(size_t)stack_top(&operator_stack))) {
            to_operate = current_operator;
            C_set_operator_associativity(&to_operate, &left_associative, \
                &right_associative);
        } else {
            stack_push(&operator_stack, (void*)(size_t)current_operator);
        }
        have_operated = true;

        if (left_associative)
            goto C_parse_operation_no_right_operand_label;

        while (true) {
            current_token += 1;
            if (*current_token == NULLPTR)
                continue;
            if (current_token == ending_token)
                goto C_parse_operation_pop_operations_label;
            if (**current_token == '\n')
                send_error("Expected another operand before new line");
            if (**current_token == '(') {
                stack_push(&operator_stack, (void*)OPENING_PAR);
                goto C_parse_operation_no_right_operand_label;
            }
            break;
        }

        /* Reading the right operand. */
        char** right_operand_pos = (*operand_func)(current_token);
        if ((void*)right_operand_pos == (void*)__INTPTR_MAX__)
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
            (*operator_func)(to_operate);

        C_parse_operation_continue_label: ;
    }

    C_parse_operation_pop_operations_label: ;

    while (stack_size(&operator_stack))
        (*operator_func)((operator)(size_t)stack_pop(&operator_stack));

    *token = (current_token + 1);
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
 * not a valid operator. Negation and subtraction need to be done separate of
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

/* This returns the priority of the inputted operator. */
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
    case MEM_DEREF:
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

#undef __PRIO_MAX__