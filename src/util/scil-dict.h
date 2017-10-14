#ifndef SCIL_DICT_H
#define SCIL_DICT_H


typedef struct scilI_dict_element scilI_dict_element_t;

struct scilI_dict_element { /* table entry: */
    char* key; /* defined key */
    char* value; /* replacement text */
    scilI_dict_element_t * next;
};


typedef struct {
  unsigned size; // the initialized size of the dictionary
  scilI_dict_element_t ** elem;
} scilI_dict_t;



scilI_dict_t * scilI_dict_create(int size);

void scilI_dict_destroy(scilI_dict_t* dict);

unsigned scilI_dict_hash(const scilI_dict_t* dict, const char* s);

scilI_dict_element_t* scilI_dict_get(const scilI_dict_t* dict, const char* s);

int scilI_dict_contains(const scilI_dict_t* dict, const char* key);

scilI_dict_element_t* scilI_dict_put(scilI_dict_t* dict, const char* key, const char* value);

void scilI_dict_remove(scilI_dict_t* dict, const char* key);

#endif // SCIL_DICT_H
