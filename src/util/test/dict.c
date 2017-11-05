#include <scil-dict.h>

#include <stdio.h>
#include <string.h>

static unsigned error_bit_mask = 0;

static void set_next_bit(int bitflag)
{
    static unsigned char index = 0;

    if (bitflag)
        error_bit_mask += bitflag << index;

    ++index;
}

static unsigned check_error_bit(unsigned char index)
{
    return error_bit_mask & 1 << index;
}

static void print_errors()
{
    unsigned char i = 0;

    if (error_bit_mask == 0) {
        printf("No Errors.\n");
        return;
    }

    if (check_error_bit(i++)) { printf("Error in scilU_dict_put or scilU_dict_contains for \"key1\" and \"value1\".\n"); }
    if (check_error_bit(i++)) { printf("Error in scilU_dict_put or scilU_dict_contains for \"key2\" and \"value2\".\n"); }
    if (check_error_bit(i++)) { printf("Error in scilU_dict_put or scilU_dict_contains for \"key3\" and \"value3\".\n"); }

    if (check_error_bit(i++)) { printf("Error in scilU_dict_get for \"key1\" and \"value1\".\n"); }
    if (check_error_bit(i++)) { printf("Error in scilU_dict_get for \"key2\" and \"value2\".\n"); }
    if (check_error_bit(i++)) { printf("Error in scilU_dict_get for \"key3\" and \"value3\".\n"); }

    if (check_error_bit(i++)) { printf("Error in scilU_dict_remove for \"key1\" and \"value1\".\n"); }
    if (check_error_bit(i++)) { printf("Error in scilU_dict_remove for \"key2\" and \"value2\".\n"); }
    if (check_error_bit(i++)) { printf("Error in scilU_dict_remove for \"key3\" and \"value3\".\n"); }
}

int main(void)
{
    scilU_dict_t* dict = scilU_dict_create(10);

    // Inserting entries
    scilU_dict_put(dict, "key1", "value1");
    scilU_dict_put(dict, "key2", "value2");
    scilU_dict_put(dict, "key3", "value3");

    // Containment of element
    set_next_bit( !scilU_dict_contains(dict, "key1") );
    set_next_bit( !scilU_dict_contains(dict, "key2") );
    set_next_bit( !scilU_dict_contains(dict, "key3") );

    // Retrieving element
    set_next_bit( strcmp(scilU_dict_get(dict, "key1")->value, "value1") != 0 );
    set_next_bit( strcmp(scilU_dict_get(dict, "key2")->value, "value2") != 0 );
    set_next_bit( strcmp(scilU_dict_get(dict, "key3")->value, "value3") != 0 );

    // Removing of element
    scilU_dict_remove(dict, "key1");
    scilU_dict_remove(dict, "key2");
    scilU_dict_remove(dict, "key3");

    // Non-Containment of element
    set_next_bit( scilU_dict_contains(dict, "key1") );
    set_next_bit( scilU_dict_contains(dict, "key2") );
    set_next_bit( scilU_dict_contains(dict, "key3") );

    scilU_dict_destroy(dict);

    print_errors();

    return error_bit_mask;
}
