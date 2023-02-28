/*
 * This optimization pass goes through the intermediates and holds ranges tied
 * to variables which holds their ranges of possible values as a system of
 * inequalities.
 */

#include<intermediate/optimization/inequalitypass.h>
#include<intermediate/intermediate.h>
#if DEBUG && linux
#include<time.h>
#include<cli.h>
#endif

/*
 * This function goes through the intermediates and assigns variables ranges of
 * possible values and optimizes statments based on those ranges.
 */
void optimization_do_inequality_pass()
{
    #if DEBUG && linux
    clock_t starting_time = clock();
    #endif
}