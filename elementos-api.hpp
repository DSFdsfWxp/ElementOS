#ifndef ElementOS_Api
#define ElementOS_Api

#include "elementos-core.hpp"

class elementosApi{
    public:
        elementosApi(elementosCore* corePointer){
            this->corePointer = corePointer;
        }

    private:
        elementosCore* corePointer;
};

#endif