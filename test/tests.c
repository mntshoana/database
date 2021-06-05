#include "test.h"
#include "bdd-for-c.h"

spec ("Tests"){
    static int count;
    static int failed;
    
    after(){
        printf("%i tests run. %i tests failed\n", count, failed);
    }
    
    before(){
        count = 0;
        failed = 0;
    }
    
    before_each(){
        printf("\n");
        count++;
        failed++;
    }
    
    after_each(){
        failed--;
    }
    
#include "test1.c"
#include "test2.c"
}


