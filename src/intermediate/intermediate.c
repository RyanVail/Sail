/*
 * This holds the intermediate enum and other general functions involving
 * intermediates.
 */
// TODO: A lot of these function names are not specific enough like
// "add_operand" which should be called something like
// "add_intermediate_operand"
#include<intermediate/intermediate.h>
#include<frontend/common/parser.h>

// TODO: It might be better to inline this.
/*
 * This takes in an intermediate and adds the inputted intermediate to the
 * inputted pass' intermediate vector if it's not a temp intermediate return
 * type indicated by _RETURN at the end of the name of the intermediate type.
 */
void add_back_intermediate(intermediate_pass* _pass, intermediate \
_intermediate)
{
    /* These intermediates are just placeholders and don't need to be added. */
    if (intermediate_type_is_temp_return(_intermediate.type))
        return;

    vector_append(&_pass->intermediates, &_intermediate);
}

/*
 * This takes either one or two operands off of "operand_stack" based on "dual"
 * then checks if the operand types are valid for an operation. If comparison
 * both values get taken off.
 */
void pop_operand(intermediate_pass* _pass, bool dual, bool comparison)
{
    if (stack_size(&_pass->operand_stack) < (u32)dual+1)
        send_error("Not enough operands to perform operation");

    operand* _first_operand = dual || comparison \
    ? stack_pop(&_pass->operand_stack) : stack_top(&_pass->operand_stack);

    /* Init the first operand if it isn't already.  */
    if (!_first_operand->initted)
        add_back_intermediate(_pass, _first_operand->intermediate);
    _first_operand->initted = true;

    /* If this is not a dual operation make sure the operand isn't a void. */
    if (!dual) {
        // TODO: Why is the void checking only done if !dual???
        if (_first_operand->type.kind == VOID_TYPE) {
            printf("Cannot preform operation on a ");
            print_type(_pass, _first_operand->type);
            printf(" type");
            send_error("");
        }
        return;
    }

    /* Getting the second operand. */
    operand* _second_operand = (comparison) \
    ? (stack_pop(&_pass->operand_stack)) : (stack_top(&_pass->operand_stack));

    /* Init the second operand if it isn't a place holder. */
    if (_second_operand->intermediate.type != VAR_RETURN) {
        if (!_second_operand->initted)
            add_back_intermediate(_pass, _second_operand->intermediate);

        _second_operand->initted = true;
    }

    /* Making sure the types of the two operands can be casted to each other. */
    type_can_implicitly_cast_to(_first_operand->type, _second_operand->type);

    /* Freeing the uneeded intermediates. */
    if (dual)
        free(_first_operand);
    if (comparison) {
        free(_second_operand);
        intermediate _to_add_intermediate = {
            .type = COMPARISON_RETURN,
            .ptr = NULLPTR,
        };

        add_operand(_pass, _to_add_intermediate, true);
    }
}

/*
 * This processes an operation by taking the needed variables off the operand
 * stack.
 */
void process_operation(intermediate_pass* _pass, intermediate_type _operation)
{
    operand* top_operand;
    switch (_operation)
    {
    case CLEAR_STACK:
        clear_operand_stack(_pass);
        break;
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case AND:
    case XOR:
    case OR:
    case LSL:
    case LSR:
    case MOD:
    case EQUAL:
        pop_operand(_pass, true, false);
        break;
    case IS_EQUAL:
    case NOT_EQUAL:
    case GREATER_THAN:
    case GREATER_THAN_EQUAL:
    case LESS_THAN:
    case LESS_THAN_EQUAL:
        pop_operand(_pass, true, true);
        break;
    case INC:
    case DEC:
    case NOT:
    case COMPLEMENT:
    case NEG:
        pop_operand(_pass, false, false);
        break;
    // TODO: More operators need to be added.
    case MEM_DEREF:
        top_operand = stack_top(&_pass->operand_stack);
        #if DEBUG
        if (top_operand->type.ptr_count == 0)
            send_error("Cannot deref. a non ptr");
        #endif
        top_operand->type.ptr_count--;
        pop_operand(_pass, false, false);
        break;
    case MEM_LOCATION:
        top_operand = stack_top(&_pass->operand_stack);
        top_operand->type.ptr_count++;
        pop_operand(_pass, false, false);
        break;
    default:
        #if DEBUG
        send_error("Unexpected operation");
        #endif
        break;
    }

    intermediate _intermediate = {
        .type = _operation,
        .ptr = NULLPTR,
    };

    add_intermediate(_pass, _intermediate);
}

