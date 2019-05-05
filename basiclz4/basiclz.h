#define MAX_FRAME_SIZE (1<<22)

#define LAZY_PARSE 1

#ifdef DEBUG
#error DEBUGGERY!
#define LOG(...)    { fprintf(__VA_ARGS__); fflush(stdout); }
#else
#define LOG(...)    /* __VA_ARGS__ */
#endif
