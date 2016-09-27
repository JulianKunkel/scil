#ifndef SCIL_DICT_H
#define SCIL_DICT_H

typedef struct scilI_dict_element scilI_dict_element_t;

struct scilI_dict_element { /* table entry: */
    scilI_dict_element_t* next; /* next entry in chain */
    char* key; /* defined key */
    char* value; /* replacement text */
};

typedef scilI_dict_element_t* scilI_dict_t;

scilI_dict_t* scilI_dict_create();

void scilI_dict_destroy(scilI_dict_t* dict);

unsigned scilI_dict_hash(const char* s);

scilI_dict_element_t* scilI_dict_get(const scilI_dict_t* dict, const char* s);

int scilI_dict_contains(const scilI_dict_t* dict, const char* key);

scilI_dict_element_t* scilI_dict_put(scilI_dict_t* dict, const char* key, const char* value);

void scilI_dict_remove(scilI_dict_t* dict, const char* key);

#endif // SCIL_DICT_H
