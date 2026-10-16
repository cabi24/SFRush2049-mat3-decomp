/* sched.c - N64-style scheduler state kept for the port. Matches. */

typedef struct SCClient_s {
    struct SCClient_s *next;
    char *msgQ;
} SCClient;

typedef struct OSScTask_s {
    struct OSScTask_s *next;
    unsigned int state;
    unsigned int flags;
    void *framebuffer;
    char list;
    char *msgQ;
    char msg;
} OSScTask;

typedef struct {
    signed short type;
    char misc[30];
} OSScMsg;

struct {
    OSScMsg retraceMsg;
    OSScMsg prenmiMsg;
    char interruptQ;
    char intBuf[8];
    char cmdQ;
    char cmdMsgBuf[8];
    char thread;
    SCClient *clientList;
    OSScTask *audioListHead;
    OSScTask *gfxListHead;
    OSScTask *audioListTail;
    OSScTask *gfxListTail;
    OSScTask *curRSPTask;
    OSScTask *curRDPTask;
    unsigned int frameCount;
    int doAudio;
} sc;
