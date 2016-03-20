// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.

#include <scil-util.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>


void* safe_malloc (size_t size, const char* file, unsigned long line)
{
    assert (size != 0);

    void* p = malloc (size);
    if (p == NULL)
    {
        fprintf (stderr, "[%s:%lu] malloc failed (%lu bytes)\n", file, line,
                 size);
        exit (EXIT_FAILURE);
    }
    return p;
}


void* safe_calloc (size_t nmemb, size_t size, const char* file,
                   unsigned long line)
{
    assert (nmemb != 0);
    assert (size != 0);

    void* p = calloc (nmemb, size);
    if (p == NULL)
    {
        fprintf (stderr, "[%s:%lu] calloc failed (%lu bytes)\n", file, line,
                 nmemb * size);
        exit (EXIT_FAILURE);
    }
    return p;
}


void* safe_realloc (void* ptr, size_t size, const char* file,
                    unsigned long line)
{
    assert (ptr != NULL);
    assert (size != 0);

    void* p = realloc (ptr, size);
    if (p == NULL)
    {
        fprintf (stderr, "[%s:%lu] realloc failed (%lu bytes)\n", file, line,
                 size);
        exit (EXIT_FAILURE);
    }
    return p;
}

FILE* safe_fopen(const char* path, const char* args, const char* src, unsigned long line){

    assert(path!=NULL);
    assert(args!=NULL);

    FILE* file = fopen(path, args);
    if(file == 0){
        fprintf (stderr, "[%s:%lu] fopen failed (path: %s, args: %s)\n", src, line, path, args);
        exit(EXIT_FAILURE);
    }

    return file;
}

size_t scil_write_dims_to_buffer(void* dest, const SCIL_dims_t dims){

    assert(dest != NULL);

    size_t header_size = 0;

    *((uint8_t*)dest) = dims.dims;
    dest = (char*)dest + 1;
    header_size++;

    for(uint8_t i = 0; i < dims.dims; ++i){
        *((size_t*)dest) = dims.length[i];
        dest = (char*)dest + 8;
        header_size += 8;
    }

    return header_size;
}

struct timespec diff_timespec (struct timespec start, struct timespec end)
{
    struct timespec diff;
    if (end.tv_nsec < start.tv_nsec)
    {
        diff.tv_sec = end.tv_sec - start.tv_sec - 1;
        diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        diff.tv_sec = end.tv_sec - start.tv_sec;
        diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return diff;
}

struct timespec sum_timespec (struct timespec t1, struct timespec t2)
{
    struct timespec sum;
    sum.tv_nsec = t1.tv_nsec + t2.tv_nsec;
    sum.tv_sec = sum.tv_nsec / 1000000000;
    sum.tv_nsec %= 1000000000;
    sum.tv_sec += t1.tv_sec + t2.tv_sec;
    return sum;
}

struct timespec div_timespec (struct timespec t, int64_t d)
{
    t.tv_nsec = t.tv_nsec / d;
    t.tv_nsec += (t.tv_sec % d) * 1000000000 / d;
    t.tv_sec /= d;
    return t;
}

double timespec_to_double (struct timespec t)
{
    double d = (double)t.tv_nsec;
    d /= 1000000000.0;
    d += (double)t.tv_sec;
    return d;
}

uint64_t timespec_to_uint64 (struct timespec t)
{
    uint64_t u = (uint64_t)t.tv_sec;
    u *= 1000000000;
    u += (uint64_t)t.tv_nsec;
    return u;
}

void print_time (struct timespec time, FILE* file)
{
    fprintf(file, "%lu.%09lu", time.tv_sec, time.tv_nsec);
}

void critical_error(const char * msg){
  printf("Critical error: %s\n", msg);
  exit(1);
}
