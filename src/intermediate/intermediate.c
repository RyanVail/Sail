/*
 * This holds the intermediate enum and other general functions involving
 * intermediates.
 */
// TODO: A lot of these function names are not specific enough like
// "add_operand" which should be called something like
// "add_intermediate_operand"
#include<intermediate/intermediate.h>
#include<frontend/common/parser.h>

static stack operand_stack = { NULLPTR };
static vector intermediates_vector = { NULLPTR, 0, 0, sizeof(intermediate) };

/*
 * This takes in an operand and adds the intermediates to
 * "intermediate_vector".
 */
void add_operand_to_intermediates(intermediate _intermediate)
{
    /* These intermediates are just placeholders and don't need to be added. */
    if (_intermediate.type == FUNC_RETURN || _intermediate.type == MEM_RETURN)
        return;

    vector_append(&intermediates_vector, &_intermediate);
}

/*
 * This takes either one or two operands off of "operand_stack" based on "dual"
 * then checks if the operand types are valid for an operation. If comparison
 * both values get taken off.
 */
void pop_operand(bool dual, bool comparison)
{
    if (stack_size(&operand_stack) < (u32)dual+1)
        send_error("Not enough operands to perform operation");

    operand* _first_operand = dual || comparison ? stack_pop(&operand_stack) : \
    stack_top(&operand_stack);

    if (!_first_operand->initted)
        add_operand_to_intermediates(_first_operand->intermediate);

    _first_operand->initted = true;

    if (!dual) {
        if (_first_operand->type.kind == VOID_TYPE) {
            printf("Cannot preform operation on a ");
            print_type(_first_operand->type, true);
            printf(" type");
            send_error("");
        }
        return;
    }

    operand* _second_operand = \
    (comparison) ? (stack_pop(&operand_stack)) : (stack_top(&operand_stack));

    if (_second_operand->intermediate.type == VAR_RETURN)
        goto pop_operand_second_operand_is_place_holder_label;

    if (!_second_operand->initted)
        add_operand_to_intermediates(_second_operand->intermediate);

    _second_operand->initted = true;

    pop_operand_second_operand_is_place_holder_label:

    type_can_implicitly_cast_to(_first_operand->type, _second_operand->type);

    if (dual)
        free(_first_operand);
    if (comparison) {
        free(_second_operand);
        intermediate _to_add_intermediate = { COMPARISON_RETURN, NULLPTR };
        add_operand(_to_add_intermediate, true);
    }
}

/*
 * This processes an operation by taking the needed variables off the operand
 * stack.
 */
void process_operation(intermediate_type _operation)
{
    operand* top_operand;
    switch (_operation)
    {
    case EQUAL:
        pop_operand(true, false);
        break;
    case CLEAR_STACK:
        clear_operand_stack();
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
        pop_operand(true, false);
        break;
    case IS_EQUAL:
    case NOT_EQUAL:
    case GREATER_THAN:
    case GREATER_THAN_EQUAL:
    case LESS_THAN:
    case LESS_THAN_EQUAL:
        pop_operand(true, true);
        break;
    case INC:
    case DEC:
    case NOT:
    case COMPLEMENT:
    case NEG:
        pop_operand(false, false);
        break;
    // TODO: More operators need to be added.
    case MEM_DEREF:
        top_operand = stack_top(&operand_stack);
        if (IS_TYPE_STRUCT(top_operand->type))
            top_operand->type.kind += (1 << 16);
        else
            top_operand->type.ptr++;
        pop_operand(false, false);
        break;
    case MEM_LOCATION:
        top_operand = stack_top(&operand_stack);
        if (IS_TYPE_STRUCT(top_operand->type))
            top_operand->type.kind -= (1 << 16);
        else
            top_operand->type.ptr--;
        pop_operand(false, false);
        break;
    default:
        #if DEBUG
        send_error("Unexpected operation");
        #endif
        break;
    }

    intermediate _intermediate = { _operation, NULLPTR };
    add_intermediate(_intermediate);
}

/*
 * This adds an intermediate to the "intermediates_vector".
 */
void add_intermediate(intermediate _intermediate)
{
    vector_append(&intermediates_vector, &_intermediate);
}

