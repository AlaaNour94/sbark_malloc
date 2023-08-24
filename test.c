#include <stdio.h>
#include "alloc.h"
#include <string.h>

typedef struct {
    int age;
    int phone;
} person;

person *create(){
    person *p = t_malloc(sizeof(person));
    if (!p){
        printf("NOOOOOOOOOOOO");
    }
    p->age = 1;
    p->phone = 15;

    return p;
}
int main(){
    person *p = create();
    printf("phone : %d  age : %d\n", p->phone, p->age);
    t_free(p);
}