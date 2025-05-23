/* Header file for libgcov-*.c.
   Copyright (C) 1996-2014 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   GCC is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef GCC_LIBGCOV_H
#define GCC_LIBGCOV_H

#ifndef __KERNEL__
/* work around the poisoned malloc/calloc in system.h.  */
#ifndef xmalloc
#define xmalloc malloc
#endif
#ifndef xcalloc
#define xcalloc calloc
#endif
#ifndef xrealloc
#define xrealloc realloc
#endif
#ifndef xfree
#define xfree free
#endif
#else /* __KERNEL__ */
#include "libgcov-kernel.h"
#endif /* __KERNEL__ */

#ifndef IN_GCOV_TOOL
/* About the target.  */
/* This path will be used by libgcov runtime.  */

#ifndef __KERNEL__
#include "tconfig.h"
#include "tsystem.h"
#include "coretypes.h"
#include "tm.h"
#include "libgcc_tm.h"
#endif /* __KERNEL__ */

#undef FUNC_ID_WIDTH
#undef FUNC_ID_MASK

#if BITS_PER_UNIT == 8
typedef unsigned gcov_unsigned_t __attribute__ ((mode (SI)));
typedef unsigned gcov_position_t __attribute__ ((mode (SI)));
#if LONG_LONG_TYPE_SIZE > 32
typedef signed gcov_type __attribute__ ((mode (DI)));
typedef unsigned gcov_type_unsigned __attribute__ ((mode (DI)));
#define FUNC_ID_WIDTH 32
#define FUNC_ID_MASK ((1ll << FUNC_ID_WIDTH) - 1)
#else
typedef signed gcov_type __attribute__ ((mode (SI)));
typedef unsigned gcov_type_unsigned __attribute__ ((mode (SI)));
#define FUNC_ID_WIDTH 16
#define FUNC_ID_MASK ((1 << FUNC_ID_WIDTH) - 1)
#endif
#else  /* BITS_PER_UNIT != 8  */
#if BITS_PER_UNIT == 16
typedef unsigned gcov_unsigned_t __attribute__ ((mode (HI)));
typedef unsigned gcov_position_t __attribute__ ((mode (HI)));
#if LONG_LONG_TYPE_SIZE > 32
typedef signed gcov_type __attribute__ ((mode (SI)));
typedef unsigned gcov_type_unsigned __attribute__ ((mode (SI)));
#define FUNC_ID_WIDTH 32
#define FUNC_ID_MASK ((1ll << FUNC_ID_WIDTH) - 1)
#else
typedef signed gcov_type __attribute__ ((mode (HI)));
typedef unsigned gcov_type_unsigned __attribute__ ((mode (HI)));
#define FUNC_ID_WIDTH 16
#define FUNC_ID_MASK ((1 << FUNC_ID_WIDTH) - 1)
#endif
#else /* BITS_PER_UNIT != 16  */
typedef unsigned gcov_unsigned_t __attribute__ ((mode (QI)));
typedef unsigned gcov_position_t __attribute__ ((mode (QI)));
#if LONG_LONG_TYPE_SIZE > 32
typedef signed gcov_type __attribute__ ((mode (HI)));
typedef unsigned gcov_type_unsigned __attribute__ ((mode (HI)));
#define FUNC_ID_WIDTH 32
#define FUNC_ID_MASK ((1ll << FUNC_ID_WIDTH) - 1)
#else
typedef signed gcov_type __attribute__ ((mode (QI)));
typedef unsigned gcov_type_unsigned __attribute__ ((mode (QI)));
#define FUNC_ID_WIDTH 16
#define FUNC_ID_MASK ((1 << FUNC_ID_WIDTH) - 1)
#endif
#endif  /* BITS_PER_UNIT == 16  */
#endif  /* BITS_PER_UNIT == 8  */

#if LONG_LONG_TYPE_SIZE > 32
#define GCOV_TYPE_ATOMIC_FETCH_ADD_FN __atomic_fetch_add_8
#define GCOV_TYPE_ATOMIC_FETCH_ADD BUILT_IN_ATOMIC_FETCH_ADD_8
#else
#define GCOV_TYPE_ATOMIC_FETCH_ADD_FN __atomic_fetch_add_4
#define GCOV_TYPE_ATOMIC_FETCH_ADD BUILT_IN_ATOMIC_FETCH_ADD_4
#endif

