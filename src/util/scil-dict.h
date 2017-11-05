#ifndef SCIL_DICT_H
#define SCIL_DICT_H


typedef struct scilU_dict_element scilU_dict_element_t;

struct scilU_dict_element { /* table entry: */
    char* key; /* defined key */
    char* value; /* replacement text */
    scilU_dict_element_t * next;
};


typedef struct {
  unsigned size; // the initialized size of the dictionary
  scilU_dict_element_t ** elem;
} scilU_dict_t;



scilU_dict_t * scilU_dict_create(int size);

void scilU_dict_destroy(scilU_dict_t* dict);

unsigned scilU_dict_hash(const scilU_dict_t* dict, const char* s);

scilU_dict_element_t* scilU_dict_get(const scilU_dict_t* dict, const char* s);

int scilU_dict_contains(const scilU_dict_t* dict, const char* key);

scilU_dict_element_t* scilU_dict_put(scilU_dict_t* dict, const char* key, const char* value);

void scilU_dict_remove(scilU_dict_t* dict, const char* key);

#endif // SCIL_DICT_H
