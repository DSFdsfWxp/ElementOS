#ifndef ElementOS_APP_system
#define ElementOS_APP_system

#include "elementos.hpp"

namespace system{
    const char *packageName = "system";

    class core{
        public:
            const char *appClassName = "core";
            const char *packageName = system::packageName;
            void start(){
                /*
                *  System entrance here.
                *  TO DO: put the code you want to run when the OS startup.
                *         (e.g.:start another app ...)
                */
            }
    };
}

#endif