/*
 * This casts the operand on top of "operand_stack" to the desired type.
 */
void cast_top_operand(type _type)
{
    #if DEBUG
    if (STACK_IS_EMPTY(operand_stack))
        send_error(\
            "Attempted to cast the top operand when there are no operands.");
    #endif
    ((operand*)stack_top(&operand_stack))->type = _type;
    pop_operand(false, false);
}

/*
 * This takes in an "operand" and sets its "type" to the type of the
 * intermediate it contains. This should only be called on an operand once.
 */
void set_type_of_operand(operand* _operand)
{
    #if DEBUG
    if (_operand->intermediate.type >= INC
    && _operand->intermediate.type <= LESS_THAN_EQUAL)
        send_error("Cannot get the type of invalid intermediate");
    #endif

    type _type = { 0, VOID_TYPE };
    switch (_operand->intermediate.type)
    {
    case CONST_PTR:
        _type.kind = get_lowest_type(*((i64*)_operand->intermediate.ptr));
        _operand->type = _type;
        break;
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
        _operand->type = ((variable_symbol*)_operand->intermediate.ptr)->type;
        break;
    case VAR_ASSIGNMENT:
    case VAR_RETURN:
    case VAR_ACCESS:
    case VAR_MEM:
        _operand->type = get_variable_symbol("", \
            *(u32*)(&_operand->intermediate.ptr))->type;
        break;
    case FUNC_RETURN:
        _operand->type = get_function_symbol("", \
            *(u32*)(&_operand->intermediate.ptr))->return_type;
        break;
    case MEM_RETURN:
    case MEM_LOCATION:
        _operand->type = *((type*)_operand->intermediate.ptr);
        break;
    default:
        _type.kind = VOID_TYPE;
        _operand->type = _type;
    }
}

/*
 * This adds an operand onto the "operand_stack" from an intermediate.
 */
void add_operand(intermediate _intermediate, bool initted)
{
    operand* _operand = malloc(sizeof(operand));
    CHECK_MALLOC(_operand);
    _operand->intermediate = _intermediate;
    _operand->initted = initted;
    set_type_of_operand(_operand);
    stack_push(&operand_stack, _operand);
}

/*
 * This clears the operand stack.
 */
void clear_operand_stack()
{
    while (!(STACK_IS_EMPTY(operand_stack)))
        free((char*)stack_pop(&operand_stack));
}

/*
 * This frees "intermediates_vector". The symbol table should be in a clean
 * scope before this function is called.
 */
