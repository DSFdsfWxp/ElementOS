#ifndef ElementOS_Api
#define ElementOS_Api

#include "inc/array/lfarray.hpp"
#include "elementos-core.hpp"
#include "errList.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"

typedef long lockKeyPair;
typedef void (*appClassFunctionPointer)();
typedef void (*appClassRemoteCallFunctionPointer)(const char*,lfArray<void*>*);

namespace ElementOSAPI{
    template <typename Tdevice>
    class deviceAccesser{
        public:
            deviceAccesser(const char *deviceName,Tdevice *devicePointer,lockMgr *lockMgrPointer,lockKeyPair lockKey){
                if (lockKey==-1){
                    throw ERR_api_invalidLockKey;
                }
                this->deviceName = *deviceName;
                this->devicePointer = devicePointer;
                this->devicePointer = lockMgrPointer;
                this->lockKey = lockKey;
            }

            deviceAccesser(const char *deviceName,Tdevice *devicePointer,lockMgr *lockMgrPointer){
                this->deviceName = *deviceName;
                this->devicePointer = devicePointer;
                this->devicePointer = lockMgrPointer;
                this->lockKey = -1;
            }

            Tdevice operator->(){
                if (this->lockKey==-1){
                    return *(this->devicePointer);
                }
                if(!(*(this->lockMgrPointer)).verifyLock("dev",&(this->deviceName),this->lockKey)){
                    throw ERR_api_deviceAccesser_lockLose;
                }
                return *(this->devicePointer);
            }

        private:
            Tdevice *devicePointer;
            lockMgr *lockMgrPointer;
            lockKeyPair lockKey;
            char deviceName;
    };

    void applicationCreateCall(void* p){
        lfArray<void*> args = *(lfArray<void*>*)p;
        appClassFunctionPointer enterPoint = (appClassFunctionPointer)(args[0]);
        xtaskMgr *taskHandle = (xtaskMgr*)(args[1]);
        enterPoint();
        delete (lfArray<void*>*)p;
        (*taskHandle).deleteTask("start");
    }

    void applicationDeleteCall(void* p){
        lfArray<void*> args = *(lfArray<void*>*)p;
        appClassFunctionPointer enterPoint = (appClassFunctionPointer)(args[0]);
        xtaskCaseList *taskHandle = (xtaskCaseList*)(args[1]);
        const char *appClassName = (const char*)(args[2]);
        lockKeyPair key = (lockKeyPair)(args[4]);
        const char *tmp = (const char *)(args[3]);
        elementosApi *apiHandle = (elementosApi*)(args[5]);
        enterPoint();
        (*apiHandle).giveLock("removeApplication",tmp,key);
        delete (lfArray<void*>*)p;
        (*taskHandle).deleteTaskCase(appClassName);
    }

    void applicationRemoteCall(void* p){
        lfArray<void*> args = *(lfArray<void*>*)p;
        appClassRemoteCallFunctionPointer enterPoint = (appClassRemoteCallFunctionPointer)(args[0]);
        xtaskMgr *taskHandle = (xtaskMgr*)(args[1]);
        lfArray<void*> *rcArgs = (lfArray<void*>*)(args[2]);
        const char *taskName = (const char*)(args[3]);
        enterPoint(taskName,rcArgs);
        delete rcArgs;
        delete (lfArray<void*>*)p;
        (*taskHandle).deleteTask(taskName);
    }

}

class elementosApi{
    public:
        elementosApi(elementosCore* corePointer){
            this->corePointer = corePointer;
        }
        
        /* lock Part */

        void createLock(const char *groupName,const char *lockName){
            if (groupName=="dev"){
                throw ERR_api_invalidLockGroupName;
            }
            (*(this->corePointer)).session.lock.createLock(groupName,lockName);
        }

        void removeLock(const char *groupName,const char *lockName,lockKeyPair lockKey){
            if (groupName=="dev"){
                throw ERR_api_invalidLockGroupName;
            }
            if (!(*(this->corePointer)).session.lock.verifyLock(groupName,lockName,lockKey)){
                throw ERR_api_invalidLockKey;
            }
            (*(this->corePointer)).session.lock.removeLock(groupName,lockName);
        }

        bool verifyLock(const char *groupName,const char *lockName,lockKeyPair lockKey){
            return (*(this->corePointer)).session.lock.verifyLock(groupName,lockName,lockKey);
        }

        lockKeyPair takeLock(const char *groupName,const char *lockName){
            lockKeyPair result = -2;
            while (result<0){
                result = (*(this->corePointer)).session.lock.takeLock(groupName,lockName);
                if (result==-1){
                    vTaskDelay(50);
                }
            }
            return result;
        }

