#ifndef ElementOS_H
#define ElementOS_H

#include "elementos-conf.hpp"
#include "elementos-api.hpp"

#include "app-system.hpp"

namespace ElementOS{
    elementosApi *Api;
    elementos OS;

    void setup(){

        /*
        *  device register part
        *  TO DO: register the device you want to use in this project here.
        */

        /* start the system core. */
        (*Api).startApplication<system::core>("system","core",true);
    }

}

class elementos{
    public:
        elementos(){
            this->corePointer = new elementosCore;
            this->apiPointer = new elementosApi(this->corePointer);
            ElementOS::Api = this->apiPointer;
        }

    private:
        elementosCore *corePointer;
        elementosApi *apiPointer;
};

#endif