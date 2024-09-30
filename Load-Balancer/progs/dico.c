#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv){

    float load;
    char buffer[64];

    FILE *loadavg = fopen(argv[2], "w");
    if (!loadavg){
        printf("Error! : opening file\n");   
        exit(1);  
    }
    fputs(argv[1], loadavg);
    fclose(loadavg);
    
    loadavg = fopen(argv[2], "r");
    fgets(buffer, 64, loadavg);
    printf("J'ai terminé, voici la valeur transmis %s\n", buffer);
    fclose(loadavg);
    return 0;

}
