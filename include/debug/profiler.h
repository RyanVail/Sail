/*
 * This file handles internall profiling of code passes. Functions in in this
 * file provide a way to easily start and stop profiles.
 */

#ifndef DEBUG_PROFILER_H
#define DEBUG_PROFILER_H

#include<common.h>

#if DEBUG
    /*
     * This starts a profile for the inputed profile name. If the parent input
     * isn't "NULLPTR" this will put the profile under the parent.
     */
    void debug_profiler_start(char* profile_name, char* parent_name);

    /*
     * This stops and prints stats on the inputed profile name. This will also
     * recursivly end any children profiles. "has_parent" should be true if this
     * profile has a parent.
     */
    void debug_profiler_end(char* profile_name, bool has_parent);

    /* This ends all current profiles. */
    void debug_profiler_end_all();

    #define START_PROFILING(profile_name, parent_profile_name) \
        debug_profiler_start(profile_name, parent_profile_name)
    #define END_PROFILING(profile_name, has_parent) \
        debug_profiler_end(profile_name, has_parent)
    #define END_ALL_PROFILING() debug_profiler_end_all()
#else
    #define START_PROFILING(profile_name, parent_profile_name)
    #define END_PROFILING(profile_name, has_parent)
    #define END_ALL_PROFILING()
#endif

#endif