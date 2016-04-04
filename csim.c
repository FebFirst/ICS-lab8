/*
 *A lab finished by: Xiong Guo
 *Student ID: 5140379069
 */
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define ADDR_SIZE 64

typedef struct{
    int hits;
    int misses;
    int evictions;
}result_t;

typedef struct{
    int s;
    int E;
    int b;
}state_t;

typedef struct{
    short valid;
    unsigned long int tag;
    int useTime;
}line_t;

typedef struct{
    line_t *lines;
    
}set_t;

typedef struct{
    set_t *sets;
}cache_t;

unsigned long int getTag(unsigned long int addr, int s, int b)
{
	//printf("getTag!\n");
    return addr >> (s + b);
}

unsigned int getSet(unsigned long int addr, int s, int b)
{
	//printf("getSet!\n");
    return (addr << (ADDR_SIZE - s - b)) >> (ADDR_SIZE - s);
}
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}
cache_t initCache(int setNum, int lineNum)
{
	//printf("initCache!\n");
    cache_t cache;
    cache.sets = (set_t *)malloc(sizeof(set_t) * setNum);
    for (int i = 0; i < setNum; i ++){
        cache.sets[i].lines = (line_t *)malloc(sizeof(line_t) * lineNum);
        for(int j = 0; j < lineNum; j++){
            cache.sets[i].lines[j].valid = 0;
            cache.sets[i].lines[j].tag = 0;
            cache.sets[i].lines[j].useTime = 0;
        }
    }
    return cache;
}
void getPara(int argc, char *argv[], state_t *currState, char **traceFile)
{
	//printf("getPara!\n");
    char tmp;
    while ((tmp = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (tmp) {
            case 's':
                currState->s = atoi(optarg);
				//printf("%d\n",currState->s);
                break;
            case 'E':
                currState->E = atoi(optarg);
				//printf("%d\n",currState->E);
                break;
            case 'b':
                currState->b = atoi(optarg);
				//printf("%d\n",currState->b);
                break;
            case 't':
                *traceFile= optarg;
				//printf("%s\n",*traceFile);
                break;
            case 'v':
                break;
            case 'h':
                printUsage(argv);
                exit(0);
            default:
                printUsage(argv);
                exit(1);
            }
    }
}
/*int getLFU(set_t* set, int lineNum)
{
	//printf("getLFU!\n");
    int leastTime = set->lines[0].useTime;
    int LFU = 0;
    for(int i = 0; i < lineNum; i ++){
        if(leastTime > set->lines[i].useTime){
            leastTime = set->lines[i].useTime;
            LFU = i;
        }
    }
    return LFU;
}*/
int *getLRU(set_t* set, int lineNum)
{
	//printf("getLFU!\n");
	int *LM = (int*)malloc(sizeof(int)*2);
    int leastTime = set->lines[0].useTime;
	int mostTime = set->lines[0].useTime;
    int LRU = 0;
    for(int i = 0; i < lineNum; i ++){
        if(leastTime > set->lines[i].useTime){
            leastTime = set->lines[i].useTime;
            LRU = i;
        }
		if(mostTime < set->lines[i].useTime){
            mostTime = set->lines[i].useTime;
        }
    }
	LM[0] = LRU;
	LM[1] = mostTime;
    return LM;
}
int getFreeLine(set_t* set, int lineNum)
{
	//printf("getFreeLine!\n");
    for(int i = 0; i < lineNum; i++)
        if (!set->lines[i].valid)
            return i;
    return -1;
}
void cacheSimu(cache_t cache, state_t *state, result_t *res, unsigned long int addr)
{
	//printf("cacheSimu!\n");
    short full = 1;
    int lineNum = state->E;
    int tmpHits = res->hits;
    set_t *currSet = &cache.sets[getSet(addr, state->s, state->b)];
    unsigned long int tag = getTag(addr, state->s, state->b);
    for(int i = 0; i < lineNum; i ++){
        if(currSet->lines[i].valid && currSet->lines[i].tag == tag){
		printf("hit: ");
            res->hits ++;
            currSet->lines[i].useTime ++;
        }
        if(!currSet->lines[i].valid)
            full = 0;
    }
    if (tmpHits == res->hits)
        res->misses ++;
    else
        return;
   /* if(full){
        int LFU = getLFU(currSet, lineNum);
        res->evictions ++;
        currSet->lines[LFU].tag = tag;
        currSet->lines[LFU].useTime = 1;
    }*/
	int* LRU = getLRU(currSet, lineNum);
     if(full){
	printf("evic: ");
        res->evictions ++;
        currSet->lines[LRU[0]].tag = tag;
        currSet->lines[LRU[0]].useTime = LRU[1] + 1;
    }
    else{
	printf("miss: ");
        int freeLine = getFreeLine(currSet, lineNum);
        if (freeLine == -1){
            printf("getFreeLine error!\n");
            exit(0);
        }
        currSet->lines[freeLine].tag = tag;
        currSet->lines[freeLine].useTime = LRU[1] + 1;
        currSet->lines[freeLine].valid = 1;
    }
	free(LRU);
}
int main(int argc, char **argv)
{
    cache_t cache;
    state_t state;
    result_t res;
    char *traceFile = "0";
    getPara(argc, argv, &state, &traceFile);
    if (state.s == 0||state.E == 0||state.b == 0||traceFile == NULL){
        printf("Initial error!\n");
        printUsage(argv);
        exit(0);
    }
    res.hits = 0;
    res.misses = 0;
    res.evictions = 0;
    int setNum = 1 << state.s;
    int lineNum = state.E;
    cache = initCache(setNum, lineNum);
//printf("%s\n",traceFile);
    FILE *traceStream = fopen(traceFile, "r");
    char mode;
    unsigned long int addr;
    int size;
    if (traceStream != NULL){
//printf("beem beem boom!\n");
        while(fscanf(traceStream, " %c %lx,%d",&mode,&addr,&size) == 3){
			//printf("beem beem boom!\n");
		if (mode != 'I')
		printf("%c %lx,%d: ",mode,addr,size);
            switch (mode){
                    case 'I':
                    break;
                    case 'L': case'S':
			//printf("beem beem boom!\n");
                    cacheSimu(cache, &state,&res, addr);
                    break;
                    case 'M':
                    cacheSimu(cache, &state,&res, addr);
                    cacheSimu(cache, &state,&res, addr);
                    break;
                    default:
                    break;
            }
if (mode != 'I')
printf("\n");
}}
    printSummary(res.hits, res.misses, res.evictions);
    fclose(traceStream);
    return 0;
}
