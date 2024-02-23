#ifndef __PAGE_H
#define __PAGE_H

/* This is used in tests which watch for a signal going up and down */
typedef enum
{
  SEEN_NEITHER = 0x00,
  SEEN_RISING  = 0x01,
  SEEN_FALLING = 0x02,
  SEEN_BOTH    = 0x03,
}
SEEN_EDGE;

typedef struct
{
  SEEN_EDGE flag;
  uint32_t  gpio;
}
EDGE_STATUS;

#endif
