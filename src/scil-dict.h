#ifndef SCIL_DICT_H_
#define SCIL_DICT_H_

typedef struct scil_dict_list { /* table entry: */
    struct scil_dict_list* next; /* next entry in chain */
    char* name; /* defined name */
    char* defn; /* replacement text */
} scil_dict_list_t;

unsigned scil_dict_hash(char* s);

char* scil_dict_strdup(char* s);

scil_dict_list_t* scil_dict_get(char* s);

scil_dict_list_t* scil_dict_put(char* name, char* defn);

#endif /* SCIL_DICT_H_ */
