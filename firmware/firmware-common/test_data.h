#ifndef __TEST_DATA_H
#define __TEST_DATA_H

/* Data structures for tests which span the 2 Picos */

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
