#ifndef TV_LOG_H_
#define TV_LOG_H_
#define DEBUG_LOG 1 
#ifdef DEBUG_LOG
#define TvLogInfo(dbg_msg...) {\
   char msg1[250];                      \
   char msg2[250];                      \
   sprintf( msg1, dbg_msg );            \
   sprintf( msg2, "%s:%s:%d: %s\n", __FILE__, __FUNCTION__, __LINE__, msg1 );\
   printf( msg2 ) ; \
}

#define TvLogTrace() { \
    printf("%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__); \
}

#else
#define TvLogInfo(dbg_msg...) {}
#define TvLogTrace() {}

#endif //DEBUG_LOG

#endif //TV_LOG_H_