#if defined (TARGET_POSIX_IO)
#define GCOV_LOCKED 1
#else
#define GCOV_LOCKED 0
#endif

#else /* IN_GCOV_TOOL */
/* About the host.  */
/* This path will be compiled for the host and linked into
   gcov-tool binary.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"

typedef unsigned gcov_unsigned_t;
typedef unsigned gcov_position_t;
/* gcov_type is typedef'd elsewhere for the compiler */
#if defined (HOST_HAS_F_SETLKW)
#define GCOV_LOCKED 1
#else
#define GCOV_LOCKED 0
#endif

#define FUNC_ID_WIDTH 32
#define FUNC_ID_MASK ((1ll << FUNC_ID_WIDTH) - 1)

/* Some Macros specific to gcov-tool.  */

#define L_gcov 1
#define L_gcov_merge_add 1
#define L_gcov_merge_single 1
#define L_gcov_merge_delta 1
#define L_gcov_merge_ior 1
#define L_gcov_merge_time_profile 1
#define L_gcov_merge_icall_topn 1
#define L_gcov_merge_dc 1

/* Make certian internal functions/variables in libgcov available for
   gcov-tool access.  */
#define GCOV_TOOL_LINKAGE 

extern gcov_type gcov_read_counter_mem ();
extern unsigned gcov_get_merge_weight ();

#endif /* !IN_GCOV_TOOL */

#undef EXTRACT_MODULE_ID_FROM_GLOBAL_ID
#undef EXTRACT_FUNC_ID_FROM_GLOBAL_ID
#undef GEN_FUNC_GLOBAL_ID
#define EXTRACT_MODULE_ID_FROM_GLOBAL_ID(gid) \
                (gcov_unsigned_t)(((gid) >> FUNC_ID_WIDTH) & FUNC_ID_MASK)
#define EXTRACT_FUNC_ID_FROM_GLOBAL_ID(gid) \
                (gcov_unsigned_t)((gid) & FUNC_ID_MASK)
#define GEN_FUNC_GLOBAL_ID(m,f) ((((gcov_type) (m)) << FUNC_ID_WIDTH) | (f))

#if defined(inhibit_libc)
#define IN_LIBGCOV (-1)
#else
#define IN_LIBGCOV 1
#if defined(L_gcov)
#define GCOV_LINKAGE /* nothing */
#endif
#endif

/* In libgcov we need these functions to be extern, so prefix them with
   __gcov.  In libgcov they must also be hidden so that the instance in
   the executable is not also used in a DSO.  */
#define gcov_var __gcov_var
#define gcov_open __gcov_open
#define gcov_close __gcov_close
#define gcov_write_tag_length __gcov_write_tag_length
#define gcov_position __gcov_position
#define gcov_seek __gcov_seek
#define gcov_rewrite __gcov_rewrite
#define gcov_truncate __gcov_truncate
#define gcov_is_error __gcov_is_error
#define gcov_write_unsigned __gcov_write_unsigned
#define gcov_write_counter __gcov_write_counter
#define gcov_write_summary __gcov_write_summary
#define gcov_write_module_info __gcov_write_module_info
#define gcov_read_unsigned __gcov_read_unsigned
#define gcov_read_counter __gcov_read_counter
#define gcov_read_summary __gcov_read_summary
#define gcov_read_buildinfo __gcov_read_buildinfo
#define gcov_read_module_info __gcov_read_module_info
#define gcov_sort_n_vals __gcov_sort_n_vals

/* Poison these, so they don't accidentally slip in.  */
#pragma GCC poison gcov_write_string gcov_write_tag gcov_write_length
#pragma GCC poison gcov_time gcov_magic

#ifdef HAVE_GAS_HIDDEN
#define ATTRIBUTE_HIDDEN  __attribute__ ((__visibility__ ("hidden")))
#else
#define ATTRIBUTE_HIDDEN
#endif

#include "gcov-io.h"

/* Structures embedded in coveraged program.  The structures generated
   by write_profile must match these.  */
/* Information about counters for a single function.  */
struct gcov_ctr_info
{
  gcov_unsigned_t num;          /* number of counters.  */
  gcov_type *values;            /* their values.  */
};

