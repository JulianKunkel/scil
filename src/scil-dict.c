#include <scil-dict.h>

#include <stdlib.h>
#include <string.h>

#define SCIL_DICT_SIZE 100
static scil_dict_list_t* dict[SCIL_DICT_SIZE]; /* pointer table */

/* hash: form hash value for string s */
unsigned scil_dict_hash(char* s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % SCIL_DICT_SIZE;
}

char* scil_dict_strdup(char* s) /* make a duplicate of s */
{
    char* p;
    p = (char*)malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL)
       strcpy(p, s);
    return p;
}

/* lookup: look for s in scil_dict */
scil_dict_list_t* scil_dict_get(char* s)
{
    scil_dict_list_t* np;
    for (np = dict[scil_dict_hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np; /* found */
    return NULL; /* not found */
}

/* install: put (name, defn) in scil_dict */
scil_dict_list_t* scil_dict_put(char* name, char* defn)
{
    scil_dict_list_t* np;
    unsigned hashval;
    if ((np = scil_dict_get(name)) == NULL) { /* not found */
        np = (scil_dict_list_t*)malloc(sizeof(*np));
        if (np == NULL || (np->name = scil_dict_strdup(name)) == NULL)
          return NULL;
        hashval = scil_dict_hash(name);
        np->next = dict[hashval];
        dict[hashval] = np;
    } else /* already there */
        free((void*) np->defn); /*free previous defn */
    if ((np->defn = scil_dict_strdup(defn)) == NULL)
       return NULL;
    return np;
}
