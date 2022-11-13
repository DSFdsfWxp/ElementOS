#ifndef ElementOS_taskMgr
#define ElementOS_taskMgr

#include "inc/lfarray.hpp"
#include "inc/statusLock.hpp"
#include "errList.hpp"
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
/*#include "freertos/portmacro.h"*/
#include "freertos/task.h"
#include "conf/elementos-conf.hpp"
#include "conf/elementos-namecase.hpp"
#include "elementos-std.hpp"

#if ElementOS_conf_fixStackDeepType
#define configSTACK_DEPTH_TYPE const uint32_t
#endif

namespace ElementOS{

enum xtaskStatus{
    xtaskRunning,
    xtaskSuspended,
    xtaskSuspendedBySuspendAll
};

class xtaskMgr{
    public:
        BaseType_t createTask(TaskFunction_t pvTaskCode,
                              elestd_name pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority){
            Serial.printf("hello world from xtaskMgr::createTask(%p,%d,%d,%p,%p)\n",pvTaskCode,(int)pcName,usStackDepth,pvParameters,uxPriority);
            this->lock.take();
            if (this->taskName.includes(pcName)){
                this->lock.give();
                throw ERR_taskMgr_taskExisted;
            }
            long pos = this->taskName.push(pcName) - 1;
            TaskHandle_t* newTask = new TaskHandle_t;
            BaseType_t result;
            result = xTaskCreate(pvTaskCode,"t",usStackDepth,pvParameters,uxPriority,newTask);
            if (result==pdPASS){
                Serial.printf("ok.\n");
                this->taskHandle.push(newTask);
                this->taskStatus.push(xtaskRunning);
            }else{
                this->taskName.remove(pos);
                delete newTask;
            }
            this->lock.give();
            Serial.printf("goodbye world from xtaskMgr::createTask(%p,%d,%d,%p,%p)\n",pvTaskCode,(int)pcName,usStackDepth,pvParameters,uxPriority);
            return result;
        }

        TaskHandle_t* createTaskWithoutName(TaskFunction_t pvTaskCode,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority){
            this->lock.take();
            long pos = this->taskName.push(ElementOS::n_noneNameTask) - 1;
            TaskHandle_t* newTask = new TaskHandle_t;
            BaseType_t result;
            result = xTaskCreate(pvTaskCode,"t",usStackDepth,pvParameters,uxPriority,newTask);
            if (result==pdPASS){
                this->taskHandle.push(newTask);
                this->taskStatus.push(xtaskRunning);
            }else{
                this->taskName.remove(pos);
                delete newTask;
                this->lock.give();
                throw result;
            }
            this->lock.give();
            return newTask;
        }
        
        BaseType_t createTaskPinnedToCore(TaskFunction_t pvTaskCode,
                              elestd_name pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,
                              const BaseType_t xCoreID){
            this->lock.take();
            if (this->taskName.includes(pcName)){
                this->lock.give();
                throw ERR_taskMgr_taskExisted;
            }
            long pos = this->taskName.push(pcName) - 1;
            TaskHandle_t* newTask = new TaskHandle_t;
            BaseType_t result;
            result = xTaskCreatePinnedToCore(pvTaskCode,"t",usStackDepth,pvParameters,uxPriority,newTask,xCoreID);
            if (result==pdPASS){
                this->taskHandle.push(newTask);
                this->taskStatus.push(xtaskRunning);
            }else{
                this->taskName.remove(pos);
                delete newTask;
            }
            this->lock.give();
            return result;
        }
        
        BaseType_t createTaskStatic(TaskFunction_t pvTaskCode,
                              elestd_name pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,StackType_t *const puxStackBuffer,
                              StaticTask_t *const pxTaskBuffer){
            this->lock.take();
            if (this->taskName.includes(pcName)){
                this->lock.give();
                throw ERR_taskMgr_taskExisted;
            }
            long pos = this->taskName.push(pcName) - 1;
            TaskHandle_t* newTask = new TaskHandle_t;
            BaseType_t result;
            *newTask = xTaskCreateStatic(pvTaskCode,"t",usStackDepth,pvParameters,uxPriority,puxStackBuffer,pxTaskBuffer);
            if (*newTask!=NULL){
                this->taskHandle.push(newTask);
                this->taskStatus.push(xtaskRunning);
                result = pdPASS;
            }else{
                this->taskName.remove(pos);
                delete newTask;
                result = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
            }
            this->lock.give();
            return result;
        }
        
