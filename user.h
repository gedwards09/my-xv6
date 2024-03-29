/*
 * Revisions:
 *   GJE p1b - add getppid system call
 *           - add getreadcount system call
 *   GJE p2b - add getpinfo system call
 *           - add settickets system call
 *           - add yield system call
 *   GJE p3b - add mprotect system call
 *           - add munprotect system call
 *   GJE p4b - add clone system call
 *           - add join system call
 *           - add thread and mutex lock library functions
 */

struct stat;
struct rtcdate;
struct pstat;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int getppid(void);
int getreadcount(void);
int settickets(int);
int getpinfo(struct pstat*);
int yield(void);
int mprotect(void*, int);
int munprotect(void*, int);
int clone(void (*fcn)(void*, void*), void* arg1, void* arg2, void* stack);
int join(void** stack);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);

typedef struct __lock_t
{
	int ticket;
	int turn;
} lock_t;

int thread_create(void (*)(void*, void*), void*, void*);
int thread_join(void);
void lock_init(lock_t* plock);
void lock_acquire(lock_t* plock);
void lock_release(lock_t* plock);
