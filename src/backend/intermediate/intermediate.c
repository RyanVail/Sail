/*
 * This holds the intermediate enum and other general functions involving
 * intermediates. Intermediates should be feed in to "add_intermediate" in
 * reverse polish notation.
 */
#include<backend/intermediate/intermediate.h>
#include<frontend/common/parser.h>

static stack operand_stack = { 0, sizeof(operand) };
static vector intermediates_vector = { 0, 0, 0, sizeof(intermediate) };

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

    operand* _first_operand = \
    dual || comparison ? stack_pop(&operand_stack) : stack_top(&operand_stack);

    if (!_first_operand->inited)
        add_operand_to_intermediates(_first_operand->intermediate);

    _first_operand->inited = true;

    if (!dual) {
        if (_first_operand->type.kind == VOID_TYPE) {
            printf("Cannot preform operation on a \'");
            print_type(_first_operand->type);
            printf("\' type");
            send_error("");
        }
        return;
    }

    operand* _second_operand = \
    comparison ? stack_pop(&operand_stack) : stack_top(&operand_stack);

    if (_second_operand->intermediate.type == VAR_RETURN)
        goto pop_operand_second_operand_is_place_holder_label;

    if (!_second_operand->inited)
        add_operand_to_intermediates(_second_operand->intermediate);

    _second_operand->inited = true;

    pop_operand_second_operand_is_place_holder_label:

    type_can_implicitly_cast_to(_first_operand->type, _second_operand->type, 1);

    if (dual)
        free(_first_operand);
    if (comparison) {
        free(_second_operand);
        intermediate _to_add_intermediate = { COMPARISON_RETURN, 0 };
        add_operand(_to_add_intermediate, true);
    }
}

/*
 * This processes an operation by taking the needed variables off the operand
 * stack.
 */
void process_operation(intermediate_type _operation)
{
    #if DEBUG
    if (_operation < INC || _operation > LESS_THAN_EQUAL
    && _operation != EQUAL && _operation != CLEAR_STACK) {
        printf("Got unexpected operation operand: %u\n", _operation);
        exit(-1);
    }
    #endif

    if (_operation == EQUAL)
        pop_operand(true, false);
    if (_operation == CLEAR_STACK)
        clear_operand_stack();
    if (_operation >= ADD && _operation <= MOD)
        pop_operand(true, false);
    if (_operation >= IS_EQUAL && _operation <= LESS_THAN_EQUAL)
        pop_operand(true, true);
    if (_operation >= INC && _operation <= NEG)
        pop_operand(false, false);

    intermediate _intermediate = { _operation, 0 };
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
    && _operand->intermediate.type <= LESS_THAN_EQUAL) {
        send_error("Cannot get the type of invalid intermediate");
    }
    #endif

    type _type = { 0, VOID_TYPE };
    switch (_operand->intermediate.type) {
        case CONST_PTR:
            _type.kind = get_lowest_type(*((i64*)_operand->intermediate.ptr));
            _operand->type = _type;
            break;
        case CONST:
            _type.kind = get_lowest_type((i64)_operand->intermediate.ptr);
            _operand->type = _type;
            break;
        case VAR_DECLERATION:
            _operand->type = \
                ((variable_symbol*)_operand->intermediate.ptr)->type;
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
 * This adds an operand onto the "operand_stack".
 */
void add_operand(intermediate _intermediate, bool inited)
{
    operand* _operand = malloc(sizeof(operand));
    if (_operand == NULLPTR)
        handle_error(0);
    _operand->intermediate = _intermediate;
    _operand->inited = inited;
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
void free_intermediates(bool free_variable_symbols, bool free_var_vectors)
{
    while (intermediates_vector.apparent_size != 0) {
        register intermediate* _intermediate = vector_pop(&intermediates_vector);

        #if !VOID_PTR_64BIT
        if (_intermediate->type == CONST_PTR || _intermediate->type == CAST)
            free(_intermediate->ptr);
        #endif
    
        if (free_variable_symbols && _intermediate->type == VAR_DECLERATION)
            free(_intermediate->ptr);

        if (free_var_vectors && _intermediate->type == REGISTER
        || _intermediate->type == VAR_USE) {
            free(((vector*)_intermediate->ptr)->contents);
            free(_intermediate->ptr);
        }
        free(_intermediate);
    }
    free(intermediates_vector.contents);
}

/*
 * This returns a pointer to the intermediate vector.
 */
vector* get_intermediate_vector()
{
    return &intermediates_vector;
}

/*
 * If the ASCII number at the inputed token is valid it adds it to the
 * intermediates. Return true if a number was added, otherwise false.
 */
bool add_if_ascii_num(char* token)
{
    if (is_ascii_number(token)) {
        i64 _const_num = get_ascii_number(token);
        /*
            * If we have an i64 larger than the size of a pointer we store the
            * value on the heap and make the ptr point to the value.
            */
        #if !VOID_PTR_64BIT
        intermediate _operand;
        if (_const_num < ~((i64)1 << ((sizeof(void*) << 3))-1) + 1
        || _const_num > ((i64)1 << ((sizeof(void*) << 3)-1))-1) {
            const_num = malloc(sizeof(i64));
            if (const_num == NULLPTR)
                handle_error(0);
            memcpy(const_num, &_const_num, sizeof(i64));
            intermediate _operand = { CONST_PTR, const_num };
        } else {
            const_num = (void*)_const_num;
            intermediate _operand = { CONST, const_num };
        }
        #else
        intermediate _operand = { CONST, (void*)_const_num };
        #endif
        add_operand(_operand, false);
        return true;
    }
    return false;
}

/*
 * This prints the intermediates.
 */
#if DEBUG

// TODO: Replace the bellow array with something less temperary.
const char* INTERMEDIATES_TEXT[] = { "Incrament", "Decrament", "Not", \
"Complement", "Negate", "Add", "Sub", "Mul", "Div", "And", "Xor", "Or", "LSL", \
"LSR", "Mod", "==", "!=", ">", ">=", "<", "<=", "=", "Var decleration", \
"Var assignment", "Var access", "Var mem", "Mem location", "Mem access", "If", \
"Else", "Loop", "End", "Continue", "Return", "Break", "Func def", "Func call", \
"Goto", "Const", "Const ptr", "Func return", "Mem return", "Comparison return",\
"Var return", "Cast", "Register", "Ignore", "Var use", "Clear stack"};

void print_intermediates()
{
    for (u32 i=0; i < VECTOR_SIZE(intermediates_vector); i++) {
        intermediate* _intermediate = vector_at(&intermediates_vector, i, 0);

        printf("INTER: %s\n", INTERMEDIATES_TEXT[_intermediate->type]);

        if (_intermediate->type == REGISTER || _intermediate->type == VAR_USE)
        {
            vector* _tmp_vec = _intermediate->ptr;
            for (u32 y=0; y < VECTOR_SIZE((*_tmp_vec)); y++)
                printf("%08x\n",*(u32*)vector_at(_intermediate->ptr,y,0));
        }
    }
}

#endif