void free_intermediates(bool free_variable_symbols, bool free_var_vectors, \
bool free_constants)
{
    while (intermediates_vector.apparent_size != 0) {
        intermediate* _intermediate = vector_pop(&intermediates_vector);

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
    free(intermediates_vector.contents);
    intermediates_vector.contents = NULLPTR;
}

/*
 * This returns a pointer to the intermediate vector.
 */
vector* get_intermediate_vector()
{
    return &intermediates_vector;
}

/* This adds the inputted f32 onto the operand stack. */
void add_float(f32 value)
{
    #if DEBUG && FLOATS_IN_PTRS
    if (sizeof(u32) != sizeof(f32))
        send_error("The size of a u32 must be equal to the size of a float");
    #endif

    #if FLOATS_IN_PTRS
    intermediate _intermediate = { FLOAT, F32_TO_VOIDPTR(value) };
    #else
    f32* new_float = malloc(sizeof(f32));
    CHECK_MALLOC(new_float);
    *new_float = value;
    intermediate _intermediate = { FLOAT, new_float };
    #endif
    add_operand(_intermediate, false);
}

/* This adds the inputted f64 onto the operand stack. */
void add_double(f64 value)
{
    #if PTRS_ARE_64BIT && FLOATS_IN_PTRS
    intermediate _intermediate = { DOUBLE, F64_TO_VOIDPTR(value) };
    #else
    f64* new_double = malloc(sizeof(f64));
    CHECK_MALLOC(new_double);
    *new_double = value;
    intermediate _intermediate = { DOUBLE, new_double };
    #endif
    add_operand(_intermediate, false);
}

/*
 * This puts the const num into the inputted "_intermediate" be that as a
 * "CONST" intermediate or a "CONST_PTR" based on the "const_num".
 */
void set_intermediate_to_const(intermediate* _intermediate, i64 const_num)
{
    #if !PTRS_ARE_64BIT
    if (const_num < ~__UINTPTR_MAX__ || const_num > __UINTPTR_MAX__) {
        if (_intermediate->type != CONST_PTR) {
            _intermediate->ptr = malloc(sizeof(i64));
            CHECK_MALLOC(_intermediate->ptr);
        }
        *(i64*)_intermediate->ptr = const_num;
        _intermediate->type = CONST_PTR;
    } else {
        if (_intermediate->type == CONST_PTR)
            free(_intermediate->ptr);
        _intermediate->ptr = (void*)const_num;
        _intermediate->type = CONST;
    }
    #else
    _intermediate->ptr = (void*)const_num;
    _intermediate->type = CONST;
    #endif
}

/*
 * This adds the inputted constant number to the operand stack. This will convert
 * it into a "CONST_PTR" if it can't fit into a pointer but it's a "CONST" by
 * default.
 */
void add_const_num(i64 const_num)
{
    intermediate _operand = { NIL, NULLPTR };
    set_intermediate_to_const(&_operand, const_num);
    add_operand(_operand, false);
}

/*
 * If the ASCII number at the inputted token is valid it adds it to the
 * intermediates. Returns true if a number was added, otherwise false.
 */
bool add_if_ascii_num(char* token)
{
    if (is_ascii_number(token)) {
        i64 _const_num = get_ascii_number(token);
        add_const_num(_const_num);
        return true;
    }
    return false;
}

/*
 * If the inputted token is a valid float or double it adds it to the
 * intermediates. Returns the ending index of the float if something was added,
 * otherwise 0.
 */
u32 add_if_ascii_float(char** starting_token)
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
        add_float(float_value);
        break;
    case 2:
        float_value = get_ascii_float(starting_token, float_info.end_ptr);
        add_double(float_value);
        break;
    }
    return float_info.token_length;
}

/* This returns a pointer to the operand stack. */
stack* get_operand_stack()
{
    return &operand_stack;
}

/*
 * This prints the intermediates.
 */
#if DEBUG

// TODO: Replace this array with something less temp.
const char* INTERMEDIATES_TEXT[] = { "Increment", "Decrement", "Not", \
"Complement", "Negate", "Add", "Sub", "Mul", "Div", "And", "Xor", "Or", "LSL", \
"LSR", "Mod", "!=", "==", ">", ">=", "<", "<=", "=", "Var declaration", \
"Var assignment", "Var access", "Var mem", "Mem location", "Mem access", "If", \
"Else", "Loop", "End", "Continue", "Return", "Break", "Func def", "Func call", \
"Goto", "Const", "Const ptr", "Float", "Double", "Get struct variable", \
"Func return", "Mem return", "Comparison return", "Var return", "Cast", \
"Register", "Ignore", "Var use", "Clear stack", "NIL" };

void print_intermediates()
{
    for (u32 i=0; i < VECTOR_SIZE(intermediates_vector); i++) {
        intermediate* _intermediate = vector_at(&intermediates_vector, i, 0);

        printf("INTER: %s\n", INTERMEDIATES_TEXT[_intermediate->type]);

        vector* _tmp_vec;

        switch(_intermediate->type)
        {
        case REGISTER:
        case VAR_USE:
            _tmp_vec = _intermediate->ptr;
            if (_tmp_vec == NULLPTR)
                continue;
            for (u32 y=0; y < VECTOR_SIZE((*_tmp_vec)); y++)
                printf("%08x\n",*(u32*)vector_at(_intermediate->ptr,y,0));
            break;
        case FLOAT:
            #if FLOATS_IN_PTRS
            printf("%f\n", VOIDPTR_TO_F32(_intermediate->ptr));
            #else
            printf("%f\n", *((f64*)_intermediate->ptr));
            #endif
            break;
        case CONST:
            printf("%li\n", _intermediate->ptr);
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

#endif
