#include<frontend/c/parser.h>
#include<datastructures/stack.h>
#include<frontend/common/parser.h>
#include<frontend/c/preprocessor.h>
#include<intermediate/intermediate.h>
#include<intermediate/struct.h>

/* This represents the priority of an operator. Higher value, higher prio. */
typedef u8 prio;

/* The logically highest value of the "prio" type. */
#define PRIO_MAX ((prio)-1)

// TODO: This has to be thread safe.
/* Stack of operators. Operators are in the pointers of the links. */
static stack operator_stack = {
    .top = NULLPTR,
};

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

    char** current_token = &VECTOR_AT(&file, 0, char*);
    char** end_token = &VECTOR_END(&file, char*);

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

// TODO: Why does these functions use INTPTR_MAX instead of NULLPTR???
/*
 * This function reads an operand from the file, adds it to the intermediates,
 * and returns the ending token of the operands so the pointer will have to be
 * incremented once to reach the next token that is not part of the operand.
 * Returns INTPTR_MAX if nothing was read.
 */
char** C_parse_operand(char** current_token)
{
    /* ASCII numbers */
    if (add_if_ascii_num(*current_token)) {
        pop_operand(false, false);
        return current_token;
    }

    /* Variables */
    variable_symbol* var = get_variable_symbol(*current_token, 0);
    if (!is_invalid_name(*current_token) && var != NULLPTR) {
        intermediate _tmp_intermediate = {
            .type = VAR_ACCESS,
            .ptr = get_variable_symbol(*current_token, var),
        };

        add_operand(_tmp_intermediate, false);
        pop_operand(false, false);

        return current_token;
    }

    // TODO: FUNC CALLS

    return (char**)INTPTR_MAX;
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
        ending_token = (char**)INTPTR_MAX;

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
    operator to_operate = OPERATOR_MAX;

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
        to_operate = OPERATOR_MAX;

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
        if (left_operand_pos == (void*)INTPTR_MAX) {
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
        if (current_operator == OPERATOR_MAX) {
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
        if ((void*)right_operand_pos == (void*)INTPTR_MAX)
            left_associative = true;
        else
            current_token = right_operand_pos;

        C_parse_operation_no_right_operand_label: ;
        // TODO: This isn't the best error handling in the world.
        /* Checks if an expected operand is missing. */
        if (left_associative && right_associative)
            send_error("Unexpected operation");

        /* Processing the operation. */
        if (to_operate != OPERATOR_MAX)
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
 * This either returns the operator of the token or OPERATOR_MAX if it is not a
 * valid operator. Negation and subtraction need to be done separate of this.
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

    return OPERATOR_MAX;
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
        return PRIO_MAX;
    }
}

// TODO: This code comes from "parser.c" and it parses type modifiers which are
// not by default in intermediate passes anymore so this file has to reimplement
// this and type reading on its own.
/*
 * This parses and returns the given type modifiers. This will increment token
 * till it reaches the end of the modifiers. unsigned and signed modifiers will
 * change the first bit of the returning kind. This will skip tokens that are
 * equal to NULLPTR and returns when it doesn't hit a modifier.
 */
// type_kind get_type_modifier(char*** token)
// {
//     char** modifier_names = get_type_modifier_names();
//     type_kind _type;
//     while (true) {
//         if (**token == NULLPTR)
//             continue;
//         for (u32 i=0; modifier_names[i][0] != '\0'; i++) {
//             if (!strcmp(modifier_names[i],**token)) {
//                 switch (i)
//                 {
//                 case 0: // Unsigned
//                     _type |= 1;
//                     goto get_type_modifier_next_token_label;
//                 case 1: // Signed
//                     _type &= 0b1111111111111111;
//                     goto get_type_modifier_next_token_label;
//                 default: // Other flags
//                     _type &= 1 << i + 4;
//                     goto get_type_modifier_next_token_label;
//                 }
//             }
//             return _type;
//         }
//         get_type_modifier_next_token_label:
//         **token += 1;
//     }
// }

#undef PRIO_MAX
