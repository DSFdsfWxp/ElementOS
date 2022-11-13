#ifndef ElementOS_sessionMgr
#define ElementOS_sessionMgr

#include "inc/lfarray.hpp"
#include "inc/statusLock.hpp"
#include "conf/elementos-namecase.hpp"
#include "elementos-std.hpp"
#include "errList.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"



namespace ElementOS{

class contextLockMgr{
    public:
        contextLockMgr(){
            Serial.printf("hello world from contextLockMgr::contextLockMgr()\n");
            this->lockStatus = 0;
        }

        void lock(){
            if (this->lockStatus==0){
                //vTaskSuspendAll();
            }
            this->lockStatus++;
            Serial.printf("contextLockMgr::lockStatus => %d\n",this->lockStatus);
        }

        void unlock(){
            if (this->lockStatus>=1){
                this->lockStatus--;
                Serial.printf("contextLockMgr::lockStatus => %d\n",this->lockStatus);
            }
            if (this->lockStatus==1){
                //xTaskResumeAll();
            }
        }

    private:
        int lockStatus;
};

class lockMgr{
    public:
        void createLock(elestd_name groupName,elestd_name lockName){
            Serial.printf("hello world from lockMgr::createLock(...)\n");
            this->lock.take();
            long pos = this->lockGroupName.indexOf(groupName);
            if (pos==-1){
                pos = this->lockGroupName.push(groupName) - 1;
                Serial.printf("hello world from lockMgr::createLock(...)\n");
                //lfArray<long> tmpB;
                Serial.printf("hello world from lockMgr::createLock(...)\n");
                this->lockName.pushNew();
                this->lockKey.pushNew();
            }
            Serial.printf("hello world from lockMgr::createLock(...)\n");
            if(this->lockName[pos].includes(lockName)){
                this->lock.give();
                throw ERR_sessionMgr_lockExisted;
            }
            this->lockName[pos].push(lockName);
            this->lockKey[pos].push(-1);
            Serial.printf("goodbye world from lockMgr::createLock(...)\n");
            /*
            Serial.printf("\n# Output of a lfArray<int>[%ld]\n",this->lockGroupName.length());
    	    for (long i=0;i<this->lockGroupName.length();i++){
        	Serial.printf(" %ld => %d\n",i,this->lockGroupName[i]);
    	    }
            Serial.printf("\n");
            
            Serial.printf("\n# Output of a lfArray<int>[%ld]\n",this->lockName[pos].length());
    	    for (long i=0;i<this->lockName[pos].length();i++){
        	Serial.printf(" %ld => %d\n",i,this->lockName[pos][i]);
    	    }
            Serial.printf("\n");
            
            Serial.printf("\n# Output of a lfArray<int>[%ld]\n",this->lockKey[pos].length());
    	    for (long i=0;i<this->lockKey[pos].length();i++){
        	Serial.printf(" %ld => %d\n",i,this->lockKey[pos][i]);
    	    }
            Serial.printf("\n");
            */
            //delay(1);
            this->lock.give();
        }

        void removeLock(elestd_name groupName,elestd_name lockName){
            this->lock.take();
            long pos = this->lockGroupName.indexOf(groupName);
            if (pos==-1){
                this->lock.give();
                throw ERR_sessionMgr_lockGroupNotExisted;
            }
            long lockPos = this->lockName[pos].indexOf(lockName);
            if (lockPos==-1){
                this->lock.give();
                throw ERR_sessionMgr_lockNotExisted;
            }
            this->lockKey[pos].remove(lockPos);
            this->lockName[pos].remove(lockPos);
            if (this->lockName[pos].length()==0){
                this->lockName.remove(pos);
                this->lockKey.remove(pos);
                this->lockGroupName.remove(pos);
            }
            this->lock.give();
        }

        bool verifyLock(elestd_name groupName,elestd_name lockName,long key){
            this->lock.begin();
            long pos = this->lockGroupName.indexOf(groupName);
            if (pos==-1){
                this->lock.end();
                throw ERR_sessionMgr_lockGroupNotExisted;
            }
            long lockPos = this->lockName[pos].indexOf(lockName);
            if (lockPos==-1){
                this->lock.end();
                throw ERR_sessionMgr_lockNotExisted;
            }
            bool result = false;
            if (key==this->lockKey[pos][lockPos]){
                result = true;
            }
            this->lock.end();
            return result;
        }

        long takeLock(elestd_name groupName,elestd_name lockName){
            this->lock.take();
            long pos = this->lockGroupName.indexOf(groupName);
            if (pos==-1){
                this->lock.give();
                throw ERR_sessionMgr_lockGroupNotExisted;
            }
            long lockPos = this->lockName[pos].indexOf(lockName);
            if (lockPos==-1){
                this->lock.give();
                throw ERR_sessionMgr_lockNotExisted;
            }
            if (this->lockKey[pos][lockPos]!=-1){
                this->lock.give();
                return -1;
            }
            long result = random(2147483647);
            this->lockKey[pos][lockPos] = result;
            this->lock.give();
            return result;
        }

        void giveLock(elestd_name groupName,elestd_name lockName,long key){
            this->lock.take();
            long pos = this->lockGroupName.indexOf(groupName);
            if (pos==-1){
                this->lock.give();
                throw ERR_sessionMgr_lockGroupNotExisted;
            }
            long lockPos = this->lockName[pos].indexOf(lockName);
            if (lockPos==-1){
                this->lock.give();
                throw ERR_sessionMgr_lockNotExisted;
            }
            if (key!=this->lockKey[pos][lockPos]){
                this->lock.give();
                throw ERR_sessionMgr_lockKeyNotValid;
            }
            this->lockKey[pos][lockPos] = -1;
            this->lock.give();
        }

