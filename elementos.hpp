#ifndef ElementOS_H
#define ElementOS_H

#include "elementos-conf.hpp"
#include "elementos-api.hpp"

class elementos{
    public:
        elementos(){
            this->corePointer = new elementosCore;
            this->apiPointer = new elementosApi(this->corePointer);
        }

    private:
        elementosCore *corePointer;
        elementosApi *apiPointer;
};

#endif