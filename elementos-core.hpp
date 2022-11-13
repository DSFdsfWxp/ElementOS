#ifndef ElementOS_Core
#define ElementOS_Core

#include "conf/core-hal.hpp"
#include "core-hardwareMgr.hpp"
#include "core-taskMgr.hpp"
#include "core-sessionMgr.hpp"
#include "core-applicationMgr.hpp"

namespace ElementOS{

class elementosCore{
    public:
        ElementOS::sessionMgr session;
        ElementOS::hardwareMgr hardware;
        ElementOS::taskMgr task;
        ElementOS::applicationMgr application;
};

}

#endif