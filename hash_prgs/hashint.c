#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

//check if fail cas, volatile types

//1048072+2095328+4076616+3767649+5883524+4784952+7124525+3552408+1197798+7432
#define ssize 32 //string size of entry/vector size
#define max_tables 64 //max tables to create
//#define vsize 15 //amount of times you want to try and hash
//#define initSize (1) //starting table size
//#define runs 1<<4
#define notIn 0 
#define in -1
#define unk -2
#define max_threads 32 //number of threads to test with
#define table_bound (1<<20)
//#define expec 100

int initSize=0;
int runs=0;
int num_threads=0;
int vsize=0;
int barrier=0;
pthread_mutex_t cm;
int failed=0;
struct h_head* global=NULL;


//items to be hashed
typedef struct int_ent{
  unsigned long val;
}int_ent;

//a table
typedef struct h_table{
  //  struct h_table* next; //next table
  int_ent** s_table; //rows (table itself)
  int t_size; //size
}h_table;

//return value for a match, can look up item in slot of table
typedef struct ret_val{
  h_table* ht;
  int slot;
  int index;
  int snum;
}ret_val;

//head of cache
typedef struct h_head{
  h_table** tt; //array of tables
  int cur; //current max index (max exclusive)
}h_head;


typedef struct hashSeeds{
  unsigned long rand1;
  unsigned long rand2;
}hashSeeds;

int checkTable(int_ent* ent, unsigned int* seeds, int start);


//string hashing function (just universal vector hash)

uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed)
{
  uint32_t h = seed;
  if (len > 3) {
    const uint32_t* key_x4 = (const uint32_t*) key;
    size_t i = len >> 2;
    do {
      uint32_t k = *key_x4++;
      k *= 0xcc9e2d51;
      k = (k << 15) | (k >> 17);
      k *= 0x1b873593;
      h ^= k;
      h = (h << 13) | (h >> 19);
      h = h * 5 + 0xe6546b64;
    } while (--i);
    key = (const uint8_t*) key_x4;
  }
  if (len & 3) {
    size_t i = len & 3;
    uint32_t k = 0;
    key = &key[i - 1];
    do {
      k <<= 8;
      k |= *key--;
    } while (--i);
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
  }
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}







int lookupQuery(int_ent** s_table,int_ent*ent, unsigned int seeds, int slots){
  unsigned int s=murmur3_32(&ent->val, 8, seeds)%slots;
  if(s_table[s]==NULL){
    return notIn;
  }
  else if(ent->val==s_table[s]->val){
    return s;
  }
  return unk;
}


int checkTableQuery(int_ent* ent, unsigned int* seeds){
  
  h_table* ht=NULL;
  for(int j=0;j<global->cur;j++){
    ht=global->tt[j];
    for(int i =0;i<vsize;i++){
      int res=lookupQuery(ht->s_table, ent, seeds[i], ht->t_size);
      if(res==unk){ //unkown if in our not
	continue;
      }
      if(res==notIn){ //is in
	return 0;
      }
      //not in (we got null so it would have been there)
      
      return 1;
    }
  }
  return 0;
}



//create a new table of size n
h_table* createTable(int n_size){
  h_table* ht=(h_table*)malloc(sizeof(h_table));
  ht->t_size=n_size;
  ht->s_table=(int_ent**)malloc(sizeof(int_ent*)*(ht->t_size));
  //  ht->next=NULL;
  return ht;
}

void freeTable(h_table* ht){
  free(ht);
}

