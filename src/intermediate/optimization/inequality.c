/*
 * This file handles the internal repersentation of inequalities and
 * operationing on those inequalites.
 */

#include<intermediate/optimization/inequality.h>

/*
 * This returns true if the two inputted inequalities have a valid solution,
 * otherwise false.
 */
bool inequality_is_in_inequality(inequality* inequality0, \
inequality* inequality1)
{
    i64* operand0;
    i64* operand1;

    /* Getting the values of the inequalities constants. */
    if (inequality0->operand.type == CONST)
        operand0 = (i64*)(&inequality0->operand.ptr);
    if (inequality1->operand.type == CONST)
        operand1 = (i64*)(&inequality1->operand.ptr);
    #if !PTRS_ARE_64BIT
    if (inequality0->operand.type == CONST_PTR)
        operand0 = inequality0->operand.ptr;
    if (inequality1->operand.type == CONST_PTR)
        operand1 = inequality1->operand.ptr;
    #endif

    // TODO: I can't think of anything other than "have equals" but that isn't
    // even english

    /* If the two constants are equal and the two operators have 'equals'. */
    if (*operand0 == *operand1 && IS_EQUAL_TO_OPERATOR(inequality0->operator)
    && IS_EQUAL_TO_OPERATOR(inequality1->operator))
        return true;

    /* If the operators are "!=" and "==" and the operands are the same. */
    if (*operand0 == *operand1 && (inequality0->operator ^
    inequality1->operator == 31))
        return false;

    /*
     * If an operator has an equal to sign make both of them equal to and
     * subsitute the constant operands into each equation.
     */
    if (IS_EQUAL_TO_OPERATOR(inequality0->operator)
    || IS_EQUAL_TO_OPERATOR(inequality1->operator)) {
        return evaluate_expression(*operand0, *operand1, inequality0->operator
        & 0b11111110) && evaluate_expression(*operand1, *operand0, \
        inequality1->operator & 0b11111110);
    }

    /*
     * Subsitute both constant operands into each equation and if both return
     * true the two inequalities share some range of constant integers.
     */
    return evaluate_expression(*operand0, *operand1, inequality0->operator)
    && evaluate_expression(*operand1, *operand0, inequality1->operator);
}

/*
 * This returns true if the inputted inequality is a solution to the inputted
 * system of inequalities, otherwise false.
 */
bool inequality_is_in_system(vector* inequalities, inequality* _inequality)
{
    inequality* current_inequality = inequalities->contents;
    for (u32 i=0; i < VECTOR_SIZE(*inequalities); i++, current_inequality++){
        if (!inequality_is_in_inequality(_inequality, \
        *(inequality**)vector_at(inequalities, i, false)))
            return false; 
    }
}

/*
 * This adds the inputted inequality to the inputted system of inequalities.
 * This is prefered over "vector_append" because this does additional work. The
 * returning bool will be true if there is no solution to the inequality.
 */
bool add_to_inequality(vector* inequalities, inequality* _inequality)
{
    /* IS_EQUAL restricts the range of possibilites to one. */
    if (_inequality->operator == IS_EQUAL) {
        bool is_in_system = inequality_is_in_system(inequalities, _inequality);
        /* Freeing the system. */
        while (VECTOR_SIZE(*inequalities)) {
            #if !VOID_PTR_64BIT
            intermediate* current_intermediate = vector_pop(inequalities);
            if (current_intermediate->type == CONST_PTR)
                free(current_intermediate->ptr);
            #endif
        }
        /* Adding back the "IS_EQUAL" as the only inequality in the system. */
        vector_append(inequalities, _inequality);

        return is_in_system;
    }

    /* Trying to simplify the system of inequality. */
    inequality* current_inequality = inequalities->contents;
    i64 operand1;
    i64 operand0;
    if (_inequality->operand.type == CONST_PTR)
        operand0 = *(i64*)_inequality->operand.ptr;
    else if (_inequality->operand.type == CONST)
        operand0 = (i64)_inequality->operand.ptr;
    for (u32 i=0; i < VECTOR_SIZE(*inequalities); i++, current_inequality++) {
        // TODO: This logic should be in its own function per inequality so
        // testing if an inequality is in a system can add to "inequality0"
        // while going through the system.
        if (current_inequality->operand.type == CONST)
            operand1 = (i64)current_inequality->operand.ptr;
        else if (current_inequality->operand.type == CONST_PTR)
            operand1 = *(i64*)current_inequality->operand.ptr;
        else
            continue;

        // TODO: All of this logic should also to the current inequality instead
        // of duplicating all of the code.
        switch (_inequality->operator)
        {
        /* SHOULD BE RUN ON BOTH SIDES BEGIN */
        case NOT_EQUAL:
            switch (current_inequality->operator)
            {
            case LESS_THAN:
                if (operand0 - 1 == operand1)
                    set_intermediate_to_const(&current_inequality->operand, \
                        operand1);
                break;
            case GREATER_THAN:
                if (operand0 + 1 == operand1)
                    set_intermediate_to_const(&current_inequality->operand, \
                        operand1);
                break;
            case GREATER_THAN_EQUAL:
            case LESS_THAN_EQUAL:
                if (operand0 == operand1)
                    current_inequality->operator |= 0b1;
                break;
            }
            break;
        /* SHOULD BE RUN ON BOTH SIDES END */
        case GREATER_THAN:
        case GREATER_THAN_EQUAL:
            switch (current_inequality->operator)
            {
            case GREATER_THAN:
            case GREATER_THAN_EQUAL:
                /*
                 * If the value of the first operand is larger than that of the
                 * second the "_EQUAL" bit of the second operator is set to
                 * the "_EQUAL" bit of the first operator.
                 */
                if (operand0 > operand1) {
                    current_inequality->operator |= _inequality->operator & \
                    0b11111110;
                }

                set_intermediate_to_const(&current_inequality->operand, \
                    operand0 > operand1 ? operand0 : operand1);
                break;
            }
            break;
        case LESS_THAN:
        case LESS_THAN_EQUAL:
            switch (current_inequality->operator)
            {
            case LESS_THAN:
            case LESS_THAN_EQUAL:
                /*
                 * If the value of the first operand is larger than that of the
                 * second the "_EQUAL" bit of the second operator is set to
                 * the "_EQUAL" bit of the first operator.
                 */
                if (operand0 > operand1) {
                    current_inequality->operator |= _inequality->operator & \
                    0b11111110;
                }

                set_intermediate_to_const(&current_inequality->operand, \
                    operand0 > operand1 ? operand0 : operand1);
                break;
            }
            break;
        }
    }

    return inequality_is_in_system(inequalities, _inequality);
}