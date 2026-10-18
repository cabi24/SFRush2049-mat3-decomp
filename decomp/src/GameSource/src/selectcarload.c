/* Car-selection model loading, appearance and window visibility.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\selectcarload.c; assembly: build/2049LE/asm/GameSource/src/selectcarload.s */
#include "rush/rush_protos.h"

static void SetSelectTireSizes(signed short slot, signed short player);
static void CreateSelectCar(signed short slot, signed short player, signed short cam);

anon_created_hidden_18 gSelectCar[52];
anon_wave_loading_c gSelectCarWavePtrs[16];

/* 0x8003b78c, size 0x118 */
void InitSelectCarLoad(void)
{
    int i;
    for(i=0;i<13;i++) gLink[i].car=i;
    for(i=0;i<16;i++) {gSelectCarWavePtrs[i].wave=-1;gSelectCarWavePtrs[i].slot=-1;gSelectCarWavePtrs[i].loading=0;}
}

/* 0x8003b8a4, size 0xd8 */
void InitVisualsCarScr(void)
{
    int i;
    for(i=0;i<52;i++) {
        gSelectCar[i].created=0;gSelectCar[i].hidden=1;gSelectCar[i].ready=0;
        gSelectCar[i].waveInst=-1;gSelectCar[i].loading=0;gSelectCar[i].PaintJob=-1;
        gSelectCar[i].rims=-1;gSelectCar[i].frontTireScale=-1.0f;gSelectCar[i].rearTireScale=-1.0f;
        gCarPaintJobInited[i]=0;
    }
}

/* 0x8003b97c, size 0x2a4 */
int LoadSelectCar(signed short slot, signed short index, int *waveInst)
{
    int i;
    for(i=0;i<16;i++) if(gSelectCarWavePtrs[i].slot==slot && gSelectCarWavePtrs[i].loading) {*waveInst=gSelectCarWavePtrs[i].wave;return 1;}
    for(i=0;i<16;i++) if(gSelectCarWavePtrs[i].slot==slot) break;
    if(i==16) {
        for(i=0;i<16;i++) if(gSelectCarWavePtrs[i].slot<0) break;
        if(i==16) return 0;
        gSelectCarWavePtrs[i].slot=slot;
    }
    *waveInst=gSelectCarWavePtrs[i].wave=LoadWave(index+64,1,1,0,0);
    gSelectCarWavePtrs[i].loading=1;
    gCarModelNums[slot]=gSelectCarWavePtrs[i].wave;
    return 1;
}

/* 0x8003bc20, size 0xac */
void UnloadSelectCar(signed short slot)
{
    int i;
    for(i=0;i<16;i++) if(gSelectCarWavePtrs[i].slot==slot) {
        while(!WaveInstanceReady(gSelectCarWavePtrs[i].wave)) {}
        UnloadWaveInstance(gSelectCarWavePtrs[i].wave,1);
        gSelectCarWavePtrs[i].wave=-1;gSelectCarWavePtrs[i].slot=-1;gSelectCarWavePtrs[i].loading=0;
        break;
    }
}

/* 0x8003bccc, size 0x178 */
static void SetSelectTireSizes(signed short slot, signed short player)
{
    signed short tire;
    _tCar *carp;
    tUVMatrix *mat;
    carp=gCarModels[(short)(slot%13)];
    for(tire=0;tire<4;tire++) {
        mat=ZOID_FindMatrix(gCarParts[slot].tires[tire]);
        mat->uvs[3][0]=carp->tireR[tire][0];
        mat->uvs[3][1]=1.25f+carp->tireR[tire][1];
        mat->uvs[3][2]=carp->tireR[tire][2];
        if(tire>=2) {
            if(gRearTireScale[player+1][(short)(slot%13)]!=1.0f) ScaleUV(unitmatrix,mat->uvs,gRearTireScale[player+1][(short)(slot%13)]);
            else CopyUV(unitmatrix,mat->uvs);
        } else {
            if(gFrontTireScale[player+1][(short)(slot%13)]!=1.0f) ScaleUV(unitmatrix,mat->uvs,gFrontTireScale[player+1][(short)(slot%13)]);
            else CopyUV(unitmatrix,mat->uvs);
        }
    }
}

/* 0x8003be44, size 0xbc */
static void CreateSelectCar(signed short slot, signed short player, signed short cam)
{
    CreateCar(slot,cam,0);
    MBOX_ShowObject(gCarParts[slot].frame,1,cam);
    MBOX_HideObject(gCarParts[slot].hood,1,cam);
    MBOX_HideObject(gCarParts[slot].brakes,1,cam);
    SetSelectTireSizes(slot,player);
}

