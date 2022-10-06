#ifndef ElementOS_errList
#define ElementOS_errList


/*
*  hardwareMgr part
*/

#define ERR_hardwareMgr_deviceExisted 1
#define ERR_hardwareMgr_deviceNotExisted 2
#define ERR_hardwareMgr_deviceCanNotBeRemoved 3

/*
*   taskMgr part
*/

#define ERR_taskMgr_taskExisted 4
#define ERR_taskMgr_taskNotExisted 5
#define ERR_taskMgr_taskCaseExisted 6
#define ERR_taskMgr_taskCaseNotExisted 7
#define ERR_taskMgr_taskCaseListExisted 8
#define ERR_taskMgr_taskCaseListNotExisted 9

/*
*   sessionMgr part
*/

#define ERR_sessionMgr_lockExisted 10
#define ERR_sessionMgr_lockNotExisted 11
#define ERR_sessionMgr_lockGroupNotExisted 12
#define ERR_sessionMgr_lockKeyNotValid 13

/*
*   applicationMgr part
*/

#define ERR_applicationMgr_appClassIsAlreadyAdded 14
#define ERR_applicationMgr_appClassNotInList 15
#define ERR_applicationMgr_appRemoteCallIsAlreadyAdded 16
#define ERR_applicationMgr_appRemoteCallNotInList 17

#endif