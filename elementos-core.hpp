#ifndef ElementOS_Core
#define ElementOS_Core

#include "core-hal.hpp"
#include "core-hardwareMgr.hpp"
#include "core-taskMgr.hpp"
#include "core-sessionMgr.hpp"
#include "core-applicationMgr.hpp"

class elementosCore{
    public:
        hardwareMgr hardware;
        taskMgr task;
        sessionMgr session;
        applicationMgr application;
};

#endif