#ifndef __PAGE_H
#define __PAGE_H

#include <stdint.h>

typedef void (*page_entry_fn_t)(void);
typedef void (*page_exit_fn_t)(void);
typedef void (*page_tests_fn_t)(void);

typedef struct _page
{
  page_entry_fn_t entry_fn;
  page_exit_fn_t  exit_fn;
  page_tests_fn_t tests_fn;
  uint32_t repeat_pause_ms;
}
page_t;


#endif
