/* ngcLoadEffect.c - Loading-screen transitions and frame-buffer capture. Status: matched.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcLoadEffect.c; assembly: build/2049LE/asm/NGC/ngcLoadEffect.s */
#include "rush/rush_protos.h"

static void perform_effect_fade(void);
static void perform_effect_scale(void);
static void perform_effect_move(void);
static void d2dDrawSprite_lg(float alpha);
static void draw_sprite();
static void step_ce_showimg(void);
static void step_ce_loading(void);
static void set_stage();
static void pre_render(void);
static void post_render(void);

int g_bIfCopyScreen = 1;
static unsigned char *_leFrameBuffer;
int _leFrameBufferSize;
static int s_threadId;
static _leEThreadState s_threadState;
static anon_draw_count_4 s_leDC;
static int s_leVSyncCallbackEnabled;
static volatile int s_step;
static _leEStage s_stage;
int g_bShowLoadingEffect;
unsigned long g_LoadFrameCount;
int g_bSaveScreen;
static char debug_info[1024];
static unsigned char s_packetBuf[1600] __attribute__((aligned(64)));
static anon_start_time_duration_20 s_leCE;
_GXTexObj g_BGTexObj;
static anon_flag_src_fb_6c s_leEC;
static _leINFO s_leInfo;
_GXTexObj g_BGTexObj_Loading;

/* 0x800a8910, size 0x54 */
void leIniFrameCopy(void)
{
    _leFrameBufferSize = GXGetTexBufferSize(640,480,6,0,0);
    _leFrameBuffer = AllocateMemoryHeap(0,_leFrameBufferSize);
    DCInvalidateRange(_leFrameBuffer,_leFrameBufferSize);
    leCreateSprBuf();
}

/* Stripped by the linker: this unused loading-texture initializer is kept only to preserve the original data emission order. */
static void leIniFrameCopy_Loading(void)
{
    GXInitTexObj(&g_BGTexObj_Loading,0,0,0,6,0,0,0);
}

/* 0x800a8964, size 0x74 */
void leCopyFrameBufferTex(void)
{
    _GXRenderModeObj_2 *rm; /* r0 */
    if (g_bIfCopyScreen == 1) {
        rm = (_GXRenderModeObj_2 *)ngcGetRenderModeObj();
        GXSetTexCopySrc(0,0,rm->fbWidth,rm->efbHeight);
        GXSetTexCopyDst(rm->fbWidth,rm->efbHeight,6,0);
        GXCopyTex(_leFrameBuffer,1);
        g_bSaveScreen=0;
    }
}

/* 0x800a89d8, size 0x44 */
void init_cs(void)
{
    OSInitSemaphore(&s_leInfo.sp,1);
    s_leInfo.cs_owner_thread=0;
    s_leInfo.cs_lock_count=0;
}

/* 0x800a8a1c, size 0x68 */
void enter_cs(void)
{
    char *pThread; /* r0 */
    unsigned long tid; /* r0 */
    pThread=(char *)OSGetCurrentThread();
    tid=(unsigned long)pThread;
    if(tid != s_leInfo.cs_owner_thread) {
        OSWaitSemaphore(&s_leInfo.sp);
        s_leInfo.cs_owner_thread=tid;
    }
    s_leInfo.cs_lock_count++;
}

/* 0x800a8a84, size 0x54 */
void exit_cs(void)
{
    char *pThread; /* r0 */
    pThread=(char *)OSGetCurrentThread();
    if(s_leInfo.cs_owner_thread==(unsigned long)pThread) {
        if(--s_leInfo.cs_lock_count==0) {
            s_leInfo.cs_owner_thread=0;
            OSSignalSemaphore(&s_leInfo.sp);
        }
    }
}

/* 0x800a8ad8, size 0x84 */
static void perform_effect_fade(void)
{
    int fade_time; /* r0 */
    fade_time=(int)(s_leDC.draw_count-s_leEC.fade_start_time);
    if(fade_time<s_leEC.fade_duration)
        s_leEC.draw_alpha=s_leEC.fade_start_val + (float)fade_time*(s_leEC.fade_end_val-s_leEC.fade_start_val)/s_leEC.fade_duration;
    else s_leEC.draw_alpha=s_leEC.fade_end_val;
}

/* 0x800a8b5c, size 0xe4 */
static void perform_effect_scale(void)
{
    int scale_time; /* r0 */
    float scale; /* f3 */
    scale_time=(int)(s_leDC.draw_count-s_leEC.scale_start_time);
    if(scale_time<s_leEC.scale_duration)
        scale=s_leEC.scale_start_val+(float)scale_time*(s_leEC.scale_end_val-s_leEC.scale_start_val)/s_leEC.scale_duration;
    else scale=s_leEC.scale_end_val;
    s_leEC.draw_w=scale*s_leEC.src_w;
    s_leEC.draw_h=scale*s_leEC.src_h;
}

