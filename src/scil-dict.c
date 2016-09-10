#include <scil-dict.h>

#include <stdlib.h>
#include <string.h>

#define SCIL_DICT_SIZE 100

static char* scil_dict_strdup(const char* const s) /* make a duplicate of s */
{
    char* p = (char*)malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL) { strcpy(p, s); }
    return p;
}

/* hash: form hash value for string s */
unsigned scil_dict_hash(const char* s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % SCIL_DICT_SIZE;
}

scil_dict_t scil_dict_create()
{

}

void scil_dict_destroy(scil_dict_t dict)
{

}

/* lookup: look for s in dict */
scil_dict_element_t* scil_dict_get(const scil_dict_t dict,
                                   const char* const s)
{
    for (scil_dict_element_t* element = &dict[scil_dict_hash(s)]; element != NULL; element = element->next)
        if (strcmp(s, element->name) == 0)
          return element; /* found */
    return NULL; /* not found */
}

/* install: put (name, defn) in scil_dict */
scil_dict_element_t* scil_dict_put(const scil_dict_t dict, const char* const name, const char* const defn)
{
    unsigned hashval;
    scil_dict_element_t* element = scil_dict_get(dict, name);
    if (element == NULL) { /* not found */
        element = (scil_dict_t)malloc(sizeof(*element));
        if (element == NULL || (element->name = scil_dict_strdup(name)) == NULL)
          return NULL;
        hashval = scil_dict_hash(name);
        element->next = &dict[hashval];
        dict[hashval] = *element;
    } else /* already there */
        free((void*) element->defn); /*free previous defn */
    if ((element->defn = scil_dict_strdup(defn)) == NULL)
       return NULL;
    return element;
}
