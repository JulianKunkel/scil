#ifndef SCIL_DICT_H_
#define SCIL_DICT_H_

typedef struct scil_dict_element scil_dict_element_t;

struct scil_dict_element { /* table entry: */
    scil_dict_element_t* next; /* next entry in chain */
    char* name; /* defined name */
    char* defn; /* replacement text */
};

typedef scil_dict_element_t** scil_dict_t;

scil_dict_t scil_dict_create();

void scil_dict_destroy(scil_dict_t dict);

unsigned scil_dict_hash(const char* s);

scil_dict_element_t* scil_dict_get(const scil_dict_t dict, const char* s);

int scil_dict_contains(const scil_dict_t dict, const char* name);

scil_dict_element_t* scil_dict_put(const scil_dict_t dict, const char* name, const char* defn);

scil_dict_element_t* scil_dict_remove(const scil_dict_t dict, const char* name);

#endif /* SCIL_DICT_H_ */