/* Information about a single function.  This uses the trailing array
   idiom. The number of counters is determined from the merge pointer
   array in gcov_info.  The key is used to detect which of a set of
   comdat functions was selected -- it points to the gcov_info object
   of the object file containing the selected comdat function.  */

struct gcov_fn_info
{
  const struct gcov_info *key;          /* comdat key */
  gcov_unsigned_t ident;                /* unique ident of function */
  gcov_unsigned_t lineno_checksum;      /* function lineo_checksum */
  gcov_unsigned_t cfg_checksum; /* function cfg checksum */
  struct gcov_ctr_info ctrs[1];         /* instrumented counters */
};

/* Type of function used to merge counters.  */
typedef void (*gcov_merge_fn) (gcov_type *, gcov_unsigned_t);

/* Information about a single object file.  */
struct gcov_info
{
  gcov_unsigned_t version;      /* expected version number */
  struct gcov_module_info *mod_info; /* addtional module info.  */
  struct gcov_info *next;       /* link to next, used by libgcov */

  gcov_unsigned_t stamp;        /* uniquifying time stamp */
  const char *filename;         /* output file name */
  gcov_unsigned_t eof_pos;      /* end position of profile data */
  gcov_merge_fn merge[GCOV_COUNTERS];  /* merge functions (null for
                                          unused) */

  unsigned n_functions;         /* number of functions */

#if !defined (IN_GCOV_TOOL) && !defined (__KERNEL__)
  const struct gcov_fn_info *const *functions; /* pointer to pointers
                                                  to function information  */
#elif defined (IN_GCOV_TOOL)
  const struct gcov_fn_info **functions;
#else
  struct gcov_fn_info **functions;
#endif /* !IN_GCOV_TOOL */
  char **build_info;            /* strings to include in BUILD_INFO
                                   section of gcda file.  */
};

/* Information about a single imported module.  */
struct dyn_imp_mod
{
  const struct gcov_info *imp_mod;
  double weight;
};

/* Register a new object file module.  */
extern void __gcov_init (struct gcov_info *) ATTRIBUTE_HIDDEN;

/* Set sampling rate to RATE.  */
extern void __gcov_set_sampling_rate (unsigned int rate);

/* Called before fork, to avoid double counting.  */
extern void __gcov_flush (void) ATTRIBUTE_HIDDEN;

/* Function to reset all counters to 0.  */
extern void __gcov_reset (void);
/* Function to enable early write of profile information so far.
   __gcov_dump is also used by __gcov_dump_all. The latter
   depends on __GCOV_DUMP to have hidden or protected visibility
   so that each library has its own copy of the registered dumper.  */
extern void __gcov_dump (void) ATTRIBUTE_HIDDEN;

/* Call __gcov_dump registered from each shared library.
   This function must have default visibility.  */
void __gcov_dump_all (void);

/* The merge function that just sums the counters.  */
extern void __gcov_merge_add (gcov_type *, unsigned) ATTRIBUTE_HIDDEN;

/* The merge function to choose the most common value.  */
extern void __gcov_merge_single (gcov_type *, unsigned) ATTRIBUTE_HIDDEN;

/* The merge function to choose the most common difference between
   consecutive values.  */
extern void __gcov_merge_delta (gcov_type *, unsigned) ATTRIBUTE_HIDDEN;

/* The merge function that just ors the counters together.  */
extern void __gcov_merge_ior (gcov_type *, unsigned) ATTRIBUTE_HIDDEN;

/* The merge function used for direct call counters.  */
extern void __gcov_merge_dc (gcov_type *, unsigned) ATTRIBUTE_HIDDEN;

/* The merge function used for indirect call counters.  */
extern void __gcov_merge_icall_topn (gcov_type *, unsigned) ATTRIBUTE_HIDDEN;

extern void __gcov_merge_time_profile (gcov_type *, unsigned) ATTRIBUTE_HIDDEN;

/* The profiler functions.  */
extern void __gcov_interval_profiler (gcov_type *, gcov_type, int, unsigned);
extern void __gcov_pow2_profiler (gcov_type *, gcov_type);
extern void __gcov_one_value_profiler (gcov_type *, gcov_type);
extern void __gcov_indirect_call_profiler (gcov_type*, gcov_type,
                                           void*, void*);
