/* Shared general utility routines for GDB, the GNU debugger.

   Copyright (C) 1986-2014 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include "config.h"
#include "ansidecl.h"
#include <stddef.h>
#include <stdarg.h>

/* If possible, define FUNCTION_NAME, a macro containing the name of
   the function being defined.  Since this macro may not always be
   defined, all uses must be protected by appropriate macro definition
   checks (Eg: "#ifdef FUNCTION_NAME").

   Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */
#if (GCC_VERSION >= 2004)
#define FUNCTION_NAME		__PRETTY_FUNCTION__
#else
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#define FUNCTION_NAME		__func__  /* ARI: func */
#endif
#endif

extern void malloc_failure (long size) ATTRIBUTE_NORETURN;
extern void internal_error (const char *file, int line, const char *, ...)
     ATTRIBUTE_NORETURN ATTRIBUTE_PRINTF (3, 4);

/* xmalloc(), xrealloc() and xcalloc() have already been declared in
   "libiberty.h". */

/* Like xmalloc, but zero the memory.  */
void *xzalloc (size_t);

void xfree (void *);

/* Like asprintf and vasprintf, but return the string, throw an error
   if no memory.  */
char *xstrprintf (const char *format, ...) ATTRIBUTE_PRINTF (1, 2);
char *xstrvprintf (const char *format, va_list ap)
     ATTRIBUTE_PRINTF (1, 0);

/* Like snprintf, but throw an error if the output buffer is too small.  */
int xsnprintf (char *str, size_t size, const char *format, ...)
     ATTRIBUTE_PRINTF (3, 4);

/* Make a copy of the string at PTR with LEN characters
   (and add a null character at the end in the copy).
   Uses malloc to get the space.  Returns the address of the copy.  */

char *savestring (const char *ptr, size_t len);

#endif
