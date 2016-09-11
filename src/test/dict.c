#include <scil-dict.h>

int main(void)
{
    scil_dict_t dict = scil_dict_create();

    scil_dict_put(dict, "index1", "value1");
    scil_dict_destroy(& dict);

    return 0;
}
