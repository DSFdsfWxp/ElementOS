#ifndef ElementOS_lfarray
#define ElementOS_lfarray

template <typename Tlfarray_lf>
struct lfarray_lf{
    Tlfarray_lf value;
    lfarray_lf<Tlfarray_lf> *nextLf;
};

template <typename Tlfarray>
class lfArray{
    public:
        lfArray(){
            this->_length = 0;
            this->firstLf = new lfarray_lf<Tlfarray>;
        }
            
        ~lfArray(){
            lfarray_lf<Tlfarray> *nowLf = (*(this->firstLf)).nextLf;
            lfarray_lf<Tlfarray> *nextLf = 0;
            delete this->firstLf;
            if (this->_length<=1){
                return;
            }
            for (long i=1;i<this->_length;i++){
                nextLf = (*nowLf).nextLf;
                delete nowLf;
                nowLf = nextLf;
            }
        }
        
        Tlfarray pop(){
            this->tLock_lock();
            Tlfarray oldValue;
            if (this->_length>0){
                lfarray_lf<Tlfarray> *oldLf = this->getLf(this->_length - 1);
                oldValue = (*oldLf).value;
                delete oldLf;
                this->_length--;
            }
            this->tLock_unlock();
            return oldValue;
        }
        
        long push(Tlfarray content){
            this->tLock_lock();
            lfarray_lf<Tlfarray> *lastLf;
            lfarray_lf<Tlfarray> *newLf;
            if (this->_length==0){
                newLf = this->firstLf;
                lastLf = newLf;
            }else{
                lastLf = this->getLf(this->_length - 1);
                newLf = new lfarray_lf<Tlfarray>;
            }
            (*newLf).value = content;
            (*lastLf).nextLf = newLf;
            this->_length++;
            this->tLock_unlock();
            return this->_length;
        }
        
        Tlfarray remove(long pos){
            if (pos<0 || pos>=this->_length){
                throw "pos out of range.";
            }
            this->tLock_lock();
            Tlfarray oldValue;
            if (this->_length>0 && pos==0){
                lfarray_lf<Tlfarray> *nextLf = this->getLf(1);
                oldValue = (*(this->firstLf)).value;
                delete this->firstLf;
                this->firstLf = nextLf;
                this->_length--;
                if (this->_length==0){
                    this->firstLf = new lfarray_lf<Tlfarray>;
                }
                this->tLock_unlock();
                return oldValue;
            }
            if (this->_length>0 && pos>0 && pos<(this->_length - 1)){
                lfarray_lf<Tlfarray> *previousLf = this->getLf(pos-1);
                lfarray_lf<Tlfarray> *posLf = (*previousLf).nextLf;
                lfarray_lf<Tlfarray> *nextLf = (*posLf).nextLf;
                oldValue = (*posLf).value;
                delete posLf;
                (*previousLf).nextLf = nextLf;
                this->_length--;
                this->tLock_unlock();
                return oldValue;
            }
            if (this->_length>0 && pos==(this->_length - 1)){
                lfarray_lf<Tlfarray> *posLf = this->getLf(this->_length - 1);
                oldValue = (*posLf).value;
                delete posLf;
                this->_length--;
                this->tLock_unlock();
                return oldValue;
            }
        }
        
        long add(long pos,Tlfarray content){
            if (pos<0 || pos>this->_length){
                return this->_length;
            }
            this->tLock_lock();
            lfarray_lf<Tlfarray> *previousLf = this->getLf(pos-1);
            lfarray_lf<Tlfarray> *nextLf = (*previousLf).nextLf;
            lfarray_lf<Tlfarray> *newLf = new lfarray_lf<Tlfarray>;
            (*newLf).value = content;
            if (pos<this->_length && pos>0){
                (*newLf).nextLf = nextLf;
            }
            if (pos>0){
                (*previousLf).nextLf = newLf;
            }else{
                (*newLf).nextLf = this->firstLf;
                this->firstLf = newLf;
            }
            this->_length++;
            this->tLock_unlock();
            return this->_length;
        }
        
        void safeWrite(long pos,Tlfarray content){
            if (pos<0 || pos>=this->_length){
                throw "pos out of range.";
            }
            this->tLock_lock();
            (*(this->getLf(pos))).value = content;
            this->tLock_unlock();
        }
        
        Tlfarray& operator[](long pos){
            if (pos<0 || pos>=this->_length){
                throw "pos out of range.";
            }
            return (*(this->getLf(pos))).value;
        }
        
        long indexOf(Tlfarray content){
            for (long i=0;i<this->_length;i++){
                if ((*(this->getLf(i))).value==content){
                    return i;
                }
            }
            return -1;
        }
        
        bool includes(Tlfarray content){
            return (this->indexOf(content)==-1) ? false : true;
        }
        
        long length(){
            return this->_length;
        }
        
    private:
        long _length;
        lfarray_lf<Tlfarray> *firstLf;
        
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
        
        lfarray_lf<Tlfarray> *getLf(long pos){
            lfarray_lf<Tlfarray> *result = this->firstLf;
            for (long i=0;i<this->_length;i++){
                if (i==pos){
                    break;
                }
                result = (*result).nextLf;
            }
            return result;
        }
};

#endif