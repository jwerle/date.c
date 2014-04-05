
/**
 * `date.h' - date.c
 *
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

/**
 * `date_t' struct
 */

typedef struct date_s {
  // timestamp
  unsigned long long ts;
} date_t;

/**
 * Parse states
 */

#define DATE_PARSE_STATES()  \
  X(PARSE_ERROR) = 0,        \
  X(BEGIN),                  \
  X(CLEANUP),                \
  X(ZONE),                   \
  X(US_TIME_ZONE),           \
  X(SECOND),                 \
  X(MINUTE),                 \
  X(HOUR),                   \
  X(TIME_OF_DAY),            \
  X(TIME),                   \
  X(DAY),                    \
  X(DAY_NAME),               \
  X(MONTH_NAME),             \
  X(YEAR),                   \
  X(DATE),                   \
  X(DATE_TIME),              \
  X(EOS)                     \

typedef enum {
#define X(T) DATE_PARSE_STATE_ ## T
  DATE_PARSE_STATES()
#undef X
} date_parse_state_t;

/**
 * Returns the time in milliseconds
 * since the epoch (Jan 1st 1970). It
 * returns -1 on failure.
 */

unsigned long long
date_now ();

/**
 * Allocates a new `date_t' from a
 * given timestamp. It returns `NULL'
 * on failure.
 */

date_t *
date_new (unsigned long long);

/**
 * Parses a string representation of
 * a date and returns the number of
 * milliseconds since Jan 1st 1970. It
 * returns -1 on failure. This is
 * rfc2822 compliant excluding obsolete
 * date and time inputs.
 * See (http://tools.ietf.org/html/rfc2822#page-14)
 * for more information.
 */

unsigned long long
date_parse (const char *);