/* 0x800a8c40, size 0x104 */
static void perform_effect_move(void)
{
    int move_time; /* r6 */
    int dx; /* r7 */
    int dy; /* r8 */
    int cy; /* r5 */
    move_time=s_leDC.draw_count-s_leEC.move_start_time;
    if(move_time<s_leEC.move_duration) {
        dx=move_time*s_leEC.move_dx/s_leEC.move_duration;
        dy=move_time*s_leEC.move_dy/s_leEC.move_duration;
    } else {dx=s_leEC.move_dx;dy=s_leEC.move_dy;}
    cy=s_leEC.dest_cy+dy;
    s_leEC.draw_x=s_leEC.dest_cx+dx-(s_leEC.draw_w>>1);
    s_leEC.draw_y=cy-(s_leEC.draw_h>>1);
}

/* 0x800a8d44, size 0x78 */
void leCreateSprBuf(void)
{
    _GXRenderModeObj_2 *rm; /* r0 */
    rm=(_GXRenderModeObj_2 *)ngcGetRenderModeObj();
    GXInitTexObj(&g_BGTexObj,_leFrameBuffer,rm->fbWidth,rm->efbHeight,6,0,0,0);
    GXInitTexObjLOD(&g_BGTexObj,1,1,0.0f,0.0f,0.0f,0,0,0);
}

/* 0x800a8dbc, size 0x58 */
static void d2dDrawSprite_lg(float alpha)
{
    BlitOneTpl_XYWHRGBA(485.0f,141.0f,128.0f,128.0f,0xffffff00+(unsigned long)(255.0f*alpha),&g_MenuTex._TexObjs[4],&g_MenuTex_a._TexObjs[4]);
}

/* 0x800a8e14, size 0x4 */
void leDrawSprBuf()
{

}

/* 0x800a8e18, size 0x30 */
static void draw_sprite(void * dest_fb, int dest_x, int dest_y, int dest_w, int dest_h, void * src_fb, int src_x, int src_y, int src_w, int src_h)
{
    leDrawSprBuf(dest_fb,dest_x,dest_y,dest_w,dest_h,src_fb,src_x,src_y,src_w,src_h);
}

/* 0x800a8e48, size 0x58 */
void d2dBitblit(_tD2DFrameBuf *src_fb, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, int dest_w, int dest_h)
{
    ((void (*)())draw_sprite)(0,dest_x,dest_y,dest_w,dest_h,src_fb,src_x,src_y,src_w,src_h,0);
}

/* 0x800a8ea0, size 0x40 */
void d2dDrawSprite(_tD2DFrameBuf *src_fb, int dest_x, int dest_y, int dest_w, int dest_h, float alpha)
{
    d2dBitblit(src_fb,0,0,src_fb->_w,src_fb->_h,dest_x,dest_y,dest_w,dest_h);
}

/* 0x800a8ee0, size 0xf0 */
static void step_ce_showimg(void)
{
    if(s_step==0 && (int)(s_leDC.draw_count-s_leEC.fade_start_time)>s_leEC.fade_duration) {
        if(s_leCE.iparam[2]) {
            s_leEC.flag |=1;
            s_leEC.fade_start_val=s_leCE.fparam[0];
            s_leEC.fade_end_val=0.0f;
            s_leEC.fade_duration=s_leCE.iparam[1];
            s_leEC.fade_start_time=s_leDC.draw_count;
            s_step++;
        } else s_stage=leEStage_Idle;
    }
    if(s_step==1 && (int)(s_leDC.draw_count-s_leEC.fade_start_time)>s_leEC.fade_duration) s_stage=leEStage_Idle;
    if(s_stage != leEStage_Idle) ((void (*)(int,int,int,float))leClearBackBuffer)(0,0,0,1.0f);
}

/* 0x800a8fd0, size 0x4 */
static void step_ce_loading(void)
{

}

/* 0x800a8fd4, size 0x20 */
static void set_stage(_leEStage stage)
{
    s_stage=stage;
    s_step=0;
    s_leDC.draw_count=s_leInfo.vsync_count;
}

