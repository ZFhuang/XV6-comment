#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    for(int i = 1; i < argc; i++){
        int count=0;
        while(1){
            if(argv[i][count]=='\0')    //find out the end
                break;
            else
                count++;
        }
        for(int j=count-1;j>=0;j--)
            printf(1,"%c",argv[i][j]);  //reversal
        printf(1, "%s", i+1 < argc ? " " : "\n");
    }
    exit();
}