/*
 * This both casts the top operand on the operand stack to the inputted type and
 * adds a type intermediate.
 */
void add_cast_intermediate(intermediate_pass* _pass, type _type)
{
    /* Casting the top operand. */
    cast_top_operand(_pass, _type);

    // TODO: There should be a function to extract and add the types from
    // and to intermediates.

    /* Creating and adding the cast operand. */
    #if PTRS_ARE_64BIT
    intermediate _tmp_intermediate = {
        .type = CAST,
        .ptr = *(void**)&_type,
    };
    #else
    type* _tmp_type = malloc(sizeof(type));
    CHECK_MALLOC(_tmp_type);
    *_tmp_type = _type;
    intermediate _tmp_intermediate = {
        .type = CAST,
        .ptr = (void*)_tmp_type,
    };
    #endif

    add_intermediate(_pass, _tmp_intermediate);
}

/*
 * This casts the operand on top of "operand_stack" to the desired type. This
 * doesn't add a cast inermediate use "add_cast_intermediate" for that.
 */
void cast_top_operand(intermediate_pass* _pass, type _type)
{
    #if DEBUG
    if (STACK_IS_EMPTY(_pass->operand_stack))
        send_error (
            "Attempted to cast the top operand when there are no operands."
        );
    #endif

    /* Casting the top operand. */
    ((operand*)stack_top(&_pass->operand_stack))->type = _type;
    pop_operand(_pass, false, false);
}

/*
 * This takes in an "operand" and sets its "type" to the type of the
 * intermediate it contains.
 */
void set_type_of_operand(intermediate_pass* _pass, operand* _operand)
{
    type _type = {
        .ptr_count = 0,
        .kind = VOID_TYPE,
    };

    switch (_operand->intermediate.type)
    {
    #if !PTRS_ARE_64BIT
    case CONST_PTR:
        _type.kind = get_lowest_type(*((i64*)_operand->intermediate.ptr));
        _operand->type = _type;
        break;
    #endif
    case CONST:
        _type.kind = get_lowest_type((i64)_operand->intermediate.ptr);
        _operand->type = _type;
        break;
    case FLOAT:
        _type.kind = FLOAT_TYPE;
        _operand->type = _type;
        break;
    case DOUBLE:
        _type.kind = DOUBLE_TYPE;
        _operand->type = _type;
        break;
    case VAR_DECLARATION:
    case VAR_ASSIGNMENT:
    case VAR_RETURN:
    case VAR_ACCESS:
    case VAR_MEM:
        _operand->type = ((variable_symbol*)_operand->intermediate.ptr)->type;
        break;
    case FUNC_RETURN:
        _operand->type = get_function_symbol (
            _pass,
            (u32)(size_t)_operand->intermediate.ptr
        )->return_type;
        break;
    case MEM_RETURN:
    case MEM_LOCATION:
        // TODO: I'm pretty sure this doesn't work for 32 bit.
        #if PTRS_ARE_64BIT
        _operand->type = *(type*)&_operand->intermediate.ptr;
        #else
        _operand->type = *(type*)_operand->intermediate.ptr;
        #endif
        break;
    default:
        _type.kind = VOID_TYPE;
        _operand->type = _type;
    }
}

/*
 * This adds an operand onto the inputted intermediate pass' "operand_stack"
 * from the inputted intermediate.
 */
void add_operand(intermediate_pass* _pass, intermediate _intermediate, \
bool initted)
{
    operand* _operand = malloc(sizeof(operand));
    CHECK_MALLOC(_operand);
    _operand->intermediate = _intermediate;
    _operand->initted = initted;
    set_type_of_operand(_pass, _operand);
    stack_push(&_pass->operand_stack, _operand);
}

/* This clears the operand stack from the inputted intermediate pass. */
void clear_operand_stack(intermediate_pass* _pass)
{
    // TODO: This is going to need more logic for extra data attached to types.
    while (!STACK_IS_EMPTY(_pass->operand_stack))
        free(stack_pop(&_pass->operand_stack));
}

