#ifndef STDIO_OUTBUF_SIZE
#define STDIO_OUTBUF_SIZE	128
#endif

typedef struct _stdio_file FILE;

struct _stdio_file_ops {
        void *(* init)(void);
        size_t (* write)(const uint8_t *buf, size_t len, void *ops_data);
};

struct _stdio_file {
        const struct _stdio_file_ops *ops;
        void *ops_data;
        unsigned int outbuf_head; // where characters are added
        unsigned int outbuf_tail; // where characters are removed
        uint8_t outbuf[STDIO_OUTBUF_SIZE];
};


extern FILE *stdout;

int printf(const char *fmt, ...)  __attribute__((__format__ (printf, 1, 2)));
int vfprintf(FILE *f, const char *fmt, va_list args);
void fflush(FILE *f);
void fputc(int c, FILE *f);
int snprintf(char *buf, size_t n, const char *fmt, ...) __attribute__((__format__ (printf, 3, 4)));
int vsnprintf(char *buf, size_t n, const char *fmt, va_list args);
