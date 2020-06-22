#include <string_utils.h>
#include <stdio.h>

int main() {
    char long_string[] = "Hello World hello world";
    char to_find[] = "hello world";


    printf("Found at %zu", find_in_string(long_string, to_find));
    //assert(find_in_string(long_string, to_find);



    return 0;
}
