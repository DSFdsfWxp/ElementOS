#ifndef ElementOS_sessionMgr
#define ElementOS_sessionMgr

#include "inc/array/lfarray.hpp"
#include "errList.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"

class foregroundMgr{
    public:
        bool switchForeground(const char *packageName,const char *applicationName){
            this->tLock_lock();
            bool result = false;
            if ((*packageName)!=this->pName && (*applicationName)!=this->aName){
                this->pName = *packageName;
                this->aName = *applicationName;
                result = true;
            }
            this->tLock_unlock();
        }
        
        const char getForegroundPackageName(){
            return this->pName;
        }
        
        const char getForegroundApplicationName(){
            return this->aName;
        }
    
    private:
        char pName;
        char aName;
        
        void tLock_lock(){
            vTaskSuspendAll();
        }
        
        void tLock_unlock(){
            xTaskResumeAll();
        }
};

class lockMgr{
    public:
        lockMgr(){
            this->isLock = false;
        }
        void createLock(const char *groupName,const char *lockName){
            this->tLock_lock();
            long pos = this->lockGroupName.indexOf(*groupName);
            if (pos==-1){
                pos = this->lockGroupName.push(*groupName) - 1;
                lfArray<char> tmpA;
                lfArray<long> tmpB;
                this->lockName.add(pos,tmpA);
                this->lockKey.add(pos,tmpB);
            }
            if(this->lockName[pos].includes(*lockName)){
                throw ERR_sessionMgr_lockExisted;
            }
            this->lockName[pos].push(*lockName);
            this->lockKey[pos].push(random());
            this->tLock_unlock();
        }

        void removeLock(const char *groupName,const char *lockName){
            this->tLock_lock();
            long pos = this->lockGroupName.indexOf(*groupName);
            if (pos==-1){
                throw ERR_sessionMgr_lockGroupNotExisted;
            }
            long lockPos = this->lockName[pos].indexOf(*lockName);
            if (lockPos==-1){
                throw ERR_sessionMgr_lockNotExisted;
            }
            this->lockKey[pos].remove(lockPos);
            this->lockName[pos].remove(lockPos);
            if (this->lockName[pos].length()==0){
                this->lockName.remove(pos);
                this->lockKey.remove(pos);
                this->lockGroupName.remove(pos);
            }
            this->tLock_unlock();
        }

        bool verifyLock(const char *groupName,const char *lockName,long key){
            this->tLock_lock();
            long pos = this->lockGroupName.indexOf(*groupName);
            if (pos==-1){
                throw ERR_sessionMgr_lockGroupNotExisted;
            }
            long lockPos = this->lockName[pos].indexOf(*lockName);
            if (lockPos==-1){
                throw ERR_sessionMgr_lockNotExisted;
            }
            bool result = false;
            if (key==this->lockKey[pos][lockPos]){
                result = true;
            }
            this->tLock_unlock();
            return result;
        }

        long takeLock(const char *groupName,const char *lockName){
            this->tLock_lock();
            long pos = this->lockGroupName.indexOf(*groupName);
            if (pos==-1){
                throw ERR_sessionMgr_lockGroupNotExisted;
            }
            long lockPos = this->lockName[pos].indexOf(*lockName);
            if (lockPos==-1){
                throw ERR_sessionMgr_lockNotExisted;
            }
            if (this->lockKey[pos][lockPos]!=-1){
                this->tLock_unlock();
                return -1;
            }
            this->lockKey[pos][lockPos] = random();
            long result = this->lockKey[pos][lockPos];
            this->tLock_unlock();
            return result;
        }

        void giveLock(const char *groupName,const char *lockName,long key){
            this->tLock_lock();
            long pos = this->lockGroupName.indexOf(*groupName);
            if (pos==-1){
                throw ERR_sessionMgr_lockGroupNotExisted;
            }
            long lockPos = this->lockName[pos].indexOf(*lockName);
            if (lockPos==-1){
                throw ERR_sessionMgr_lockNotExisted;
            }
            if (key!=this->lockKey[pos][lockPos]){
                throw ERR_sessionMgr_lockKeyNotValid;
            }
            this->lockKey[pos][lockPos] = -1;
            this->tLock_unlock();
        }

    private:
        lfArray<lfArray<long>> lockKey;
        lfArray<lfArray<char>> lockName;
        lfArray<char> lockGroupName;
        bool isLock;

        void tLock_lock(){
            if (!(this->isLock)){
                vTaskSuspendAll();
                this->isLock = true;
            }
        }
        
        void tLock_unlock(){
            if (this->isLock){
                xTaskResumeAll();
                this->isLock = false;
            }
        }

};

class sessionMgr{
    public:
        foregroundMgr foreground;
        lockMgr lock;
};

#endif