        BaseType_t createTaskStaticPinnedToCore(TaskFunction_t pvTaskCode,
                              elestd_name pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,StackType_t *const puxStackBuffer,
                              StaticTask_t *const pxTaskBuffer,
                              const BaseType_t xCoreID){
            this->lock.take();
            if (this->taskName.includes(pcName)){
                this->lock.give();
                throw ERR_taskMgr_taskExisted;
            }
            long pos = this->taskName.push(pcName) - 1;
            TaskHandle_t* newTask = new TaskHandle_t;
            BaseType_t result;
            *newTask = xTaskCreateStaticPinnedToCore(pvTaskCode,"t",usStackDepth,pvParameters,uxPriority,puxStackBuffer,pxTaskBuffer,xCoreID);
            if (*newTask!=NULL){
                this->taskHandle.push(newTask);
                this->taskStatus.push(xtaskRunning);
                result = pdPASS;
            }else{
                this->taskName.remove(pos);
                delete newTask;
                result = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
            }
            this->lock.give();
            return result;
        }
        
        #if ElementOS_conf_enableTaskTracking || ElementOS_conf_enableDebug
        
        elestd_name getTaskName(long pos){
            this->lock.begin();
            elestd_name result = this->taskName[pos];
            this->lock.end();
            return result;
        }
        
        #endif
        
        void deleteTask(elestd_name pcName){
            this->lock.take();
            if (!(this->taskName.includes(pcName))){
                this->lock.give();
                throw ERR_taskMgr_taskNotExisted;
            }
            long pos = this->taskName.indexOf(pcName);
            vTaskDelete(*(this->taskHandle[pos]));
            this->taskHandle.remove(pos);
            this->taskName.remove(pos);
            this->taskStatus.remove(pos);
            this->lock.give();
        }

        void deleteTask(TaskHandle_t *taskHandleP){
            this->lock.take();
            if (!(this->taskHandle.includes(taskHandleP))){
                this->lock.give();
                throw ERR_taskMgr_taskNotExisted;
            }
            long pos = this->taskHandle.indexOf(taskHandleP);
            vTaskDelete(*(this->taskHandle[pos]));
            this->taskHandle.remove(pos);
            this->taskName.remove(pos);
            this->taskStatus.remove(pos);
            this->lock.give();
        }
        
        void suspendTask(elestd_name pcName){
            this->lock.take();
            if (!(this->taskName.includes(pcName))){
                this->lock.give();
                throw ERR_taskMgr_taskNotExisted;
            }
            long pos = this->taskName.indexOf(pcName);
            if (this->taskStatus[pos]==xtaskRunning){
                vTaskSuspend(*(this->taskHandle[pos]));
            }
            this->taskStatus[pos] = xtaskSuspended;
            this->lock.give();
        }

        void suspendTask(TaskHandle_t *taskHandleP){
            this->lock.take();
            if (!(this->taskHandle.includes(taskHandleP))){
                this->lock.give();
                throw ERR_taskMgr_taskNotExisted;
            }
            long pos = this->taskHandle.indexOf(taskHandleP);
            if (this->taskStatus[pos]==xtaskRunning){
                vTaskSuspend(*(this->taskHandle[pos]));
            }
            this->taskStatus[pos] = xtaskSuspended;
            this->lock.give();
        }
        
        void resumeTask(elestd_name pcName){
            this->lock.take();
            if (!(this->taskName.includes(pcName))){
                this->lock.give();
                throw ERR_taskMgr_taskNotExisted;
            }
            long pos = this->taskName.indexOf(pcName);
            if (this->taskStatus[pos]==xtaskSuspended || this->taskStatus[pos]==xtaskSuspendedBySuspendAll){
                vTaskResume(*(this->taskHandle[pos]));
            }
            this->taskStatus[pos] = xtaskRunning;
            this->lock.give();
        }

