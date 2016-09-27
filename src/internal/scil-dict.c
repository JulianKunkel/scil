#include <scil-dict.h>

#include <stdlib.h>
#include <string.h>

#define SCIL_DICT_SIZE 100

static char* scilI_dict_strdup(const char* s) /* make a duplicate of s */
{
    char* p = (char*)malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL) { strcpy(p, s); }
    return p;
}

static void free_element(scilI_dict_element_t* element)
{
    free(element->key);
    free(element->value);
    free(element);
}

/* hash: form hash value for string s */
unsigned scilI_dict_hash(const char* s)
{
    unsigned hashval = 0;
    for (; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % SCIL_DICT_SIZE;
}

scilI_dict_t* scilI_dict_create()
{
    return (scilI_dict_t*) calloc(SCIL_DICT_SIZE, sizeof(scilI_dict_element_t*));
}

void scilI_dict_destroy(scilI_dict_t* dict)
{
    for(unsigned i = 0; i < SCIL_DICT_SIZE; ++i)
    {
        if (dict[i] == NULL)
            continue;

        for (scilI_dict_element_t* element = dict[i]; element != NULL;)
        {
            scilI_dict_element_t* next = element->next;
            free_element(element);
            element = next;
        }
    }
    free(dict);
}

/* lookup: look for s in dict */
scilI_dict_element_t* scilI_dict_get(const scilI_dict_t* dict, const char* s)
{
    for (scilI_dict_element_t* element = dict[scilI_dict_hash(s)]; element != NULL; element = element->next) {
        if (strcmp(s, element->key) == 0)
            return element; /* found */
    }
    return NULL; /* not found */
}

int scilI_dict_contains(const scilI_dict_t* dict, const char* key)
{
    return scilI_dict_get(dict, key) != NULL;
}

/* install: put (key, value) in scilI_dict */
scilI_dict_element_t* scilI_dict_put(scilI_dict_t* dict, const char* key, const char* value)
{
    unsigned hashval;
    scilI_dict_element_t* element = scilI_dict_get(dict, key);
    if (element == NULL) { /* not found */
        element = (scilI_dict_element_t*)malloc(sizeof(*element));
        if (element == NULL || (element->key = scilI_dict_strdup(key)) == NULL)
          return NULL;
        hashval = scilI_dict_hash(key);
        element->next = dict[hashval];
        dict[hashval] = element;
    } else /* already there */
        free((void*) element->value); /*free previous value */
    if ((element->value = scilI_dict_strdup(value)) == NULL)
       return NULL;
    return element;
}

void scilI_dict_remove(scilI_dict_t* dict, const char* key)
{
    unsigned hashval = scilI_dict_hash(key);
    scilI_dict_element_t* element = dict[hashval];

    if (element == NULL)
        return;

    // First element
    if (strcmp(key, element->key) == 0){
        dict[hashval] = element->next;
        free_element(element);
        return;
    }

    // The rest...
    scilI_dict_element_t* previous = element;
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
