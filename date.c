
/**
 * `date.c' - date.c
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include "date.h"

#define EQ(a,b) (0 == strcmp(a,b))
#define STATE(T) DATE_PARSE_STATE_ ## T

#define strtolower(str) ({                       \
    int i = 0;                                   \
    for (; str[i]; ++i) {                        \
      str[i] = tolower(str[i]);                  \
    }                                            \
})

unsigned long long
date_now () {
  struct timeval tv;
  unsigned long long sec = 0;
  unsigned long long usec = 0;
  if (-1 == gettimeofday(&tv, NULL)) { return -1; }
  sec = (unsigned long long) (tv.tv_sec) * (1000);
  usec = (unsigned long long) (tv.tv_usec) / (1000);
  return (unsigned long long) sec + usec;
}

date_t *
date_new (unsigned long long ts) {
  date_t *date = (date_t *) malloc(sizeof(date_t));
  if (NULL == date) { return NULL; }
  ts = (ts > 0 ? ts : date_now());
  date->ts = ts;
  return date;
}


unsigned long long
date_parse (const char *src) {
  unsigned long long ts = 0;
  char *emsg;
  char *str = malloc(sizeof(char) + strlen(src));
  char *tmp = NULL;
  char buf[1024];
  char bufread[4028];
  size_t size = 0;
  int offset = 0;
  char ch = 0;
  date_parse_state_t hint = 0;
  date_parse_state_t state = 0;
  time_t timer = time(NULL);
  struct tm *tmv = NULL;
  struct tm tval;
  tmv = gmtime(&timer);

  // grammar
  char zone[6]; // (( "+" / "-" ) 4DIGIT)
  char second[3]; // (2DIGIT)
  char minute[3]; // (2DIGIT)
  char hour[3]; // (2DIGIT)
  char tod[12]; // ( hour ":" minute [ ":" second ])
  char time[20]; // (tod FWS zone)
  char day[3]; //  ([FWS] 1*2DIGIT)
  char month[4]; // Jan - Dec
  char year[5]; // 2014
  char date[10]; // day month year
  char day_name[4]; // Mon - Sun
  char date_time[32]; // [ day_name"," ] date FWS time

#define X(v) v[0] = '\0';

  X(zone);
  X(second);
  X(minute);
  X(hour);
  X(tod);
  X(time);
  X(day);
  X(month);
  X(year);
  X(date);
  X(day_name);
  X(date_time);

#undef X

#define next() str[offset++]
#define rewind() str[--offset];
#define peek() str[offset]
#define prev() str[offset - 2]
#define push(c) buf[size++] = c;
#define join() (buf[size] = '\0', size = 0, buf)

#define CLEANUP(S) ({                            \
    state = STATE(CLEANUP);                      \
    hint = STATE(S);                             \
})

#define empty(str) ({                            \
    int i = 0;                                   \
    for (; str[i]; i++) {  str[i] = 0; }         \
    size = 0;                                    \
})

#define set(str) ({                              \
    strcpy(str, join());                         \
})

#define E(str) ({                                \
    emsg = str, state = STATE(PARSE_ERROR);      \
    goto scan;                                   \
})

#define pushstr(str) ({                          \
    int i = 0;                                   \
    for (; str[i]; ++i) { push(str[i]); }        \
})

#define DAY_PARSE_ERROR() ({                     \
    prev();                                      \
    E("Day parse error");                        \
})

#define MONTH_PARSE_ERROR() ({                   \
    prev();                                      \
    E("Day parse error");                        \
})

  str = strdup(src);
  state = STATE(BEGIN);
  strtolower(str);

scan:
  while ((ch = next())) {

    switch (state) {
      case STATE(CLEANUP):
        while (' ' == next()) ;
        state = hint;
        hint = 0;
        break;

      case STATE(BEGIN):
        switch (ch) {
          case '\0':
            state = STATE(EOS);
            break;

          case '0': case '1':
          case '2': case '3':
          case '4': case '5':
          case '6': case '7':
          case '8': case'9':
            state = STATE(DAY);
            push(ch);
            break;

          case 'm': // mon, mar, may
            ch = next();
            if ('o' == ch) { // mo
              ch = next();
              if ('n' == ch) { // mon
                state = STATE(DAY_NAME);
                pushstr("Mon");
              } else {
                DAY_PARSE_ERROR();
              }
            } else if ('a' == ch) {
              ch = next();
              if ('r' == ch || 'y' == ch) { // mar, may
                pushstr("Ma"); push(ch);
                state = STATE(MONTH_NAME);
              } else {
                MONTH_PARSE_ERROR();
              }
            } else {
              MONTH_PARSE_ERROR();
            }
            break;

          case 'w': // wed
          case 't': // tue, thu
            if ('w' == ch) {
              if ('e' == next() && 'd' == next()) { // wed
                pushstr("Wed");
              } else {
                DAY_PARSE_ERROR();
              }
            } else if ('t' == ch) {
              ch = next();
              if ('u' == ch && 'e' == next()) { // tue
                pushstr("Tue");
              } else if ('h' == ch && 'u' == next()) { // thu
                pushstr("Thu");
              } else {
                DAY_PARSE_ERROR();
              }
            } else {
              DAY_PARSE_ERROR();
            }
            state = STATE(DAY_NAME);
            break;

          case 's': // sep, sat, sun
            ch = next();
            if ('e' == ch && 'p' == next()) { // sep
              pushstr("sep");
              state = STATE(MONTH_NAME);
            } else if ('a' == ch && 't' == next()) { // sat
              pushstr("sat");
              state = STATE(DAY_NAME);
            } else if ('u' == next() && 'n' == next()) { // sun
              pushstr("sun");
              state = STATE(DAY_NAME);
            } else {
              MONTH_PARSE_ERROR();
            }
            break;

          case 'j': // jan, jun, jul
            ch = next();
            if ('a' == ch && 'n' == next()) { // jan
              pushstr("sep");
              state = STATE(MONTH_NAME);
            } else if ('u' == ch) {
              if ('n' == next()) { // jun
                pushstr("jun");
                state = STATE(MONTH_NAME);
              } else if ('l' == next()) { // jul
                pushstr("jul");
                state = STATE(MONTH_NAME);
              } else {
                MONTH_PARSE_ERROR();
              }
            } else {
              MONTH_PARSE_ERROR();
            }
            break;

          case 'a': // apr, aug
            ch = next();
            if ('p' == ch && 'r' == next()) { // apr
              pushstr("apr");
              state = STATE(MONTH_NAME);
            } else if ('u' == ch && 'g' == next()) { // aug
              pushstr("aug");
              state = STATE(MONTH_NAME);
            } else {
              MONTH_PARSE_ERROR();
            }
            break;

          case 'o': // oct
            if ('o' == ch && 'c' == next() && 't' == next()) {
              pushstr("oct");
              state = STATE(MONTH_NAME);
            } else {
              MONTH_PARSE_ERROR();
            }
            break;

          case 'n': // nov
            if ('n' == ch && 'o' == next() && 'v' == next()) {
              pushstr("nov");
              state = STATE(MONTH_NAME);
            } else {
              MONTH_PARSE_ERROR();
            }
            break;

          case 'd': // dec
            if ('e' == next() && 'c' == next()) {
              pushstr("dec");
              state = STATE(MONTH_NAME);
            } else {
              MONTH_PARSE_ERROR();
            }
            break;

          case 'f': // feb, fri
            ch = next();
            if ('e' == ch && 'b' == next()) { // feb
              pushstr("Feb");
              state = STATE(MONTH_NAME);
            } else if ('r' == ch && 'i' == next()) { // fri
              pushstr("Fri");
              state = STATE(DAY_NAME);
            } else {
              MONTH_PARSE_ERROR();
            }
            break;

          case '+':
          case '-':
            state = STATE(ZONE);
            push(ch);
            break;
        }
        break;

      case STATE(ZONE):
        if ('(' == ch) {
          set(zone);
          state = STATE(US_TIME_ZONE);
        } else if (ch < 0 && ch > 9) {
          state = STATE(PARSE_ERROR);
        } else if (ch >= '0' && ch <= '9') {
          push(ch);
        } else if (' ' == ch || '\0' == ch) {
          state = STATE(TIME);
        }
        break;

      case STATE(US_TIME_ZONE):
        switch (ch) {
          case 'E':
          case 'C':
          case 'M':
          case 'P':
          case 'S':
          case 'D':
            push(ch);
            break;

          case 'T':
            empty(buf);
            if (EQ("EDT", tmp) || EQ("EST", tmp) ||
                EQ("CDT", tmp) || EQ("CST", tmp) ||
                EQ("MDT", tmp) || EQ("MST", tmp) ||
                EQ("PDT", tmp) || EQ("PST", tmp)) {
              if (')' == peek()) { next(); }
              state = STATE(TIME);
            } else {
              state = STATE(PARSE_ERROR);
            }
            break;
        }
        break;

      case STATE(SECOND):
        if (0 == strlen(second)) {
          if (':' == ch) {
            ch = next();
          }

          if (ch >= 0 && ch <= '9') {
            if (' ' == peek()) {
              push('0');
            } else {
              push(ch);
              ch = next();
            }
            if (ch >= 0 && ch <= '9') {
              push(ch);
            }
          } else {
            E("Time of day second parse error");
          }
          set(second);
        }
        state = STATE(TIME_OF_DAY);
        break;

      case STATE(MINUTE):
        if (0 == strlen(minute)) {
          if (':' == ch) { ch = next(); }
          if (ch >= 0 && ch <= '9') {
            if (':' == peek()) { push('0'); }
            else { push(ch); ch = next(); }
            if (ch >= 0 && ch <= '9') { push(ch); }
          } else { E("Time of day minute parse error"); }
          set(minute);
        }
        state = STATE(SECOND);
        break;

      case STATE(HOUR):
        if (0 == strlen(hour)) {
          if (':' == peek()) {
            push('0');
            push(ch);
            ch = next();
          } else if (peek() >= '0' && peek() <= '9') {
            push(ch);
            push(ch = next());
            set(hour);
            if (' ' == peek()) {
              pushstr("00");
              set(minute);
              pushstr("00");
              set(second);
              state = STATE(TIME_OF_DAY);
            } else if (':' != peek()) {
              E("Time of day hour parse error. Expecting `:'");
            } else {
              ch = next();
            }
          } else {
            E("Time of day hour parse error number or `:'");
          }
          empty(buf);
        }
        state = STATE(MINUTE);
        break;

      case STATE(TIME):
        empty(buf);
        pushstr(tod);
        push(' ');
        pushstr(zone);
        set(time);
        break;

      case STATE(TIME_OF_DAY):
        if (strlen(hour) && strlen(minute) && strlen(second)) {
          empty(buf);
          pushstr(hour); push(':'); pushstr(minute); push(':'); pushstr(second);
          set(tod);
          state = STATE(ZONE);
        } else {
          E("Time of day parse error");
        }
        break;

      case STATE(DAY):
        if (strlen(day) > 0) {
          state = STATE(YEAR);
        } else {
          if (' ' == ch ) { CLEANUP(DAY); break; }
          if (' ' == peek()) {
            if ('0' == ch) ch = '1'; // 00 => 01
            push(ch);
            set(day);
            state = STATE(YEAR);
          } else if (isdigit(peek())) { // 000
            tmp = join(); // copy
            pushstr("01"); // day
            set(day); // set default day
            empty(buf); // clear
            pushstr(tmp); // restore
            state  = STATE(YEAR);
            rewind(); rewind(); // rewind
            push(peek());
          } else {
            push(ch);
          }
        }
        break;

      case STATE(DAY_NAME):
        state = STATE(BEGIN);
        set(day_name);
        break;

      case STATE(MONTH_NAME):
        state = STATE(BEGIN);
        set(month);
        break;

      case STATE(YEAR):
        if (' ' == ch) break;
        if (strlen(year) > 0) {
          state = STATE(HOUR);
        } else if (2 == size) {
          if (' ' == peek()) {
            tmp = join();
            pushstr("20");
            pushstr(tmp);
            state = STATE(HOUR);
          } else if (':' == peek()) {
            rewind(); rewind();
            *tmp = '\0';
            pushstr(tmp);
            set(year);
            state = STATE(HOUR);
          } else {
            push(ch);
          }
        } else if (size == 4) {
          set(year);
          state = STATE(HOUR);
          rewind();
        } else {
          push(ch);
        }
        break;

      case STATE(DATE):
        break;

      case STATE(DATE_TIME):
        break;

      case STATE(EOS):
        break;

      case STATE(PARSE_ERROR):
        fprintf(stderr, "error: %s\n       at (%s[%c])\n",
            emsg, buf, src[offset - 1]);
        return -1;
    }
  }

  tval.tm_sec = atoi(second);

  return mktime(&tval);
}