        void giveLock(const char *groupName,const char *lockName,lockKeyPair lockKey){
            (*(this->corePointer)).session.lock.giveLock(groupName,lockName,lockKey);
        }

        /* hardware Part */

        template <typename Tdevice>
        ElementOSAPI::deviceAccesser<Tdevice> accessDevice(const char *deviceName,lockKeyPair lockKey){
            if (!((*(this->corePointer)).session.lock.verifyLock("dev",deviceName,lockKey))){
                throw ERR_api_invalidLockKey;
            }
            ElementOSAPI::deviceAccesser<Tdevice> result(deviceName,(*(this->corePointer)).hardware.reachDevice<Tdevice>(deviceName),&((*(this->corePointer)).session.lock),lockKey);
            return result;
        }

        template <typename Tdevice>
        ElementOSAPI::deviceAccesser<Tdevice> accessDevice(const char *deviceName){
            if ((*(this->corePointer)).hardware.isDeviceWithLock(deviceName)){
                throw ERR_api_deviceNeedLock;
            }
            ElementOSAPI::deviceAccesser<Tdevice> result(deviceName,(*(this->corePointer)).hardware.reachDevice<Tdevice>(deviceName),&((*(this->corePointer)).session.lock));
            return result;
        }
        
        template <typename Tdevice>
        void removeDevice(const char *deviceName,lockKeyPair lockKey){
            if (!((*(this->corePointer)).session.lock.verifyLock("dev",deviceName,lockKey))){
                throw ERR_api_invalidLockKey;
            }
            (*(this->corePointer)).hardware.unregDevice<Tdevice>(deviceName);
            (*(this->corePointer)).session.lock.removeLock("dev",deviceName);
        }

        template <typename Tdevice>
        void removeDevice(const char *deviceName){
            if ((*(this->corePointer)).hardware.isDeviceWithLock(deviceName)){
                throw ERR_api_deviceNeedLock;
            }
            (*(this->corePointer)).hardware.unregDevice<Tdevice>(deviceName);
        }

        template <typename Tdevice>
        void installDevice(const char*deviceName,lfArray<void*> initArgs,bool removeable,bool withLock){
            Tdevice *newDevice = new Tdevice(initArgs);
            (*(this->corePointer)).hardware.regDevice<Tdevice>(newDevice,deviceName,removeable,withLock);
            if (withLock){
                (*(this->corePointer)).session.lock.createLock("dev",deviceName);
            }
        }

        /* task Part */

        template <typename Tappclass>
        BaseType_t createTask(TaskFunction_t pvTaskCode,
                              Tappclass *appClass,
                              const char *pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority){
            (*(this->corePointer)).task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((const char *)((*appClass).appClassName))->createTask(pvTaskCode,pcName,usStackDepth,pvParameters,uxPriority);
        }

        template <typename Tappclass>
        BaseType_t createTaskPinnedToCore(TaskFunction_t pvTaskCode,
                              Tappclass *appClass,
                              const char *pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,
                              const BaseType_t xCoreID){
            (*(this->corePointer)).task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((const char *)((*appClass).appClassName))->createTaskPinnedToCore(pvTaskCode,pcName,usStackDepth,pvParameters,uxPriority,xCoreID);
        }

        template <typename Tappclass>
        BaseType_t createTaskStatic(TaskFunction_t pvTaskCode,
                              Tappclass *appClass,
                              const char *pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,StackType_t *const puxStackBuffer,
                              StaticTask_t *const pxTaskBuffer){
            (*(this->corePointer)).task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((const char *)((*appClass).appClassName))->createTaskStatic(pvTaskCode,pcName,usStackDepth,pvParameters,uxPriority,puxStackBuffer,pxTaskBuffer);
        }

        template <typename Tappclass>
        BaseType_t createTaskStaticPinnedToCore(TaskFunction_t pvTaskCode,
                              Tappclass *appClass,
                              const char *pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,StackType_t *const puxStackBuffer,
                              StaticTask_t *const pxTaskBuffer,
                              const BaseType_t xCoreID){
            (*(this->corePointer)).task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((const char *)((*appClass).appClassName))->createTaskStaticPinnedToCore(pvTaskCode,pcName,usStackDepth,pvParameters,uxPriority,puxStackBuffer,pxTaskBuffer,xCoreID);
        }

        template <typename Tappclass>
        void suspendTask(Tappclass *appClass,const char *pcName){
            (*(this->corePointer)).task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((const char *)((*appClass).appClassName))->suspendTask(pcName);
        }

        template <typename Tappclass>
        void resumeTask(Tappclass *appClass,const char *pcName){
            (*(this->corePointer)).task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((const char *)((*appClass).appClassName))->resumeTask(pcName);
        }

