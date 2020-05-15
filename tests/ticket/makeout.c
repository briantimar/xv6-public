#include <stdio.h>

int main(int argc, char *argv[])
{
    FILE* target = fopen("1.out", "w");
   fprintf(target, "TEST 1\r\n");
   fprintf(target, "TEST 10\r\n");
   fprintf(target, "TEST 10\r\n");
   fclose(target);
   return 0;
}