        void resumeTask(TaskHandle_t *taskHandleP){
            this->lock.take();
            if (!(this->taskHandle.includes(taskHandleP))){
                this->lock.give();
                throw ERR_taskMgr_taskNotExisted;
            }
            long pos = this->taskHandle.indexOf(taskHandleP);
            if (this->taskStatus[pos]==xtaskSuspended || this->taskStatus[pos]==xtaskSuspendedBySuspendAll){
                vTaskResume(*(this->taskHandle[pos]));
            }
            this->taskStatus[pos] = xtaskRunning;
            this->lock.give();
        }
        
        xtaskStatus getTaskStatus(elestd_name pcName){
            this->lock.begin();
            if (!(this->taskName.includes(pcName))){
                this->lock.end();
                throw ERR_taskMgr_taskNotExisted;
            }
            xtaskStatus result = this->taskStatus[this->taskName.indexOf(pcName)];
            this->lock.end();
            return result;
        }
        
        #if ElementOS_conf_enableTaskTracking || ElementOS_conf_enableDebug
        
        xtaskStatus getTaskStatus(long pos){
            this->lock.begin();
            xtaskStatus result = this->taskStatus[pos];
            this->lock.end();
            return result;
        }
        
        #endif
        
        void deleteAllTask(){
            this->lock.take();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                TaskHandle_t *nowHandle = this->taskHandle.pop();
                vTaskDelete(*nowHandle);
                delete nowHandle;
                this->taskName.pop();
                this->taskStatus.pop();
            }
            this->lock.give();
        }
        
        void deleteAllTask(elestd_name pcName){
            this->lock.take();
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
            this->lock.give();
        }
        
        void suspendAllTask(){
            this->lock.take();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if (this->taskStatus[i]==xtaskRunning){
                    vTaskSuspend(*(this->taskHandle[i]));
                    this->taskStatus[i] = xtaskSuspendedBySuspendAll;
                }
            }
            this->lock.give();
        }
        
        void suspendAllTask(elestd_name pcName){
            this->lock.take();
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
            this->lock.give();
        }
        
        void refreshTask(){
            this->lock.take();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if (this->taskStatus[i]==xtaskSuspendedBySuspendAll){
                    this->taskStatus[i] = xtaskSuspended;
                }
            }
            this->lock.give();
        }
        
        void unsuspendAllTask(){
            this->lock.take();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if (this->taskStatus[i]==xtaskSuspendedBySuspendAll){
                    vTaskResume(*(this->taskHandle[i]));
                    this->taskStatus[i] = xtaskRunning;
                }
            }
            this->lock.give();
        }
        
        void resumeAllTask(){
            this->lock.take();
            long length = this->taskHandle.length();
            for (long i=0;i<length;i++){
                if (this->taskStatus[i]==xtaskSuspended || this->taskStatus[i]==xtaskSuspendedBySuspendAll){
                    vTaskResume(*(this->taskHandle[i]));
                    this->taskStatus[i] = xtaskRunning;
                }
            }
            this->lock.give();
        }
        
        #if ElementOS_conf_enableTaskTracking || ElementOS_conf_enableDebug
        
        long length(){
            this->lock.begin();
            long result = this->taskName.length();
            this->lock.end();
            return result;
        }
        
        #endif
        
    private:
        lfArray<TaskHandle_t*> taskHandle;
        lfArray<elestd_name> taskName;
        lfArray<xtaskStatus> taskStatus;
        statusLock lock;
};

class xtaskCaseList{
    public:
        void createTaskCase(elestd_name name){
            this->lock.take();
            if (this->taskCaseName.includes(name)){
                this->lock.give();
                throw ERR_taskMgr_taskCaseExisted;
            }
            xtaskMgr *newTaskMgr = new xtaskMgr;
            this->taskCase.push(newTaskMgr);
            this->taskCaseName.push(name);
            this->lock.give();
        }
        
        void deleteTaskCase(elestd_name name){
            this->lock.take();
            if (!(this->taskCaseName.includes(name))){
                this->lock.give();
                throw ERR_taskMgr_taskCaseNotExisted;
            }
            long pos = this->taskCaseName.indexOf(name);
            (*(this->taskCase[pos])).deleteAllTask();
            delete this->taskCase[pos];
            this->taskCase.remove(pos);
            this->taskCaseName.remove(pos);
            this->lock.give();
        }
        
