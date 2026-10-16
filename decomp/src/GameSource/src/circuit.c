/* circuit.c - Circuit generation, race standings and password persistence.
   Status: five functions exact; ContinueCircuit remains a near miss.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\circuit.c; assembly: build/2049LE/asm/GameSource/src/circuit.s */
#include "rush/rush_protos.h"

static unsigned short _CircuitRandom(unsigned short max);

static int gCircuitPoints[6] = { 10, 7, 5, 3, 2, 1 };
int gNumCircuitTracks[4] = { 4, 5, 6, 6 };
static int gCircuitFog[4] = { 0, 4, 7, 9 };
static int gCircuitWind[4] = { 0, 1, 2, 2 };
static int gCircuitDiff[4] = { 0, 1, 2, 3 };
unsigned char gCircuitPlayer;
signed char gCircuitTrack;
signed char gCircuitSize;
static unsigned int gCircuitSeed;
static unsigned char gCircuitNumTrackRaces[6];
static unsigned char gTempCircuitFirstTrack[4];
static unsigned short gTempCircuitPoints[4][6];
anon_trackNum_backward_5 gCircuitTracks[24];
anon_name_totalPlace_4c gCircuitCars[6];

/* 0x8000a3f8, size 0xdc */
void NewCircuit(enum_kNumCircuitTypes circuitType)
{
    anon_checksum_active_4c *circuit; /* r29 */
    circuit=&(*(*gPlayers[gCircuitPlayer].save)->datah)->circuit;memset(circuit,0,sizeof(*circuit));((void(*)(tPS_File**,void*,int))PS_WriteFile)((*gPlayers[gCircuitPlayer].save)->fileh,circuit,sizeof(*circuit));circuit->type=circuitType;circuit->seed=*(unsigned int*)&gSeconds&0xfffff;circuit->active=1;circuit->totalTime=0.0f;if(gOptions[31])circuit->death=1;circuit->difficulty=gCircuitDiff[circuitType];
}

