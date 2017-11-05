#include <scil-dict.h>

#include <stdlib.h>
#include <string.h>

static char* scilU_dict_strdup(const char* s) /* make a duplicate of s */
{
    char* p = (char*) malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL) { strcpy(p, s); }
    return p;
}

static void free_element(scilU_dict_element_t* element)
{
    free(element->key);
    free(element->value);
    free(element);
}

/* hash: form hash value for string s */
unsigned scilU_dict_hash(const scilU_dict_t* dict, const char* s)
{
    unsigned hashval = 0;
    for (; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % dict->size;
}

scilU_dict_t * scilU_dict_create(int size){
  scilU_dict_t * dict = (scilU_dict_t *) malloc(sizeof(scilU_dict_t));
  dict->size = size;
  dict->elem = malloc(size*sizeof(scilU_dict_element_t*));
  memset(dict->elem, 0, size*sizeof(scilU_dict_element_t*));
  return dict;
}


void scilU_dict_destroy(scilU_dict_t* dict)
{
    for(unsigned i = 0; i < dict->size; ++i)
    {
        if (dict->elem[i] == NULL)
            continue;

        for (scilU_dict_element_t* element = dict->elem[i]; element != NULL;)
        {
            scilU_dict_element_t* next = element->next;
            free_element(element);
            element = next;
        }
    }
    free(dict->elem);
}

/* lookup: look for s in dict */
scilU_dict_element_t* scilU_dict_get(const scilU_dict_t* dict, const char* s)
{
  unsigned i = scilU_dict_hash(dict, s);
    for (scilU_dict_element_t* element = dict->elem[i]; element != NULL; element = element->next) {
        if (strcmp(s, element->key) == 0)
            return element; /* found */
    }
    return NULL; /* not found */
}

int scilU_dict_contains(const scilU_dict_t* dict, const char* key)
{
    return scilU_dict_get(dict, key) != NULL;
}

/* install: put (key, value) in scilU_dict */
scilU_dict_element_t* scilU_dict_put(scilU_dict_t* dict, const char* key, const char* value)
{
    unsigned hashval;
    scilU_dict_element_t* element = scilU_dict_get(dict, key);
    if (element == NULL) { /* not found */
        element = (scilU_dict_element_t*)malloc(sizeof(*element));
        if (element == NULL || (element->key = scilU_dict_strdup(key)) == NULL)
          return NULL;
        hashval = scilU_dict_hash(dict, key);
        element->next = dict->elem[hashval];
        dict->elem[hashval] = element;
    } else /* already there */
        free((void*) element->value); /*free previous value */
    if ((element->value = scilU_dict_strdup(value)) == NULL)
       return NULL;
    return element;
}

void scilU_dict_remove(scilU_dict_t* dict, const char* key)
{
    unsigned hashval = scilU_dict_hash(dict, key);
    scilU_dict_element_t* element = dict->elem[hashval];

    if (element == NULL)
        return;

    // First element
    if (strcmp(key, element->key) == 0){
        dict->elem[hashval] = element->next;
        free_element(element);
        return;
    }

    // The rest...
    scilU_dict_element_t* previous = element;
    element = element->next;

    for (; element != NULL; element = element->next)
    {
        if (strcmp(key, element->key) != 0) {
            previous = element;
            continue;
        }

        previous->next = element->next;
        free_element(element);
        return;
    }

    return;
}