//add new table to list of tables
int addDrop(int_ent* ent, unsigned int* seeds,h_table* toadd, int tt_size){
  h_table* expected=NULL;
  int res = __atomic_compare_exchange(&global->tt[tt_size] ,&expected, &toadd, 1, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
  if(res){
    int newSize=tt_size+1;
    __atomic_compare_exchange(&global->cur,
			      &tt_size,
			      &newSize,
			      1,__ATOMIC_RELAXED, __ATOMIC_RELAXED);
    checkTable(ent, seeds,1);
  }
  else{
    freeTable(toadd);
    int newSize=tt_size+1;
    __atomic_compare_exchange(&global->cur,
			      &tt_size,
			      &newSize,
			      1, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    checkTable(ent, seeds, 1);
  }
  return 0;
}

//check if entry for a given hashing vector is in a table
int lookup(int_ent** s_table,int_ent* ent, unsigned int seeds, int slots){

  //  unsigned int s=hashInt(ent->val, slots, seeds);
  //  unsigned int s=murmur_hash_64(ent->val, 8, seeds.rand1, slots);

    unsigned int s= murmur3_32(&ent->val, 8, seeds)%slots;
  if(s_table[s]==NULL){
    return s;
  }
  else if(s_table[s]->val==ent->val){
    return in;
  }
  return unk;
}

//check if an item is found in the table, if not make new table and store it
/*
ret_val checkTable(int_ent* ent, unsigned int* seeds, int start, int seed_num){
  int startCur=global->cur;
  h_table* ht=NULL;
  for(int j=start;j<global->cur;j++){
    ht=global->tt[j];
    for(int i =seed_num;i<vsize;i++){
      int res=lookup(ht->s_table, ent, seeds[i], ht->t_size);
      if(res==unk){ //unkown if in our not
	continue;
      }
      if(res==in){ //is in
	ret_val ret={ .ht=NULL, .slot=0, .index=j, .snum=i};
	return ret;
      }
      //not in (we got null so it would have been there)
      ret_val ret={ .ht=ht, .slot=res, .index=j, .snum=i};
      return ret;
    }
    startCur=global->cur;
    //    ht=ht->next;
  }

  
  
  h_table* new_table=createTable(global->tt[startCur-1]->t_size<<1);
  addDrop(ent, seeds, new_table, startCur);
}
*/

int checkTable(int_ent* ent, unsigned int* seeds, int start){
  int startCur=global->cur;
  h_table* ht=NULL;
  for(int j=start;j<global->cur;j++){
    ht=global->tt[j];
    for(int i =0;i<vsize;i++){
      int res=lookup(ht->s_table, ent, seeds[i], ht->t_size);
      if(res==unk){ //unkown if in our not
	continue;
      }
      if(res==in){ //is in
	return 0;
      }

      int_ent* expected=NULL;
      int cmp= __atomic_compare_exchange(&ht->s_table[res],
					&expected,
					&ent,
					1, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
      if(cmp){
	return 0;
      }
      else{
	failed++;
	if(ht->s_table[res]->val==ent->val){
	  return 0;
	}
      }
    }
    startCur=global->cur;
    //    ht=ht->next;
  }

  //create new table
  /*  int new_size=0;
  if(global->tt[startCur-1]->t_size>table_bound&&startCur%2){
    new_size=global->tt[startCur-1]->t_size;
  }
  else{
    new_size=global->tt[startCur-1]->t_size<<1;
  }

  h_table* new_table=createTable(new_size);*/
  h_table* new_table=createTable(global->tt[startCur-1]->t_size<<1);
  addDrop(ent, seeds, new_table, startCur);
}


//add item using ret_val struct info
int tryAdd(int_ent* ent, unsigned int* seeds, int start, int seed_num){
  ret_val loc;//=checkTable(ent, seeds, start);//, seed_num);
  int_ent* expected=NULL;
  if(loc.ht){
    int res = __atomic_compare_exchange(&loc.ht->s_table[loc.slot],
					&expected,
					&ent,
					1, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    if(res){
      return 0;
    }
    else{
      tryAdd(ent, seeds, loc.index, loc.snum-(loc.snum!=0));
    }
  }
  return 0;
}

//print table (smallest to largest, also computes total items)

void printTables(unsigned int* seeds){
  FILE* fp = stdout;//fopen("output.txt","wa");
  int* items=(int*)malloc(sizeof(int)*global->cur);
  h_table* ht=NULL;
  int count=0;
  for(int j=0;j<global->cur;j++){
    ht=global->tt[j];
    items[j]=0;
    //     fprintf(fp, "Table Size: %d\n", ht->t_size);
    for(int i =0;i<ht->t_size;i++){
      if(ht->s_table[i]!=NULL){
	//	       	fprintf(fp, "%d: %lu\n",i,ht->s_table[i]->val);
	//	fprintf(fp, "%lu", ht->s_table[i]->val);
	free(ht->s_table[i]);
	items[j]++;
	count++;
      }
      else{
	//		fprintf(fp,"%d: NULL\n", i);
      }
    }
    //       fprintf(fp,"\n\n\n");
	//    ht=ht->next;
  }
  
  //    fprintf(fp,"------------------start----------------\n");
  //      if(count!=expec){
  //      printf("fuck up\n");
  //    }
     fprintf(fp,"count=%d\n", count);
  for(int i =0;i<vsize;i++){
    printf("r1: %u\n", seeds[i]);
  }
  h_table* temp=NULL;
  fprintf(fp,"tables:\n");
  for(int j=0;j<global->cur;j++){
    temp=global->tt[j];
    fprintf(fp,"%d/%d\n", items[j],temp->t_size);
    free(temp);
  }
  fprintf(fp,"------------------end----------------\n");

}


void* run(void* argp){

  unsigned int* seeds=(unsigned int*)argp;
  //    pthread_mutex_lock(&cm);
    //    barrier++;
    //    pthread_mutex_unlock(&cm);
    //    printf("here, %d, %d\n", barrier, num_threads);
    //    while(barrier<num_threads){
      //    printf("here2, %d, %d\n", barrier, num_threads);    
    //    }

  for(int i =0;i<(runs);i++){
    int_ent* testAdd=(int_ent*)malloc(sizeof(int_ent));
    unsigned long temp=rand();
      testAdd->val=rand();
        testAdd->val=testAdd->val*temp;
    //    testAdd->val=rand()%expec;
    checkTable(testAdd, seeds, 0);
    
   
     
    
  }
}
int main(int argc, char** argv){


  if(argc!=5){
    printf("5 args\n");
    exit(0);
  }

  srand(time(NULL));
  initSize=atoi(argv[1]);
  runs=atoi(argv[2]);
  num_threads=atoi(argv[3]);
  vsize=atoi(argv[4]);
  if(num_threads>max_threads){
    num_threads=max_threads;
  }

  global=(h_head*)malloc(sizeof(h_head));
  global->tt=(h_table**)malloc(max_tables*sizeof(h_table*));
  global->cur=1;
  global->tt[0]=createTable(initSize);

  //  hashSeeds* seeds=(hashSeeds*)malloc(sizeof(hashSeeds)*vsize);
  unsigned int * seeds=(unsigned int*)malloc(sizeof(unsigned int)*vsize);
  for(int i =0;i<vsize;i++){
    seeds[i]=rand();

  }

  
  //creating num_threads threads to add items in parallel
  //see run function for more details
  int cores=sysconf(_SC_NPROCESSORS_ONLN);
  pthread_t threads[max_threads];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  cpu_set_t sets[max_threads];
  for(int i =0;i<num_threads;i++){
    CPU_ZERO(&sets[i]);
    CPU_SET(i, &sets[i]);
    threads[i]=pthread_self();
    pthread_setaffinity_np(threads[i], sizeof(cpu_set_t),&sets[i]);
    pthread_create(&threads[i], &attr,run,(void*)seeds);

  }
  for(int i =0;i<num_threads;i++){
    pthread_join(threads[i], NULL);
  }

  printf("failed = %d\n", failed);
   
  //           printTables(seeds);
	   //         free(global);
	 //       free(seeds);

  

}