        void deleteAllTaskCase(){
            this->lock.take();
            long length = this->taskCase.length();
            for (long i=0;i<length;i++){
                xtaskMgr *nowTaskMgr = this->taskCase.pop();
                (*nowTaskMgr).deleteAllTask();
                delete nowTaskMgr;
                this->taskCaseName.pop();
            }
            this->lock.give();
        }
        
        xtaskMgr* reachTaskCase(elestd_name name){
            this->lock.begin();
            if (!(this->taskCaseName.includes(name))){
                this->lock.end();
                throw ERR_taskMgr_taskCaseNotExisted;
            }
            xtaskMgr* result = this->taskCase[this->taskCaseName.indexOf(name)];
            this->lock.end();
            return result;
        }
        
        #if ElementOS_conf_enableTaskTracking || ElementOS_conf_enableDebug
        
        xtaskMgr* reachTaskCase(long pos){
            this->lock.begin();
            xtaskMgr* result = this->taskCase[pos];
            this->lock.end();
            return result;
        }
        
        elestd_name getTaskCaseName(long pos){
            this->lock.begin();
            elestd_name result = this->taskCaseName[pos];
            this->lock.end();
            return result;
        }
        
        long length(){
            this->lock.begin();
            long result = this->taskCaseName.length();
            this->lock.end();
            return result;
        }
        
        #endif
        
    private:
        lfArray<xtaskMgr*> taskCase;
        lfArray<elestd_name> taskCaseName;
        statusLock lock;
};

class taskMgr{
    public:
        void createTaskCaseList(elestd_name name){
            this->lock.take();
            if (this->taskCaseListName.includes(name)){
                this->lock.give();
                throw ERR_taskMgr_taskCaseListExisted;
            }
            xtaskCaseList *newTaskList = new xtaskCaseList;
            this->taskCaseList.push(newTaskList);
            this->taskCaseListName.push(name);
            this->lock.give();
        }
        
        void deleteTaskCaseList(elestd_name name){
            this->lock.take();
            if (!(this->taskCaseListName.includes(name))){
                this->lock.give();
                throw ERR_taskMgr_taskCaseListNotExisted;
            }
            long pos = this->taskCaseListName.indexOf(name);
            (*(this->taskCaseList[pos])).deleteAllTaskCase();
            delete this->taskCaseList[pos];
            this->taskCaseList.remove(pos);
            this->taskCaseListName.remove(pos);
            this->lock.give();
        }
        
        xtaskCaseList* reachTaskCaseList(elestd_name name){
            this->lock.begin();
            if (!(this->taskCaseListName.includes(name))){
                this->lock.end();
                throw ERR_taskMgr_taskCaseListNotExisted;
            }
            xtaskCaseList* result = this->taskCaseList[this->taskCaseListName.indexOf(name)];
            this->lock.end();
            return result;
        }
        
        #if ElementOS_conf_enableDebug
        void listTask(){
            this->lock.begin();
            Serial.printf("# The list of all task: \n\n# Name # Status #\n");
            for (long i=0;i<this->taskCaseListName.length();i++){
                xtaskCaseList *nowTaskList = this->taskCaseList[i];
                Serial.printf(" \"%d\" -",this->taskCaseListName[i]);
                for (long ii=0;ii<(*nowTaskList).length();ii++){
                    xtaskMgr *nowTaskMgr = (*nowTaskList).reachTaskCase(ii);
                    Serial.printf(" \"%d.%d\" -",this->taskCaseListName[i],(*nowTaskList).getTaskCaseName(ii));
                    for (long iii=0;iii<(*nowTaskMgr).length();iii++){
                        Serial.printf(" \"%d.%d.%d\" %s",this->taskCaseListName[i],(*nowTaskList).getTaskCaseName(ii),((*nowTaskMgr).getTaskName(iii)),(((*nowTaskMgr).getTaskStatus(iii) == xtaskRunning) ? "Running" : "Suspended"));
                    }
                }
            }
            this->lock.end();
        }
        #endif
        
    private:
        lfArray<xtaskCaseList*> taskCaseList;
        lfArray<elestd_name> taskCaseListName;
        statusLock lock;
};

}

#endif
