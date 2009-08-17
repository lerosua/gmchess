/*
 * =====================================================================================
 *
 *       Filename:  define.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2009年02月16日 16时08分04秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  lerosua@gmail.com
 *        Company:  cyclone
 *
 * =====================================================================================
 */

#ifndef __DEFINE_HH_


#ifdef __DEBUG_D

#define DLOG(fmt, ...) \
    { \
        char buffer[36] = {0}; \
        time_t t = time(NULL); \
        strftime(buffer, 36, "%F %T ", localtime(&t)); \
        fprintf(stderr, "%s %s|%d| " fmt, \
                buffer, __FILE__, __LINE__, ##__VA_ARGS__); \
    }

#define RLOG(fmt, ...) \
    { \
        char buffer[36] = {0}; \
        time_t t = time(NULL); \
        strftime(buffer, 36, "%F %T ", localtime(&t)); \
        fprintf(stderr, "%s %s|%d| " fmt, \
                buffer, __FILE__, __LINE__, ##__VA_ARGS__); \
    }

#elif __RELEASE_D

#define DLOG(fmt, ...) \
        ;

#define RLOG(fmt, ...) \
    { \
        char buffer[36] = {0}; \
        time_t t = time(NULL); \
        strftime(buffer, 36, "%F %T ", localtime(&t)); \
        fprintf(stdout, "%s " fmt, buffer, ##__VA_ARGS__); \
    }

#else // by default: __RELEASE_D and __DEBUG_D are not present in compilation

#define DLOG(fmt, ...) \
    { \
        char buffer[36] = {0}; \
        time_t t = time(NULL); \
        strftime(buffer, 36, "%F %T ", localtime(&t)); \
        fprintf(stderr, "%s %s|%d| " fmt, \
                buffer, __FILE__, __LINE__, ##__VA_ARGS__); \
    }

#define RLOG(fmt, ...) \
    { \
        char buffer[36] = {0}; \
        time_t t = time(NULL); \
        strftime(buffer, 36, "%F %T ", localtime(&t)); \
        fprintf(stdout, "%s " fmt, buffer, ##__VA_ARGS__); \
    }

#endif


#endif