/*
 * This frees the intermediates of the inputted intermediate pass. The symbol
 * table should be in a clean scope before this function is called.
 */
void free_intermediates(intermediate_pass* _pass, bool free_variable_symbols, \
bool free_var_vectors, bool free_constants)
{
    while (VECTOR_SIZE(_pass->intermediates) != 0) {
        intermediate* _intermediate = vector_pop(&_pass->intermediates);

        #if !PTRS_ARE_64BIT
        if (free_constants && _intermediate->ptr != NULLPTR
        && (_intermediate->type == CONST_PTR || _intermediate->type == CAST))
            free(_intermediate->ptr);
        #endif
    
        if (free_variable_symbols && _intermediate->type == VAR_DECLARATION)
            free(_intermediate->ptr);

        if (free_var_vectors && (_intermediate->type == REGISTER
        || _intermediate->type == VAR_USE)) {
            free(((vector*)_intermediate->ptr)->contents);
            free(_intermediate->ptr);
        }
        free(_intermediate);
    }
    free(_pass->intermediates.contents);
    _pass->intermediates.contents = NULLPTR;
}

/* This adds the inputted f32 to the inputted pass' operand stack. */
void add_float_intermediate(intermediate_pass* _pass, f32 value)
{
    #if DEBUG && FLOATS_IN_PTRS
    if (sizeof(u32) != sizeof(f32))
        send_error("The size of a u32 must be equal to the size of a float");
    #endif

    #if FLOATS_IN_PTRS
    intermediate _intermediate = {
        .type = FLOAT,
        .ptr = F32_TO_VOIDPTR(value),
    };
    #else
    f32* new_float = malloc(sizeof(f32));
    CHECK_MALLOC(new_float);
    *new_float = value;
    intermediate _intermediate = {
        .type = FLOAT,
        .ptr = new_float,
    };
    #endif

    add_operand(_pass, _intermediate, false);
}

/* This adds the inputted f64 to the inputted pass' operand stack. */
void add_double_intermediate(intermediate_pass* _pass, f64 value)
{
    #if PTRS_ARE_64BIT && FLOATS_IN_PTRS
    intermediate _intermediate = {
        .type = DOUBLE,
        .ptr = F64_TO_VOIDPTR(value)
    };

    #else
    f64* new_double = malloc(sizeof(f64));
    CHECK_MALLOC(new_double);
    *new_double = value;
    intermediate _intermediate = {
        .type = DOUBLE,
        .ptr = new_double
    };

    #endif
    add_operand(_pass, _intermediate, false);
}

/*
 * This puts the const num into the inputted "_intermediate" be that as a
 * "CONST" intermediate or a "CONST_PTR" based on the "const_num" and the ptr
 * size of the computer the compiler is running on. If the intermediate passed
 * into this function has unitied values this function could result in undefined
 * behaviour if the random value assigned to the type of the intermediate is a
 * CONST_PTR so the type of the intermediate needs to be set to NIL.
 */
void set_intermediate_to_const(intermediate* _intermediate, num const_num)
{
    /* if the number cannot fit into a "CONST" intermediate. */
    if (const_num.magnitude > INTPTR_MAX + const_num.negative) {
        if (_intermediate->type != CONST_PTR) {
            _intermediate->ptr = malloc(sizeof(num));
            CHECK_MALLOC(_intermediate->ptr);
        }
        *(num*)_intermediate->ptr = const_num;
        _intermediate->type = CONST_PTR;
    } else {
        if (_intermediate->type == CONST_PTR)
            free(_intermediate->ptr);

        _intermediate->type = CONST;
        _intermediate->ptr = (void*)(size_t)const_num.magnitude;
        if (const_num.negative)
            *(ssize_t*)&_intermediate->ptr = -(ssize_t)_intermediate->ptr;
    }
}

/*
 * This adds the inputted constant number to the operand stack of the inputted
 * intermediate pass. This will convert the constant numebr into a "CONST_PTR"
 * if it can't fit into a pointer otherwise it will be a "CONST".
 */
void add_const_num(intermediate_pass* _pass, num const_num)
{
    intermediate _operand = {
        .type = NIL,
    };

    set_intermediate_to_const(&_operand, const_num);
    add_operand(_pass, _operand, false);
}

