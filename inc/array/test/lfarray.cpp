
#include "stdio.h"
#include "iostream"
#include "lfarray.hpp"


void printlfArrayInt(lfArray<int> &a){
    printf("\n# Output of a lfArray<int>[%ld]\n",a.length());
    for (long i=0;i<a.length();i++){
        printf(" %ld => %d\n",i,a[i]);
    }
    printf("\n");
}

int main(){
    printf("Defining a...\n");
    lfArray<int> a;
    
    printf("Pushing int into a...\n");
    for (int i=1;i<6;i++){
        a.push(i);
    }
    printlfArrayInt(a);
    
    printf("Letting a[0] = 2 ...\n");
    a[0] = 2;
    printlfArrayInt(a);
    
    printf("Poping a int out of a...\n");
    a.pop();
    printlfArrayInt(a);
    
    printf("Adding 11 to pos 0 in a...\n");
    a.add(0,11);
    printlfArrayInt(a);
    
    printf("Adding 99 to pos 5 in a...\n");
    a.add(5,99);
    printlfArrayInt(a);
    
    printf("Adding 66 to pos 3 in a...\n");
    a.add(3,66);
    printlfArrayInt(a);
    
    printf("Removing the int at pos 0 in a...\n");
    a.remove(0);
    printlfArrayInt(a);
    
    printf("Removing the int at pos 5 in a...\n");
    a.remove(5);
    printlfArrayInt(a);
    
    printf("Removing the int at pos 3 in a...\n");
    a.remove(3);
    printlfArrayInt(a);
    
    printf("Index of 3 in a: %ld\n",a.indexOf(3));
    printf("Index of 73 in a: %ld\n",a.indexOf(73));
    printf("If a includes 66: %s\n",((a.includes(66)) ? "true" : "false"));
    printf("If a includes 676: %s\n",((a.includes(676)) ? "true" : "false"));
    
    return 0;
}
