#include "array.h"


int main(){
    Array<int> a;
    Array<int> b;
    printf("Pushing numbers into a.\n");
    for (int i=0;i<5;i++){
        a.push(i);
    }
    printf("Listing numbers in a:\n");
    for (int i=0;i<5;i++){
        printf(" -> %d\n",a[i]);
    }
    printf("The length of a: %d\n",a.length());
    printf("Poping a number out of a.\n");
    a.pop();
    printf("The length of a: %d\n",a.length());
    printf("Listing numbers in a:\n");
    for (int i=0;i<4;i++){
        printf(" -> %d\n",a[i]);
    }
    printf("Leting b=a.\n");
    b = a;
    printf("The length of b: %d\n",b.length());
    printf("Listing numbers in b:\n");
    for (int i=0;i<4;i++){
        printf(" -> %d\n",b[i]);
    }
    printf("Letting b[3]=5.\n");
    b[3] = 5;
    printf("Listing numbers in a:\n");
    for (int i=0;i<4;i++){
        printf(" -> %d\n",a[i]);
    }
    printf("Listing numbers in b:\n");
    for (int i=0;i<4;i++){
        printf(" -> %d\n",b[i]);
    }

    printf("If b includes number 5 ? %s\n",(b.includes(5) ? "true" : "false"));
    printf("If b includes number 10 ? %s\n",(b.includes(10) ? "true" : "false"));
    printf("The index of number 5 in b : %d\n",b.indexOf(5));

    return 0;
}