/*
 * If the ASCII number at the inputted token is valid it adds it to the
 * intermediates of the inputted intermediate pass. Returns true if a number
 * was added, otherwise false.
 */
bool add_if_ascii_num(intermediate_pass* _pass, char* token,
const char* prefixes, const char* suffixes)
{
    if (is_ascii_number(token, prefixes, suffixes)) {
        // TODO: This has to support prefixes and suffixes in the number parsing
        num _const_num = get_ascii_number(token);
        add_const_num(_pass, _const_num);
        return true;
    }

    return false;
}

/*
 * If the inputted token is a valid float or double it adds it to the
 * intermediates of the inputted intermediate pass. Returns the ending index of
 * the float if something was added, otherwise 0.
 */
u32 add_if_ascii_float(intermediate_pass* _pass, char** starting_token)
{
    is_ascii_float_return float_info = is_ascii_float(starting_token);
    f64 float_value;
    switch (float_info.type)
    {
    case 0:
        return 0;
        break;
    case 1:
        float_value = get_ascii_float(starting_token, float_info.end_ptr);
        add_float_intermediate(_pass, float_value);
        break;
    case 2:
        float_value = get_ascii_float(starting_token, float_info.end_ptr);
        add_double_intermediate(_pass, float_value);
        break;
    }
    return float_info.token_length;
}

/*
 * This intermediates from the inputted intermediate pass into the inputted
 * stream.
 */
#if DEBUG
// TODO: Replace this array with something less temp.
static const char* INTERMEDIATES_TEXT[] = { "Increment", "Decrement", "Not", \
"Complement", "Negate", "Add", "Sub", "Mul", "Div", "And", "Xor", "Or", "LSL", \
"LSR", "Mod", "!=", "==", ">", ">=", "<", "<=", "=", "Var declaration", \
"Var assignment", "Var access", "Var mem", "Mem location", "Mem access", "If", \
"Else", "Loop", "End", "Continue", "Return", "Break", "Func def", "Func call", \
"Goto", "Const", "Const ptr", "Float", "Double", "Get struct variable", \
"Func return", "Mem return", "Comparison return", "Var return", "Cast", \
"Register", "Ignore", "Var use", "Clear stack", "NIL" };

/*
 * This prints the intermediates in the inputted intermedates pass in a non
 * human readable format for debugging intermediates.
 */
void print_raw_intermediates(intermediate_pass* _pass)
{
    for (u32 i=0; i < VECTOR_SIZE(_pass->intermediates); i++) {
        intermediate* _intermediate = &VECTOR_AT (
            &_pass->intermediates,
            i,
            intermediate
        );

        printf("INTER: %s\n", INTERMEDIATES_TEXT[_intermediate->type]);

        vector* _tmp_vec;

        switch(_intermediate->type)
        {
        case REGISTER:
        case VAR_USE:
            _tmp_vec = _intermediate->ptr;
            if (_tmp_vec == NULLPTR)
                continue;
            for (u32 y=0; y < VECTOR_SIZE(*_tmp_vec); y++)
                printf("%08x\n", VECTOR_AT(_intermediate->ptr, y, u32));
            break;
        case FLOAT:
            #if FLOATS_IN_PTRS
            printf("%f\n", VOIDPTR_TO_F32(_intermediate->ptr));
            #else
            printf("%f\n", *((f64*)_intermediate->ptr));
            #endif
            break;
        case CONST_PTR:
            printf("%li\n", *(u64*)_intermediate->ptr);
            break;
        case CONST:
            printf("%li\n", (size_t)_intermediate->ptr);
            break;
        case DOUBLE:
            #if PTRS_ARE_64BIT && FLOATS_IN_PTRS
            printf("%lf\n", VOIDPTR_TO_F64(_intermediate->ptr));
            #else
            printf("%lf\n", *((f64*)_intermediate->ptr));
            #endif
            break;
        }
    }
}

