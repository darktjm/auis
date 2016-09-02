/*
 * 20 April 1987 - Miles Bader
 * Last edit by Miles Bader (bader) on Thu,  4 Jun 1987 -  7:41pm
 */

#define HASHSIZE    103

#define	HASHENTRY   struct hashEntry
struct	hashEntry{
    char	*name;
    HASHENTRY	*next;
    VOID	*value;
};

#define HASHTABLE struct hashTable
struct hashTable{
    VOID    (*deleteFunc)();
    HASHENTRY	*table[HASHSIZE];
};

VOID	*hash_Find(),hash_Install();
HASHTABLE   *hash_New();
