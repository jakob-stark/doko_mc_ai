#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "simulate.h"
#include "mc.h"
#include "core.h"

int main(void) {
    InputInfo input_info = {
        {"jakob", "jakob", "jakob", "jakob"},
        1,
        0,
        {0},
        0,
        {0,1,2,3,4,5,6,7,8,9,10,11},
        12
    };
    
    CardId res = GetBestCard(&input_info);
    printf("%d\n",res);
        
	return 0;
}