// TODO: Implement that '`' negation operator in the Salmon front end.
const static char* INTERMEDIATE_FORMATED_TEXT[] = {
    [INC] = "++",
    [DEC] = "--",
    [NOT] = "!",
    [COMPLEMENT] = "~",
    [ADD] = "+",
    [SUB] = "-",
    [MUL] = "*",
    [DIV] = "/",
    [AND] = "&",
    [XOR] = "^",
    [OR] = "|",
    [LSL] = "<<",
    [LSR] = ">>",
    [MOD] = "%%",
    [NOT_EQUAL] = "!=",
    [IS_EQUAL] = "==",
    [GREATER_THAN] = ">",
    [GREATER_THAN_EQUAL] = ">=",
    [LESS_THAN] = "<",
    [LESS_THAN_EQUAL] = "<=",
    [EQUAL] = "=",
    [VAR_DECLARATION] = "",
    [VAR_ASSIGNMENT] = "",
    [VAR_ACCESS] = "",
    [VAR_MEM] = "#",
    [MEM_LOCATION] = "#",
    [MEM_DEREF] = "@",
    [IF] = "if {",
    [ELSE] = "else {",
    [LOOP] = "loop {",
    [END] = "}",
    [CONTINUE] = "continue",
    [RETURN] = "return",
    [BREAK] = "break",
    [FUNC_DEF] = "fn",
    [FUNC_CALL] = "",
    [GOTO] = "goto",
    [CONST] = "",
    [CONST_PTR] = "",
    [FLOAT] = "",
    [DOUBLE] = "",
    [GET_STRUCT_VARIABLE] = ".",
    [FUNC_RETURN] = "",
    [MEM_RETURN] = "",
    [COMPARISON_RETURN] = "",
    [VAR_RETURN] = "",
    [CAST] = "as ",
    [REGISTER] = "",
    [IGNORE] = "",
    [VAR_USE] = "",
    [CLEAR_STACK] = ";",
    [NIL] = "NIL",
};

// TODO: This needs to be finished.
/*
 * This prints the intermediates from the inputted intermediate pass in a human
 * readable format.
 */
void print_intermediates(intermediate_pass* _pass)
{
    /* The current # of tabs being printed. */
    u32 indentation_count = 0;

    for (u32 i=0; i < VECTOR_SIZE(_pass->intermediates); i++) {
        intermediate* _intermediate = vector_at(&_pass->intermediates, i, 0);

        /* Printing the inital intermediate text. */
        printf("%s", INTERMEDIATE_FORMATED_TEXT[_intermediate->type]);

        /* Changing the tab size. */
        switch (_intermediate->type)
        {
        case IF:
        case ELSE:
        case LOOP:
        case FUNC_DEF:
            indentation_count++;
            break;
        case END:
            indentation_count--;
        }

        /* Printing the conditional intermediate data. */
        switch (_intermediate->type)
        {
        case CONST:
            printf("%li\n", (size_t)_intermediate->ptr);
            break;
        case CONST_PTR:
            printf("%li\n", *(u64*)_intermediate->ptr);
            break;
        case FLOAT:
            #if FLOATS_IN_PTRS
            printf("%f\n", VOIDPTR_TO_F32(_intermediate->ptr));
            #else
            printf("%f\n", *((f64*)_intermediate->ptr));
            #endif
            break;
        case DOUBLE:
            #if PTRS_ARE_64BIT && FLOATS_IN_PTRS
            printf("%lf\n", VOIDPTR_TO_F64(_intermediate->ptr));
            #else
            printf("%lf\n", *((f64*)_intermediate->ptr));
            #endif
            break;
        case VAR_ACCESS:
        case VAR_ASSIGNMENT:
            // TODO: Print name
            // TODO: Print name
            break;
        case VAR_DECLARATION:
            // TODO: Print name
        case CAST:
            #if PTRS_ARE_64BIT
            printf("\n\n== %p ==\n\n", _intermediate->ptr);
            // print_type(_pass, *(type*)&_intermediate->ptr);
            #else
            print_type(_pass, *(type*)_intermediate->ptr);
            #endif
            break;
        default:
            continue;
        }

        /* Printing the new line, tabs, and space.. */
        u32 i = 0;
        switch (_intermediate->type)
        {
        case IF:
        case ELSE:
        case LOOP:
        case END:
        case CONTINUE:
        case CLEAR_STACK:
        case FUNC_DEF:
            printf("\n");
            for (i=0; i < indentation_count; i++)
                printf("\t");
            break;
        default:
            printf(" ");
        }
    }
    printf("\n");
}

#endif
