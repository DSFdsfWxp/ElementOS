#ifndef ElementOS_taskMgr
#define ElementOS_taskMgr

#include "inc/lfarray.hpp"
#include "errList.hpp"
#include "Arduino.h"
#include "task.h"

enum xtaskStatus{
    xtaskRunning,
    xtaskSuspended,
    xtaskSuspendedBySuspendAll
};

class xtaskMgr{
    public:
        BaseType_t createTask(TaskFunction_t pvTaskCode,
                              const char pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority){
            this->tLock_lock();
            if (this->taskName.includes(pcName)){
                throw ERR_taskMgr_taskExisted;
            }
            long pos = this->taskName.push(pcName) - 1;
            TaskHandle_t* newTask = new TaskHandle_t;
            BaseType_t result;
            result = xTaskCreate(pvTaskCode,&(this->taskName[pos]),usStackDepth,pvParameters,uxPriority,newTask);
            if (result==pdPass){
                this->taskHandle.push(newTask);
                this->taskStatus.push(xtaskRunning);
            }else{
                this->taskName.remove(pos);
                delete newTask;
            }
            this->tLock_unlock();
            return result;
        }
        
        BaseType_t createTaskPinnedToCore(TaskFunction_t pvTaskCode,
                              const char pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,
                              const BaseType_t xCoreID){
            this->tLock_lock();
            if (this->taskName.includes(pcName)){
                throw ERR_taskMgr_taskExisted;
            }
            long pos = this->taskName.push(pcName) - 1;
            TaskHandle_t* newTask = new TaskHandle_t;
            BaseType_t result;
            result = xTaskCreatePinnedToCore(pvTaskCode,&(this->taskName[pos]),usStackDepth,pvParameters,uxPriority,newTask,xCoreID);
            if (result==pdPass){
                this->taskHandle.push(newTask);
                this->taskStatus.push(xtaskRunning);
            }else{
                this->taskName.remove(pos);
                delete newTask;
            }
            this->tLock_unlock();
            return result;
        }
        
        BaseType_t createTaskStatic(TaskFunction_t pvTaskCode,
                              const char pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,StackType_t *const puxStackBuffer,
                              StaticTask_t *const pxTaskBuffer){
            this->tLock_lock();
            if (this->taskName.includes(pcName)){
                throw ERR_taskMgr_taskExisted;
            }
            long pos = this->taskName.push(pcName) - 1;
            TaskHandle_t* newTask = new TaskHandle_t;
            BaseType_t result;
            *newTask = xTaskCreateStatic(pvTaskCode,&(this->taskName[pos]),usStackDepth,pvParameters,uxPriority,puxStackBuffer,pxTaskBuffer);
            if (*puxStackBuffer!=NULL && *pxTaskBuffer!=NULL){
                this->taskHandle.push(newTask);
                this->taskStatus.push(xtaskRunning);
                result = pdPass;
            }else{
                this->taskName.remove(pos);
                delete newTask;
                result = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
            }
            this->tLock_unlock();
            return result;
        }
        
        BaseType_t createTaskStaticPinnedToCore(TaskFunction_t pvTaskCode,
                              const char pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,StackType_t *const puxStackBuffer,
                              StaticTask_t *const pxTaskBuffer,
                              const BaseType_t xCoreID){
            this->tLock_lock();
            if (this->taskName.includes(pcName)){
                throw ERR_taskMgr_taskExisted;
            }
            long pos = this->taskName.push(pcName) - 1;
            TaskHandle_t* newTask = new TaskHandle_t;
            BaseType_t result;
            *newTask = xTaskCreateStaticPinnedToCore(pvTaskCode,&(this->taskName[pos]),usStackDepth,pvParameters,uxPriority,puxStackBuffer,pxTaskBuffer);
            if (*puxStackBuffer!=NULL && *pxTaskBuffer!=NULL){
                this->taskHandle.push(newTask);
                this->taskStatus.push(xtaskRunning);
                result = pdPass;
            }else{
                this->taskName.remove(pos);
                delete newTask;
                result = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
            }
            this->tLock_unlock();
            return result;
        }
        
        #if ElementOS_conf_enableTaskTracking || ElementOS_conf_enableDebug
        
        const char getTaskName(long pos){
            return this->taskName[pos];
        }
        
        #endif
        
        void deleteTask(const char pcName){
            this->tLock_lock();
            if (!(this->taskName.includes(pcName))){
                throw ERR_taskMgr_taskNotExisted;
            }
            long pos = this->taskName.indexOf(pcName);
            vTaskDelete(*(this->taskHandle[pos]));
            this->taskHandle.remove(pos);
            this->taskName.remove(pos);
            this->taskStatus.remove(pos);
            this->tLock_unlock();
        }
        
