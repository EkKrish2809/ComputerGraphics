// Standard Header files
#include <stdio.h> // for standard I/O
#include <stdlib.h> // for exit()

#ifdef __LINUX__
// nothing here
#else
    #define MYICON 101
#endif

#include "common.h"
#include "window_manager.h"