        template <typename Tappclass>
        void deleteTask(Tappclass *appClass,const char *pcName){
            (*(this->corePointer)).task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((const char *)((*appClass).appClassName))->deleteTask(pcName);
        }

        /* application Part */

        template <typename TappClass>
        void killApplication(const char *packageName,const char *appClassName){
            if (packageName=="*" && appClassName=="*"){
                return;
            }
            lfArray<void*> *args = new lfArray<void*>;
            (*args).push((void*)(&((*(this->corePointer)).application.reachApplication<TappClass>(packageName,appClassName)->end)));
            (*args).push((void*)((*(this->corePointer)).task.reachTaskCaseList(packageName)));
            (*args).push((void*)appClassName);
            String tmp = packageName;
            tmp += ".";
            tmp += appClassName;
            this->createLock("removeApplication",tmp.c_str());
            (*args).push((void*)(tmp.c_str()));
            lockKeyPair key = this->takeLock("removeApplication",tmp.c_str());
            (*args).push((void*)key)
            (*args).push((void*)(this);
            (*(this->corePointer)).session.foreground.switchForeground("*","*");
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->createTask(&(ElementOSAPI::applicationDeleteCall),"end",512,(void*)args,1);
            key = takeLock("removeApplication",tmp.c_str());
            (*(this->corePointer)).application.removeApplication<TappClass>(packageName,appClassName);
            this->removeLock("removeApplication",tmp.c_str(),key);
        }

        template <typename TappClass>
        void startApplication(const char* packageName,const char *appClassName,bool isService){
            try{
                (*(this->corePointer)).task.createTaskCaseList(packageName);
            }catch(ERR_taskMgr_taskCaseListExisted){}
            try{
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->createTaskCase(appClassName);
            }catch(ERR_taskMgr_taskCaseExisted){
                throw ERR_api_applicationIsExisted;
            }
            if (!isService){
                this->killApplication(&((*(this->corePointer)).session.foreground.getForegroundPackageName()),&((*(this->corePointer)).session.foreground.getForegroundApplicationName()));
                (*(this->corePointer)).session.foreground.switchForeground(packageName,appClassName);
            }
            TappClass *newApp = (*(this->corePointer)).application.addApplication<TappClass>(packageName,appClassName);
            lfArray<void*> *args = new lfArray<void*>;
            (*args).push((void*)(&((*newApp)->start)));
            (*args).push((void*)((*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)));
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->createTask(&(ElementOSAPI::applicationDeleteCall),"start",512,(void*)args,1);
        }

        void suspendApplication(const char* packageName,const char *appClassName){
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->suspendAllTask();
        }

        void suspendApplicationTask(const char* packageName,const char *appClassName,const char *pcName){
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->suspendAllTask(pcName);
        }

        void refreshSuspendedApplicationTaskStatus(const char* packageName,const char *appClassName){
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->refreshTask();
        }

        void resumeApplication(const char* packageName,const char *appClassName){
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->unsuspendAllTask();
        }

        void resumeApplicationTask(const char* packageName,const char *appClassName){
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->resumeAllTask();
        }

        void deleteApplicationTask(const char* packageName,const char *appClassName,const char *pcName){
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->deleteAllTask(pcName);
        }

        template <typename TappClass>
        void addRemoteCallToApplication(TappClass *appClass,const char *remoteCallName,applicationMgr_applicationRemoteCallFunctionPointer remoteCallFunction){
            (*(this->corePointer)).application.addRemoteCall((*appClass).packageName,(*appClass).appClassName,remoteCallName,remoteCallFunction);
        }

        template <typename TappClass>
        void removeRemoteCallFromApplication(TappClass *appClass,const char *remoteCallName){
            (*(this->corePointer)).application.removeRemoteCall((*appClass).packageName,(*appClass).appClassName,remoteCallName);
        }

        void callApplicationRemoteCall(const char *packageName,const char *appClassName,const char *remoteCallName,lfArray<void*> *args){
            lfArray<void*> *callArgs = new lfArray<void*>;
            (*callArgs).push((void*)((*(this->corePointer)).application.reachRemoteCall(packageName,appClassName,remoteCallName)));
            (*callArgs).push((void*)(((*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName))));
            (*callArgs).push((void*)(args));
            String taskName = "RC";
            taskName += remoteCallName;
            taskName += random();
            (*callArgs).push((void*)(taskName.c_str()));
            (*(this->corePointer)).task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->createTask(&(ElementOSAPI::applicationRemoteCall),taskName.c_str(),512,(void*)(callArgs),0);
        }

    private:
        elementosCore* corePointer;
};

#endif