/* 0x800a8ff4, size 0x2fc */
static void pre_render(void)
{
    switch(s_stage) {
    case leEStage_ShowImage: step_ce_showimg(); break;
    case leEStage_Loading: step_ce_loading(); break;
    case leEStage_HideScreen:
        if((int)(s_leDC.draw_count-s_leEC.fade_start_time)>s_leEC.fade_duration) s_stage=leEStage_Idle;
        else ((void (*)(int,int,int,float))leClearBackBuffer)(0,0,0,1.0f);
        break;
    case leEStage_PrepareLoading:
        {
            int offset=g_LoadFrameCount%60;
            float alpha;
            if(offset<30) alpha=offset/30.0f;
            else alpha=(60-offset)/30.0f;
            ((void (*)(int,int,int,float))leClearBackBuffer)(0,0,0,1.0f);
            ((void (*)(_tD2DFrameBuf*,int,int,int,int,float))d2dDrawSprite_lg)(&s_leInfo.fbUser[1],s_leCE.iparam[1],s_leCE.iparam[2],s_leInfo.fbUser[1]._w,s_leInfo.fbUser[1]._h,alpha);
        }
        break;
    case leEStage_PrepareLoading2:
        {
            int offset=(s_leDC.draw_count-s_leCE.iparam[0])%60;
            float alpha;
            float a;
            if(offset<30) alpha=offset/30.0f;
            else alpha=(60-offset)/30.0f;
            ((void (*)(int,int,int,float))leClearBackBuffer)(0,0,0,1.0f);
            if(s_leDC.draw_count-s_leCE.iparam[0]<100) {
                a=(s_leDC.draw_count-s_leCE.iparam[0])/100.0f;
                d2dDrawSprite(&s_leInfo.fbUser[3],0,0,s_leInfo.fbUser[3]._w,s_leInfo.fbUser[3]._h,1.0f-a);
                ((void (*)(_tD2DFrameBuf*,int,int,int,int,int,int,int,int,float))d2dBitblit)(&s_leInfo.fbUser[0],0,0,512,400,0,0,((_tD2DFrameBuf*)0)->_w,((_tD2DFrameBuf*)0)->_h,a);
            } else {
                ((void (*)(_tD2DFrameBuf*,int,int,int,int,int,int,int,int,float))d2dBitblit)(&s_leInfo.fbUser[0],0,0,512,400,0,0,((_tD2DFrameBuf*)0)->_w,((_tD2DFrameBuf*)0)->_h,1.0f);
            }
            d2dDrawSprite(&s_leInfo.fbUser[2],s_leCE.iparam[1],s_leCE.iparam[2],s_leInfo.fbUser[2]._w,s_leInfo.fbUser[2]._h,alpha);
        }
        break;
    }
}

/* 0x800a92f0, size 0x30 */
static void post_render(void)
{
    if(s_stage != leEStage_Idle) leSwapBuffer(0);
}

/* 0x800a9320, size 0x110 */
void do_loading_effect(void)
{
    int src_x; /* r4 */
    int src_y; /* r5 */
    pre_render();
    if(s_stage != leEStage_Idle) {
        if(s_leEC.flag&1) perform_effect_fade();
        else s_leEC.draw_alpha=1.0f;
        if(s_leEC.flag&2) perform_effect_scale();
        else {s_leEC.draw_w=s_leEC.dest_w;s_leEC.draw_h=s_leEC.dest_h;}
        if(s_leEC.flag&4) perform_effect_move();
        else {s_leEC.draw_x=s_leEC.dest_cx-(s_leEC.draw_w>>1);s_leEC.draw_y=s_leEC.dest_cy-(s_leEC.draw_h>>1);}
        src_x=s_leEC.src_cx-(s_leEC.src_w>>1);
        src_y=s_leEC.src_cy-(s_leEC.src_h>>1);
        if(s_leEC.src_fb) ((void (*)())d2dBitblit)(s_leEC.src_fb,src_x,src_y,s_leEC.src_w,s_leEC.src_h,s_leEC.draw_x,s_leEC.draw_y,s_leEC.draw_w,s_leEC.draw_h,s_leEC.draw_alpha);
        post_render();
    }
}

/* 0x800a9430, size 0x4 */
void leSwapBuffer()
{

}

/* 0x800a9434, size 0x4 */
void leClearBackBuffer()
{

}

/* 0x800a9438, size 0x74 */
void leInitialize(int cdMode, int prio, int stack_size)
{
    s_threadId=0;
    memset(&s_leInfo,0,sizeof(s_leInfo));
    s_leInfo.cdMode=cdMode;
    s_leInfo.thread_prio=prio;
    s_leInfo.thread_stack_size=stack_size;
    init_cs();
}

/* 0x800a94ac, size 0x54 */
void leStartLoadingEffect(void)
{
    if(s_threadId==0) {
        enter_cs();
        s_threadState=leETS_Working;
        s_stage=leEStage_Idle;
        s_step=0;
        g_bIfCopyScreen=0;
        g_bRenderToTex=0;
        g_bShowLoadingEffect=1;
        s_leVSyncCallbackEnabled=1;
        exit_cs();
    }
}

/* 0x800a9500, size 0x3c */
void leEndLoadingEffect(void)
{
    enter_cs();
    g_bIfCopyScreen=1;
    g_bRenderToTex=1;
    g_bShowLoadingEffect=0;
    s_threadState=leETS_Quitting;
    exit_cs();
}

/* 0x800a953c, size 0x4 */
void leWaitIdleStage(void)
{

}

/* 0x800a9540, size 0x34 */
void lePrepareLoading(void)
{
    leWaitIdleStage();
    enter_cs();
    ((void (*)())set_stage)(leEStage_PrepareLoading,1);
    exit_cs();
}

