#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char ** argv){
	if (argc == 1){
		printf("Synopsis: <HDF5 plugin.so>\n");
		return 1;
	}
	void * ret = dlopen(argv[1], RTLD_NOW);
	printf("%p, error: %s\n", ret,  dlerror() );
	dlclose(ret);
}
