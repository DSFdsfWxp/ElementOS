
#include "Arduino.h"
#include "array.hpp"


void arrayTest(){
    Array<int> a;
    Array<int> b;
    Serial.printf("Pushing numbers into a.\n");
    for (int i=0;i<5;i++){
        a.push(i);
    }
    Serial.printf("Listing numbers in a:\n");
    for (int i=0;i<5;i++){
        Serial.printf(" -> %d\n",a[i]);
    }
    Serial.printf("The length of a: %d\n",a.length());
    Serial.printf("Poping a number out of a.\n");
    a.pop();
    Serial.printf("The length of a: %d\n",a.length());
    Serial.printf("Listing numbers in a:\n");
    for (int i=0;i<4;i++){
        Serial.printf(" -> %d\n",a[i]);
    }
    Serial.printf("Leting b=a.\n");
    b = a;
    Serial.printf("The length of b: %d\n",b.length());
    Serial.printf("Listing numbers in b:\n");
    for (int i=0;i<4;i++){
        Serial.printf(" -> %d\n",b[i]);
    }
    Serial.printf("Letting b[3]=5.\n");
    b[3] = 5;
    Serial.printf("Listing numbers in a:\n");
    for (int i=0;i<4;i++){
        Serial.printf(" -> %d\n",a[i]);
    }
    Serial.printf("Listing numbers in b:\n");
    for (int i=0;i<4;i++){
        Serial.printf(" -> %d\n",b[i]);
    }

    Serial.printf("If b includes number 5 ? %s\n",(b.includes(5) ? "true" : "false"));
    Serial.printf("If b includes number 10 ? %s\n",(b.includes(10) ? "true" : "false"));
    Serial.printf("The index of number 5 in b : %d\n",b.indexOf(5));

  
}



void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  Serial.printf("Start arrayTest\n");

  arrayTest();

  Serial.printf("End arrayTest\n");
}
