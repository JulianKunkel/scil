#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char ** argv){
	char * lib;
	if (argc == 1){
		printf("Synopsis: <HDF5 plugin.so>\n");
		lib = "libhdf5-filter-scil.so";
	}else{
		lib = argv[1];
	}
	void * ret = dlopen(lib, RTLD_NOW);
	printf("%p, error: %s\n", ret,  dlerror() );
	dlclose(ret);
	return ret == NULL;
}