    private:
        lfArray<lfArray<long>> lockKey;
        lfArray<lfArray<elestd_name>> lockName;
        lfArray<elestd_name> lockGroupName;
        statusLock lock;
};

class sessionMgr{
    public:
        contextLockMgr contextLock;
        lockMgr lock;

        void createSession(elestd_name name){
            this->slock.take();
            if(this->sessionName.includes(name)){
                this->slock.give();
                throw ERR_sessionMgr_sessionExisted;
            }
            Serial.printf("hello world from sessionMgr::createSession(...)\n");
            long pos = this->sessionName.push(name) - 1;
            lfArray<elestd_name> tmpA;
            lfArray<void*> tmpB;
            Serial.printf("hello world from sessionMgr::createSession(...)\n");
            this->sessionFormTitle.pushNew();
            Serial.printf("hello world from sessionMgr::createSession(...)\n");
            this->sessionFormValue.pushNew();
            Serial.printf("hello world from sessionMgr::createSession(...)\n");
            this->slock.give();
        }

        template <typename Tvalue>
        void addSessionFormCase(elestd_name name,elestd_name caseName,Tvalue value){
            this->slock.take();
            Serial.printf("hello world from sessionMgr::addSessionFormCase(...)\n");
            if (!(this->sessionName.includes(name))){
                this->slock.give();
                throw ERR_sessionMgr_sessionNotExisted;
            }
            long pos = this->sessionName.indexOf(name);
            if (this->sessionFormTitle[pos].includes(caseName)){
                this->slock.give();
                throw ERR_sessionMgr_sessionFormCaseExisted;
            }
            Serial.printf("hello world from sessionMgr::addSessionFormCase(...)\n");
            this->sessionFormTitle[pos].push(caseName);
            Serial.printf("hello world from sessionMgr::addSessionFormCase(...)\n");
            Tvalue *newValue = new Tvalue;
            *newValue = value;
            this->sessionFormValue[pos].push((void*)newValue);
            Serial.printf("hello world from sessionMgr::addSessionFormCase(...)\n");
            this->slock.give();
        }

        template <typename Tvalue>
        void removeSessionFormCase(elestd_name name,elestd_name caseName){
            this->slock.take();
            if (!(this->sessionName.includes(name))){
                this->slock.give();
                throw ERR_sessionMgr_sessionNotExisted;
            }
            long pos = this->sessionName.indexOf(name);
            if (!(this->sessionFormTitle[pos].includes(caseName))){
                this->slock.give();
                throw ERR_sessionMgr_sessionFormCaseNotExisted;
            }
            long casePos = this->sessionFormTitle[pos].indexOf(caseName);
            this->sessionFormTitle[pos].remove(casePos);
            delete (Tvalue*)(this->sessionFormValue[pos][casePos]);
            this->sessionFormValue[pos].remove(casePos);
            this->slock.give();
        }

        template <typename Tvalue>
        Tvalue getSessionFormCase(elestd_name name,elestd_name caseName){
            this->slock.begin();
            if (!(this->sessionName.includes(name))){
               this->slock.end();
                throw ERR_sessionMgr_sessionNotExisted;
            }
            long pos = this->sessionName.indexOf(name);
            if (!(this->sessionFormTitle[pos].includes(caseName))){
                this->slock.end();
                throw ERR_sessionMgr_sessionFormCaseNotExisted;
            }
            long casePos = this->sessionFormTitle[pos].indexOf(caseName);
            Tvalue result =  *((Tvalue*)(this->sessionFormValue[pos][casePos]));
            this->slock.end();
            return result;
        }

        template <typename Tvalue>
        void setSessionFormCase(elestd_name name,elestd_name caseName,Tvalue value){
            this->slock.take();
            if (!(this->sessionName.includes(name))){
                this->slock.give();
                throw ERR_sessionMgr_sessionNotExisted;
            }
            long pos = this->sessionName.indexOf(name);
            if (!(this->sessionFormTitle[pos].includes(caseName))){
                this->slock.give();
                throw ERR_sessionMgr_sessionFormCaseNotExisted;
            }
            long casePos = this->sessionFormTitle[pos].indexOf(caseName);
            *((Tvalue*)(this->sessionFormValue[pos][casePos])) = value;
            this->slock.give();
        }

        void removeSession(elestd_name name){
            this->slock.take();
            if (!(this->sessionName.includes(name))){
                this->slock.give();
                throw ERR_sessionMgr_sessionNotExisted;
            }
            long pos = this->sessionName.indexOf(name);
            if (this->sessionFormTitle[pos].length() != 0){
                this->slock.give();
                throw ERR_sessionMgr_sessionFormNotEmpty;
            }
            this->sessionFormTitle.remove(pos);
            this->sessionFormValue.remove(pos);
            this->sessionName.remove(pos);
            this->slock.give();
        }
        
    private:
        lfArray<elestd_name> sessionName;
        lfArray<lfArray<elestd_name>> sessionFormTitle;
        lfArray<lfArray<void*>> sessionFormValue;
        statusLock slock;
};

}

#endif
