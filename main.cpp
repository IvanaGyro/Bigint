
#include <dlfcn.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    void *fHandle;
    void (*func)();

    fHandle = dlopen("./libhello.so", RTLD_LAZY);

    if (!fHandle) {
        fprintf (stderr, "%s\n", dlerror());
        exit(1);
    }
    dlerror(); // flush errors

    const char *func_sym;
    if (argc > 1) {
        func_sym = argv[1];        
    } else {
        func_sym = "hello_word";
    }
    func = (void(*)())dlsym(fHandle, func_sym);

    if (func) {
        func();
    } else {
        fprintf (stderr, "%s\n", dlerror());
        exit(1);
    }

    dlclose(fHandle);
    return 0;
}
