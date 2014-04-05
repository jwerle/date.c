
/**
 * `test.c' - date.c
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

#include <stdio.h>
#include <assert.h>
#include <ok.h>
#include "date.h"

#define test(name, e) {                   \
  if (!(e)) {                             \
    fprintf(stderr, "failed: %s\n", # e); \
    return 1;                             \
  } else {                                \
    ok(name);                             \
  }                                       \
}

int
main (void) {
  date_t *date = NULL;
  test("date_now", date_now() > 0);
  test("date_new", date_new(0));
  test("date_parse", date_parse("Fri Feb 28 2014 16:48:24 GMT-0500 (EST)") > 0);
  test("date_parse", date_parse("Fri Feb 2014 16:48:24 GMT-0500 (EST)") > 0);
  ok_done();
  return 0;
}
