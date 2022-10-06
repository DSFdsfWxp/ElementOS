#ifndef ElementOS_sessionMgr
#define ElementOS_sessionMgr

class foregroundMgr{
    public:
        bool switchForeground(char packageName,char applicationName){
            this->tLock_lock();
            bool result = false;
            if (packageName!=this->pName && applicationName!=this->aName){
                this->pName = packageName;
                this->aName = applicationName;
                result = true;
            }
            this->tLock_unlock();
        }
        
        char getForegroundPackageName(){
            return this->pName;
        }
        
        char getForegroundApplicationName(){
            return this->aName;
        }
    
    private:
        char pName;
        char aName;
        
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

class lockMgr{
    public:
    
    private:
        
}

#endif