extern void __gcov_indirect_call_profiler_v2 (gcov_type, void *);
extern void __gcov_indirect_call_topn_profiler (void *, void *, gcov_unsigned_t) ATTRIBUTE_HIDDEN;
extern void __gcov_direct_call_profiler (void *, void *, gcov_unsigned_t) ATTRIBUTE_HIDDEN;
extern void __gcov_average_profiler (gcov_type *, gcov_type);
extern void __gcov_ior_profiler (gcov_type *, gcov_type);
extern void __gcov_sort_n_vals (gcov_type *value_array, int n);
extern void __gcov_time_profiler (gcov_type *);

#ifndef inhibit_libc
/* The wrappers around some library functions..  */
extern pid_t __gcov_fork (void) ATTRIBUTE_HIDDEN;
extern int __gcov_execl (const char *, char *, ...) ATTRIBUTE_HIDDEN;
extern int __gcov_execlp (const char *, char *, ...) ATTRIBUTE_HIDDEN;
extern int __gcov_execle (const char *, char *, ...) ATTRIBUTE_HIDDEN;
extern int __gcov_execv (const char *, char *const []) ATTRIBUTE_HIDDEN;
extern int __gcov_execvp (const char *, char *const []) ATTRIBUTE_HIDDEN;
extern int __gcov_execve (const char *, char  *const [], char *const [])
  ATTRIBUTE_HIDDEN;


/* Functions that only available in libgcov.  */
GCOV_LINKAGE int gcov_open (const char */*name*/) ATTRIBUTE_HIDDEN;
GCOV_LINKAGE void gcov_write_counter (gcov_type) ATTRIBUTE_HIDDEN;
GCOV_LINKAGE void gcov_write_tag_length (gcov_unsigned_t, gcov_unsigned_t)
    ATTRIBUTE_HIDDEN;
GCOV_LINKAGE void gcov_write_summary (gcov_unsigned_t /*tag*/,
                                      const struct gcov_summary *)
    ATTRIBUTE_HIDDEN;
GCOV_LINKAGE void gcov_seek (gcov_position_t /*position*/) ATTRIBUTE_HIDDEN;
GCOV_LINKAGE void gcov_truncate (void) ATTRIBUTE_HIDDEN;
void gcov_write_module_info (const struct gcov_info *, unsigned)
    ATTRIBUTE_HIDDEN;
GCOV_LINKAGE void gcov_write_module_infos (struct gcov_info *mod_info)
    ATTRIBUTE_HIDDEN;
GCOV_LINKAGE const struct dyn_imp_mod **
gcov_get_sorted_import_module_array (struct gcov_info *mod_info, unsigned *len)
    ATTRIBUTE_HIDDEN;
GCOV_LINKAGE inline void gcov_rewrite (void);

extern void set_gcov_fn_fixed_up (int fixed_up);
extern int get_gcov_fn_fixed_up (void);

/* "Counts" stored in gcda files can be a real counter value, or
   an target address. When differentiate these two types because
   when manipulating counts, we should only change real counter values,
   rather target addresses.  */

static inline gcov_type
gcov_get_counter (void)
{
#ifndef IN_GCOV_TOOL
  /* This version is for reading count values in libgcov runtime:
     we read from gcda files.  */

  if (get_gcov_fn_fixed_up ())
    {
      gcov_read_counter ();
      return 0;
    }
  else
    return gcov_read_counter ();
#else
  /* This version is for gcov-tool. We read the value from memory and
     multiply it by the merge weight.  */

  return gcov_read_counter_mem () * gcov_get_merge_weight ();
#endif
}

/* Similar function as gcov_get_counter(), but handles target address
   counters.  */

static inline gcov_type
gcov_get_counter_target (void)
{
#ifndef IN_GCOV_TOOL
  /* This version is for reading count target values in libgcov runtime:
     we read from gcda files.  */

  if (get_gcov_fn_fixed_up ())
    {
      gcov_read_counter ();
      return 0;
    }
  else
    return gcov_read_counter ();
#else
  /* This version is for gcov-tool.  We read the value from memory and we do NOT
     multiply it by the merge weight.  */

  return gcov_read_counter_mem ();
#endif
}

#endif /* !inhibit_libc */

#endif /* GCC_LIBGCOV_H */