        void suspendTask(const char pcName){
            this->tLock_lock();
            if (!(this->taskName.includes(pcName))){
                throw ERR_taskMgr_taskNotExisted;
            }
            long pos = this->taskName.indexOf(pcName);
            if (this->taskStatus[pos]==xtaskRunning){
                vTaskSuspend(*(this->taskHandle[pos]));
            }
            this->taskStatus[pos] = xtaskSuspended;
            this->tLock_unlock();
        }
        
        void resumeTask(const char pcName){
            this->tLock_lock();
            if (!(this->taskName.includes(pcName))){
                throw ERR_taskMgr_taskNotExisted;
            }
            long pos = this->taskName.indexOf(pcName);
            if (this->taskStatus[pos]==xtaskSuspended || this->taskStatus[pos]==xtaskSuspendedBySuspendAll){
                vTaskResume(*(this->taskHandle[pos]));
            }
            this->taskStatus[pos] = xtaskRunning;
            this->tLock_unlock();
        }
        
        xtaskStatus getTaskStatus(const char pcName){
            if (!(this->taskName.includes(pcName))){
                throw ERR_taskMgr_taskNotExisted;
            }
            return this->taskStatus[this->taskName.indexOf(pcName)];
        }
        
        #if ElementOS_conf_enableTaskTracking || ElementOS_conf_enableDebug
        
        xtaskStatus getTaskStatus(long pos){
            return this->taskStatus[pos];
        }
        
        #endif
        
        void deleteAllTask(){
            this->tLock_lock();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                TaskHandle_t *nowHandle = this->taskHandle.pop();
                vTaskDelete(*nowHandle);
                delete nowHandle;
                this->taskName.pop();
                this->taskStatus.pop();
            }
            this->tLock_unlock();
        }
        
        void deleteAllTask(const char pcName){
            this->tLock_lock();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if ((this->taskName.pop())==pcName){
                    this->taskName.push(pcName);
                    continue;
                }
                TaskHandle_t *nowHandle = this->taskHandle.pop();
                vTaskDelete(*nowHandle);
                delete nowHandle;
                this->taskStatus.pop();
            }
            this->tLock_unlock();
        }
        
        void suspendAllTask(){
            this->tLock_lock();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if (this->taskStatus[i]==xtaskRunning){
                    vTaskSuspend(*(this->taskHandle[i]));
                    this->taskStatus[i] = xtaskSuspendedBySuspendAll;
                }
            }
            this->tLock_unlock();
        }
        
        void suspendAllTask(const char pcName){
            this->tLock_lock();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if (this->taskName[i]==pcName){
                    continue;
                }
                if (this->taskStatus[i]==xtaskRunning){
                    vTaskSuspend(*(this->taskHandle[i]));
                    this->taskStatus[i] = xtaskSuspendedBySuspendAll;
                }
            }
            this->tLock_unlock();
        }
        
        void refreshTask(){
            this->tLock_lock();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if (this->taskStatus[i]==xtaskSuspendedBySuspendAll){
                    this->taskStatus[i] = xtaskSuspended;
                }
            }
            this->tLock_unlock();
        }
        
        void unsuspendAllTask(){
            this->tLock_lock();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if (this->taskStatus[i]==xtaskSuspendedBySuspendAll){
                    vTaskResume(*(this->taskHandle[i]));
                    this->taskStatus[i] = xtaskRunning;
                }
            }
            this->tLock_unlock();
        }
        
        void resumeAllTask(){
            this->tLock_lock();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if (this->taskStatus[i]==xtaskSuspended || this->taskStatus[i]==xtaskSuspendedBySuspendAll){
                    vTaskResume(*(this->taskHandle[i]));
                    this->taskStatus[i] = xtaskRunning;
                }
            }
            this->tLock_unlock();
        }
        
        #if ElementOS_conf_enableTaskTracking || ElementOS_conf_enableDebug
        
        long length(){
            return this->taskName.length();
        }
        
        #endif
        
    private:
        lfArray<TaskHandle_t*> taskHandle;
        lfArray<const char> taskName;
        lfArray<xtaskStatus> taskStatus;
        
        
        void tLock_lock(){
            /*
                Put locking operation of thread safe lock here.
            */
        }
        
        void tLock_unlock(){
            /*
                Put unlocking operation of thread safe lock here.
            */
        }
        
};

class xtaskCaseList{
    public:
        void createTaskCase(char name){
            this->tLock_lock();
            if (this->taskCaseName.includes(name)){
                throw ERR_taskMgr_taskCaseExisted;
            }
            xtaskMgr *newTaskMgr = new xtaskMgr;
            this->taskCase.push(newTaskMgr);
            this->taskCaseName.push(name);
            this->tLock_unlock();
        }
        