/* 0x8000a4d4, size 0xae4 */
void ContinueCircuit(void)
{
    int i; /* r6 */
    int j; /* r0 */
    int k; /* r0 */
    int n; /* r0 */
    int bit; /* r26 */
    int b; /* r27 */
    int sortList[6]; /* stack */
    int numDrones; /* r22 */
    int numCircuitTracks; /* r29 */
    anon_checksum_active_4c *circuit; /* r28 */
    int orient; /* r6 */
    static int skills[6][12]={0,0,0,0,0,1,1,1,0,1,2,2,1,1,1,1,1,2,2,2,1,0,0,3,2,2,2,3,3,3,3,4,2,4,3,1,3,3,4,4,4,0,0,3,3,2,5,4,4,5,5,2,2,5,5,5,5,3,4,0,5,4,3,5,5,4,4,0,4,5,1,5};
circuit=&(*(*gPlayers[gCircuitPlayer].save)->datah)->circuit;gCircuitSeed=circuit->seed;numCircuitTracks=gNumCircuitTracks[circuit->type];if(circuit->type==3)gCircuitSize=numCircuitTracks*4;else gCircuitSize=numCircuitTracks*2;gCircuitTrack=circuit->track<gCircuitSize-1?circuit->track:gCircuitSize-1;
for(i=0;i<numCircuitTracks;i++)gCircuitNumTrackRaces[i]=0;
numDrones=6-gNumPlayers;for(i=0;i<numDrones;i++){again:gCircuitCars[i+gNumPlayers].name=_CircuitRandom(57);for(j=0;j<i;j++)if(gCircuitCars[i+gNumPlayers].name==gCircuitCars[j+gNumPlayers].name)goto again;}
for(i=0;i<numDrones;i++)sortList[i]=i;
for(i=0;i<numDrones;i++){do{j=_CircuitRandom(numDrones-1);}while(j==i);sortList[i]^=sortList[j];sortList[j]^=sortList[i];sortList[i]^=sortList[j];}
for(i=0;i<numDrones;i++){n=sortList[i];sortList[i]=n;for(j=0;j<12;j++)gCircuitCars[gNumPlayers+sortList[i]].skill[j]=skills[i][j];}
for(i=0;i<12;i++){do{j=_CircuitRandom(11);}while(i==j);for(k=0;k<numDrones;k++){gCircuitCars[sortList[k]+gNumPlayers].skill[i]^=gCircuitCars[sortList[k]+gNumPlayers].skill[j];gCircuitCars[sortList[k]+gNumPlayers].skill[j]^=gCircuitCars[sortList[k]+gNumPlayers].skill[i];gCircuitCars[sortList[k]+gNumPlayers].skill[i]^=gCircuitCars[sortList[k]+gNumPlayers].skill[j];}}
for(i=0;i<=gCircuitTrack;i++){
if(circuit->type==3){gCircuitTracks[i].trackNum=_CircuitRandom(numCircuitTracks-1);gCircuitTracks[i].backward=_CircuitRandom(1);gCircuitTracks[i].mirror=_CircuitRandom(1);
if(i){k=0;while((gCircuitTracks[i].trackNum==gCircuitTracks[i-1].trackNum&&k<numCircuitTracks)||gCircuitNumTrackRaces[gCircuitTracks[i].trackNum]==4){gCircuitTracks[i].trackNum++;if(gCircuitTracks[i].trackNum>=numCircuitTracks)gCircuitTracks[i].trackNum=0;k++;}
do{for(j=0;j<i;j++)if(gCircuitTracks[i].trackNum==gCircuitTracks[j].trackNum&&gCircuitTracks[i].backward==gCircuitTracks[j].backward&&gCircuitTracks[i].mirror==gCircuitTracks[j].mirror){orient=(gCircuitTracks[i].backward|(gCircuitTracks[i].mirror<<1))+1;orient%=4;gCircuitTracks[i].backward=orient&1;gCircuitTracks[i].mirror=(orient>>1)&1;break;}}while(j!=i);}
gCircuitNumTrackRaces[gCircuitTracks[i].trackNum]++;}else{gCircuitTracks[i].trackNum=i%numCircuitTracks;gCircuitTracks[i].backward=i>=numCircuitTracks;}
n=_CircuitRandom(gCircuitFog[circuit->type]);gCircuitTracks[i].fog=n<4?0:n<7?1:n<9?2:3;gCircuitTracks[i].wind=_CircuitRandom(gCircuitWind[circuit->type]);}
n=6-(gNumPlayers==1?0:2);
for(i=0;i<n;i++){
gCircuitCars[i].totalPoints=circuit->temp?gTempCircuitPoints[gPlayers[gCircuitPlayer].controller][i]:0;
for(j=0;j<circuit->track;j++){if(circuit->temp&&j<gTempCircuitFirstTrack[gPlayers[gCircuitPlayer].controller]){gCircuitCars[i].places[j]=6;gCircuitCars[i].points[j]=0;}else{
gCircuitCars[i].places[j]=0;bit=j*3;for(b=0;b<3;b++,bit++)gCircuitCars[i].places[j]|=((circuit->ranks[i][bit>>3]>>(bit&7))&1)<<b;
gCircuitCars[i].points[j]=gCircuitPoints[gCircuitCars[i].places[j]];gCircuitCars[i].totalPoints+=gCircuitCars[i].points[j];}}
for(;j<gCircuitSize;j++){gCircuitCars[i].places[j]=6;gCircuitCars[i].points[j]=0;}}
for(;i<6;i++){gCircuitCars[i].totalPoints=0;for(j=0;j<gCircuitSize;j++){gCircuitCars[i].places[j]=6;gCircuitCars[i].points[j]=0;}}
for(i=0;i<6;i++)sortList[i]=i;
for(i=0;i<5;i++)for(j=i+1;j<6;j++)if(gCircuitCars[sortList[i]].totalPoints<gCircuitCars[sortList[j]].totalPoints){sortList[i]^=sortList[j];sortList[j]^=sortList[i];sortList[i]^=sortList[j];}
for(i=0;i<6;i++)gCircuitCars[sortList[i]].totalPlace=i;
}

