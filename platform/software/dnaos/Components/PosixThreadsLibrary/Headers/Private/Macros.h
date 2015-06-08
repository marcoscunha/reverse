/***************************************************
 * MACROS.H : PTHREADS MACROS                       *
 * Copyright TIMA Laboratory - 2007, Patrice GERIN * 
 ***************************************************/

#ifndef _PTHREAD_MACROS_H_
#define _PTHREAD_MACROS_H_

#define ASSERT(expression)                          \
if(expression) {                                    \
  printf("%s : assertion failed in %s - %s\n",      \
      __FILE__,__func__, #expression);              \
}  

#define ASSERT_EXIT(expression,value)                      \
if(expression) {                                          \
  printf("%s : assertion failed in %s - %s exit %s\n",    \
      __FILE__,__func__, #expression, #value);            \
  exit(value);                                            \
}  

#define ASSERT_RETURN(expression,value)                      \
if(expression) {                                            \
  printf("%s : assertion failed in %s - %s return %s\n",    \
      __FILE__,__func__, #expression, #value);              \
  return(value);                                            \
}  

#endif
