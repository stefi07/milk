/**
 * @file    processtools.h
 * @brief   Command line interface
 *
 * Command line interface (CLI) definitions and function prototypes
 *
 * @author  O. Guyon
 * @date    9 Jul 2017
 *
 * @bug No known bugs.
 *
 */

#define _GNU_SOURCE

#ifndef _CACAO_DEPENDENCY_H
#define _CACAO_DEPENDENCY_H

// #include <fftw3.h>
// #include <gsl/gsl_rng.h>  // for random numbers
// #include <semaphore.h>
// #include <signal.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/types.h>
// #include <unistd.h>

#include <time.h>
#include <errno.h>
#include "ImageStreamIO.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int C_ERRNO;			// C errno (from errno.h)

struct timespec info_time_diff(struct timespec start, struct timespec end);
int print_header(const char *str, char c);
void quick_sort2l(double *array, long *array1, long count);
void quick_sort2l_double(double *array, long *array1, long count);
void quick_sort_long(long *array, long count);
int printERROR(const char *file, const char *func, int line, char *errmessage);

long image_ID_from_images(IMAGE* images, const char *name);

long image_get_first_ID_available_from_images(IMAGE* images);

#ifdef __cplusplus
}
#endif

#endif  // _CACAO_DEPENDENCY_H