/* 0x8000afb8, size 0x398 */
void UpdateCircuit(unsigned char *ranks, float time)
{
    int i; /* r0 */
    int n; /* r5 */
    int bit; /* r10 */
    int b; /* r0 */
    int place; /* r5 */
    int spread; /* r0 */
    int diff; /* r6 */
    int first; /* r30 */
    int second; /* r29 */
    anon_checksum_active_4c *circuit; /* r28 */
    circuit=&(*(*gPlayers[gCircuitPlayer].save)->datah)->circuit;n=6-(gNumPlayers==1?0:2);if(gCircuitPlayer){ranks[0]^=ranks[1];ranks[1]^=ranks[0];ranks[0]^=ranks[1];}
for(i=0;i<n;i++,ranks++){bit=gCircuitTrack*3;for(b=0;b<3;b++,bit++){circuit->ranks[i][bit>>3]=(circuit->ranks[i][bit>>3]&~(1<<(bit&7)))|((*ranks&1)<<(bit&7));*ranks>>=1;}}
circuit->track++;circuit->totalTime+=time;ContinueCircuit();gCircuitTrack--;if(circuit->track>=gCircuitSize)RecordCircuit();
if(circuit->type==3){if(gNumPlayers==1)place=gCircuitCars[0].totalPlace;else place=(gCircuitCars[0].totalPlace<gCircuitCars[1].totalPlace?gCircuitCars[0].totalPlace:gCircuitCars[1].totalPlace);
for(i=0;i<6;i++){if(gCircuitCars[i].totalPlace==0)first=i;else if(gCircuitCars[i].totalPlace==1)second=i;}
diff=circuit->difficulty;spread=gCircuitCars[first].totalPoints-gCircuitCars[second].totalPoints;
if((place==0&&diff<5&&spread>=20)||(place==0&&diff<4&&spread>=6)||(place<2&&diff<3)||(place<3&&diff<2)||(place<4&&diff<1))circuit->difficulty=diff+1;}
SavePlayerCircuit(gPlayers[gCircuitPlayer].save);
}

/* 0x8000b350, size 0xec */
void GetCircuitCode(unsigned char *code)
{
    int i; /* r30 */
    anon_checksum_active_4c *circuit; /* r0 */
    circuit=&(*(*gPlayers[gCircuitPlayer].save)->datah)->circuit;ContinueCircuit();gCircuitTrack--;NewPassword(86,14);StorePasswordElement(circuit->type,3);StorePasswordElement(circuit->death,1);StorePasswordElement(circuit->track,5);StorePasswordElement(circuit->seed,20);StorePasswordElement(circuit->difficulty,3);for(i=0;i<6;i++)StorePasswordElement(gCircuitCars[i].totalPoints,9);GetPasswordString((char*)code);
}

/* 0x8000b43c, size 0x338 */
int SetCircuitCode(unsigned char *code)
{
    unsigned int type; /* stack */
    unsigned int death; /* stack */
    unsigned int track; /* stack */
    unsigned int seed; /* stack */
    unsigned int diff; /* stack */
    unsigned int points[6]; /* stack */
    int i; /* r29 */
    int j; /* r10 */
    int sortList[6]; /* stack */
    int givenPoints; /* r11 */
    int totalPoints; /* r8 */
    anon_checksum_active_4c *circuit; /* r0 */
    circuit=&(*(*gPlayers[gCircuitPlayer].save)->datah)->circuit;NewPassword(86,14);if(!SetPasswordString((char*)code))return 0;GetPasswordElement(&type,3);GetPasswordElement(&death,1);GetPasswordElement(&track,5);GetPasswordElement(&seed,20);GetPasswordElement(&diff,3);for(i=0;i<6;i++)GetPasswordElement(&points[i],9);
if(track==0||track>=gNumCircuitTracks[type]*4)return 0;
for(i=0;i<6;i++)sortList[i]=i;for(i=0;i<5;i++)for(j=i+1;j<6;j++)if(points[sortList[i]]<points[sortList[j]]){sortList[i]^=sortList[j];sortList[j]^=sortList[i];sortList[i]^=sortList[j];}
if(points[sortList[0]]>track*gCircuitPoints[0])return 0;
givenPoints=0;for(i=0;i<6;i++)givenPoints+=gCircuitPoints[i];totalPoints=0;for(i=0;i<6;i++)totalPoints+=points[sortList[i]];if(totalPoints!=givenPoints*track)return 0;
NewCircuit(type);circuit->temp=1;circuit->death=death;circuit->track=track;gTempCircuitFirstTrack[gPlayers[gCircuitPlayer].controller]=track;circuit->seed=seed;circuit->difficulty=diff;for(i=0;i<6;i++)gTempCircuitPoints[gPlayers[gCircuitPlayer].controller][i]=points[i];ContinueCircuit();return 1;
}

/* 0x8000b774, size 0x48 */
static unsigned short _CircuitRandom(unsigned short max)
{
    return rand_r(&gCircuitSeed)%(max+1u);
}