/* 0x8003bf00, size 0x6c0 */
void UpdateSelectCar(signed short slot, signed short player, unsigned char PaintJob, signed short sheenIdx, float *pos, float (*uvs)[3], unsigned char alpha)
{
    float newuvs[3][3];
    signed short idx;
    signed short cam;
    tRGBA alphaCol={255,255,255,255};
    tTexture *wheelTex;
    idx=slot%13;
    if(gNumWindows<3) {if(player==2) player=0;if(player==3) player=1;}
    switch(player) {case 0:cam=1;break;case 1:cam=2;break;case 2:cam=4;break;case 3:cam=8;break;}
    if(alpha==0) {
        if(!gSelectCar[slot].hidden) {gSelectCar[slot].hidden=1;MBOX_HideObject(gCarParts[slot].frame,0,cam);}
        if(gSelectCar[slot].waveInst!=-1 && WaveInstanceReady(gSelectCar[slot].waveInst) && gSelectCar[slot].created) {
            UnloadSelectCar(slot);
            UnloadWaveInstance(gSelectCar[slot].pjModelNum,0);
            gSelectCar[slot].created=0;gSelectCar[slot].ready=0;gSelectCar[slot].waveInst=-1;gSelectCar[slot].loading=0;
            gSelectCar[slot].PaintJob=-1;gSelectCar[slot].rims=-1;
            gSelectCar[slot].frontTireScale=gSelectCar[slot].rearTireScale=-1.0f;
            ZOID_RemoveObjectTree(gCarParts[slot].frame,1);
            gCarParts[slot].frame=0;gCarParts[slot].brakes=0;gCarParts[slot].trim=0;
            gCarParts[slot].headlight[0]=0;gCarParts[slot].headlight[1]=0;gCarParts[slot].headlight[2]=0;gCarParts[slot].headlight[3]=0;
            gCarParts[slot].extraparts[0]=0;gCarParts[slot].extraparts[1]=0;gCarParts[slot].extraparts[2]=0;
            gCarParts[slot].hood=0;gCarParts[slot].sheen=0;
            gCarParts[slot].tires[0]=0;gCarParts[slot].tires[1]=0;gCarParts[slot].tires[2]=0;gCarParts[slot].tires[3]=0;
        }
    } else {
        if(!gSelectCar[slot].loading) gSelectCar[slot].loading=LoadSelectCar(slot,idx,&gSelectCar[slot].waveInst);
        if(!WaveInstanceReady(gSelectCar[slot].waveInst)) return;
        if(!gSelectCar[slot].ready) {gSelectCar[slot].ready=1;InitCarDynamicObjs(slot,idx,gCarModelNums[slot]);}
        if(!gSelectCar[slot].created) {
            gSelectCar[slot].created=1;gSelectCar[slot].hidden=0;
            CreateSelectCar(slot,player,cam);SetSheen(idx,player,sheenIdx);
        }
        if(gSelectCar[slot].hidden) {MBOX_ShowObject(gCarParts[slot].frame,0,cam);gSelectCar[slot].hidden=0;}
        if(gSelectCar[slot].PaintJob!=PaintJob) {
            if(gSelectCar[slot].PaintJob!=-1) UnloadWaveInstance(gSelectCar[slot].pjModelNum,0);
            gSelectCar[slot].pjModelNum=LoadWave(gCarPaintJobWaveNums[idx]+PaintJob,1,0,0,0);
            SetupCarColor(slot,idx,PaintJob);gSelectCar[slot].PaintJob=PaintJob;
        }
        if(idx==10 && PaintJob==6) MBOX_HideObject(gCarParts[slot].sheen,1,cam);
        else MBOX_ShowObject(gCarParts[slot].sheen,0,cam);
        if(idx==3) {if(PaintJob==5) MBOX_ShowObject(gCarParts[slot].extraparts[0],0,cam);else MBOX_HideObject(gCarParts[slot].extraparts[0],1,cam);}
        alphaCol.c[3]=alpha;SetFrameColor(slot,&alphaCol);
        if(idx==3) ZOID_SetColor(gCarParts[slot].extraparts[0],&alphaCol);
        CopyUV(uvs,newuvs);
        if(gFrameScale) {newuvs[0][1]*=FSCLS[gFrameScale-1];newuvs[1][1]*=FSCLS[gFrameScale-1];newuvs[2][1]*=FSCLS[gFrameScale-1];}
        ZOID_UpdateObject(gCarParts[slot].frame,pos,newuvs);
        if(gSelectCar[slot].rims!=gTireRims[player+1][idx]) {
            gSelectCar[slot].rims=gTireRims[player+1][idx];
            wheelTex=gWheelTextures[gTireRims[player+1][idx]+24];
            ZOID_SetTexture(gCarParts[slot].tires[0],wheelTex,-1);ZOID_SetTexture(gCarParts[slot].tires[1],wheelTex,-1);
            ZOID_SetTexture(gCarParts[slot].tires[2],wheelTex,-1);ZOID_SetTexture(gCarParts[slot].tires[3],wheelTex,-1);
        }
        if(gSelectCar[slot].frontTireScale!=gFrontTireScale[player+1][idx] || gSelectCar[slot].rearTireScale!=gRearTireScale[player+1][idx]) {
            gSelectCar[slot].frontTireScale=gFrontTireScale[player+1][idx];gSelectCar[slot].rearTireScale=gRearTireScale[player+1][idx];
            SetSelectTireSizes(slot,player);
        }
        ObjInstanceList[gCarParts[slot].frame].flags&=~16;ObjInstanceList[gCarParts[slot].frame].flags&=~32;
        ObjInstanceList[gCarParts[slot].frame].culling=0;
    }
}

