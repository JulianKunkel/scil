#include <stdlib.h>

#include <zlib.h>

#include <scil-util.h>


int main(int argc, char** argv){
    uLong count = 100;

    Bytef* buf = (Bytef*)SAFE_MALLOC(count * sizeof(Bytef));

    for(int i = 0; i < count; ++i){
        buf[i] = i % 10;
        printf("%d ", buf[i]);
    }
    printf("\n\n");

    uLongf c_size = compressBound(count);
    Bytef* c_buf = (char*)SAFE_MALLOC(c_size);

    int ret = compress(c_buf, &c_size, buf, count);
    printf("%d\n", c_size);
    printf("%d\n\n", ret);

    uLongf u_size = count;
    Bytef* u_buf = (Bytef*)SAFE_MALLOC(u_size);

    ret = uncompress(u_buf, &u_size, c_buf, c_size);
    printf("%d\n\n", ret);

    for(int i = 0; i < count; ++i){
        buf[i] = i % 10;
        printf("%d ", buf[i]);
    }
    printf("\n\n");

    free(buf);
    free(c_buf);
    free(u_buf);

    return 0;
}
