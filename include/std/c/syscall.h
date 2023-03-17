#ifndef STD_C_SYSCALL_H
#define STD_C_SYSCALL_H

// TODO: Implement this in asm.
#if linux && (__GNUC__ || __clang__)
    #include<unistd.h>
#endif

#if linux && __x86_64__
    /* Syscalls */
    #define RESTART_SYSCALL 0
    #define SYS_READ 0
    #define SYS_WRITE 1
    #define SYS_OPEN 2
    #define SYS_CLOSE 3
    #define SYS_STAT 4
    #define SYS_FSTAT 5
    #define SYS_BRK 12
    #define SYS_GETPID 39
    #define SYS_EXIT 60
    #define SYS_GETTID 186
    #define SYS_TGKILL 234

    /* File settings flags */
    #define O_RDONLY 0
    #define O_WRONLY 1
    #define O_RDWR 2
    #define O_CREAT 64
    #define O_EXCL 128
    #define O_NOCTTY 256
    #define O_TRUNC 512
    #define O_APPEND 1024
    #define O_NONBLOCK 2048
    #define O_DIRECTORY 65536

    /* Signals */
    #define SIGHUP 1
    #define SIGINT 2
    #define SIGQUIT 3
    #define SIGILL 4
    #define SIGTRAP 5
    #define SIGABRT 6
    #define SIGEMT 7
    #define SIGFPE 8
    #define SIGKILL 9
    #define SIGBUS 10
    #define SIGSEGV 11
    #define SIGSYS 12
    #define SIGPIPE 13
    #define SIGALRM 14
    #define SIGTERM 15
    #define SIGURG 16
    #define SIGSTOP	17
    #define SIGTSTP	18
    #define SIGCONT	19
    #define SIGCHLD	20
    #define SIGTTIN	21
    #define SIGTTOU	22
    #define SIGIO 23
    #define SIGXCPU	24
    #define SIGXFSZ	25
    #define SIGVTALRM 26
    #define SIGPROF 27
    #define SIGWINCH 28
    #define SIGINFO	29
    #define SIGUSR1	30
    #define SIGUSR2	31
#endif

// TODO: Implement windows support for gcc clang ans msvc
// TODO: Implement native sail support

#endif