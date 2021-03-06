#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_
#define includeDel
typedef struct HashTable HashTable;

typedef struct entry{
  unsigned long val;
#ifdef includeDel
  volatile unsigned long isDeleted;
#endif
}entry;


// free hash table when done
double freeAll(HashTable* head, int last, int verbose);

// see if entry is in the table
int checkTableQuery(HashTable* head, unsigned long val);

// initialize a new main hashtable
HashTable* initTable(HashTable* head, int InitSize, int HashAttempts, int numThreads, unsigned int * seeds, double lines);


// return 1 if inserted, 0 if already there
int insertTable(HashTable* head,  int start, entry* ent, int tid);

int getStart(HashTable* head);


int deleteVal(HashTable* head, unsigned long val, int tid);

// check that library is compiled correctly for system currently being run on
int checkCompiledCorrectly(void);

////////////////////////////////////////////////////////////////
// used only for testing and timing

extern const char* tablename;
extern const char* shortname;

#endif
