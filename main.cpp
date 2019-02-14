#include <cstdio>
#include <cstdlib>
#include <cstring>


int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    
    char var[5] = {'y','o','!','!','\0'};
    printf("Hello, world! %s\n", var);
    strcpy(var, "ok");
    printf("%s\n", var);
    return EXIT_SUCCESS;
}