        void deleteTaskCase(char name){
            this->tLock_lock();
            if (!(this->taskCaseName.includes(name))){
                throw ERR_taskMgr_taskCaseNotExisted;
            }
            long pos = this->taskCaseName.indexOf(name);
            *(this->taskCase[pos]).deleteAllTask();
            delete this->taskCase[pos];
            this->taskCase.remove(pos);
            this->taskCaseName.remove(pos);
            this->tLock_unlock();
        }
        
        void deleteAllTaskCase(){
            this->tLock_lock();
            long length = this->taskCase.length();
            for (long i=0;i<length;i++){
                xtaskMgr *nowTaskMgr = this->taskCase.pop();
                *nowTaskMgr.deleteAllTask();
                delete nowTaskMgr;
                this->taskCaseName.pop();
            }
            this->tLock_unlock();
        }
        
        xtaskMgr* reachTaskCase(char name){
            if (!(this->taskCaseName.includes(name))){
                throw ERR_taskMgr_taskCaseNotExisted;
            }
            return this->taskCase[this->taskCaseName.indexOf(name)];
        }
        
        #if ElementOS_conf_enableTaskTracking || ElementOS_conf_enableDebug
        
        xtaskMgr* reachTaskCase(long pos){
            return this->taskCase[pos];
        }
        
        char getTaskCaseName(long pos){
            return this->taskCaseName[pos];
        }
        
        long length(){
            return this->taskCaseName.length();
        }
        
        #endif
        
    private:
        lfArray<xtaskMgr*> taskCase;
        lfArray<char> taskCaseName;
        
        void tLock_lock(){
            /*
                Put locking operation of thread safe lock here.
            */
        }
        
        void tLock_unlock(){
            /*
                Put unlocking operation of thread safe lock here.
            */
        }
};

class taskMgr{
    public:
        void createTaskCaseList(char name){
            this->tLock_lock();
            if (this->taskCaseListName.includes(name)){
                throw ERR_taskMgr_taskCaseListExisted;
            }
            xtaskCaseList *newTaskList = new xtaskCaseList;
            this->taskCaseList.push(newTaskList);
            this->taskCaseListName.push(name);
            this->tLock_unlock();
        }
        
        void deleteTaskCaseList(char name){
            this->tLock_lock();
            if (!(this->taskCaseListName.includes(name))){
                throw ERR_taskMgr_taskCaseListNotExisted;
            }
            long pos = this->taskCaseListName.indexOf(name);
            *(this->taskCaseList[pos]).deleteAllTaskCase();
            delete this->taskCaseList[pos];
            this->taskCaseList.remove(pos);
            this->taskCaseListName.remove(pos);
            this->tLock_unlock();
        }
        
        xtaskCaseList* reachTaskCaseList(char name){
            if (!(this->taskCaseListName.includes(name))){
                throw ERR_taskMgr_taskCaseListNotExisted;
            }
            return this->taskCaseList[this->taskCaseListName.indexOf(name)];
        }
        
        #if ElementOS_conf_enableDebug
        void listTask(){
            this->tLock_lock();
            Serial.printf("# The list of all task: \n\n# Name # Status #\n");
            for (long i=0;i<this->taskCaseListName.length();i++){
                xtaskCaseList *nowTaskList = this->taskCaseList[i];
                Serial.printf(" \"%s\" -",this->taskCaseListName[i]);
                for (long ii=0;ii<(*nowTaskList).length();ii++){
                    xtaskMgr *nowTaskMgr = *nowTaskList.reachTaskCase(ii);
                    Serial.printf(" \"%s\" -",this->taskCaseListName[i] + "." + *nowTaskList.getTaskCaseName(ii));
                    for (long iii=0;iii<(*nowTaskMgr).length();iii++){
                        Serial.printf(" \"%s\" %s",this->taskCaseListName[i] + "." + *nowTaskList.getTaskCaseName(ii) + "." + (char)(*nowTaskMgr.getTaskName(iii)),((*nowTaskMgr.getTaskStatus(pos) == xtaskRunning) ? "Running" : "Suspended"));
                    }
                }
            }
            this->tLock_unlock();
        }
        #endif
        
    private:
        lfArray<xtaskCaseList*> taskCaseList;
        lfArray<char> taskCaseListName;
        
        void tLock_lock(){
            /*
                Put locking operation of thread safe lock here.
            */
        }
        
        void tLock_unlock(){
            /*
                Put unlocking operation of thread safe lock here.
            */
        }
};

#endif