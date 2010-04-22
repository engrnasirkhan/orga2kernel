#ifndef __UNISTD__H__
#define __UNISTD__H__

#include <config.h>
#include <types.h>
CDECLSTART

extern ssize_t write( int fd, const void *buf, size_t num );
extern ssize_t read( int fd, const void *buf, size_t num );
extern int close( int fd );
extern int open( const char *camino, int flags, ... );
extern int creat( const char *camino, mode_t mode );
extern pid_t fork( void );
extern pid_t wait( int *status );
extern pid_t waitpid( pid_t pid, int *status, int options );
extern pid_t getpid();


CDECLEND
#endif // __UNISTD__H__
