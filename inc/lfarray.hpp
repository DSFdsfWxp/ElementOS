#ifndef ElementOS_lfarray
#define ElementOS_lfarray

#include "Arduino.h"
#include "statusLock.hpp"

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
            this->firstLf = 0;
            this->lastLf = 0;
            this->lock = new statusLock;
        }
	
        lfArray(const lfArray<Tlfarray> &p){
            p.lock->take();
            this->lock = new statusLock;
            this->lock->take();
            if (p._length==0){
                p.lock->give();
                this->_length = 0;
                this->firstLf = 0;
                this->lastLf = 0;
                this->lock->give();
            }
            lfarray_lf<Tlfarray> *nextLf;
            lfarray_lf<Tlfarray> *nowLf = p.firstLf;
            lfarray_lf<Tlfarray> *tnowLf = new lfarray_lf<Tlfarray>;
            this->firstLf = tnowLf;
            for (long i=0;i<p._length;i++){
                nextLf = nowLf->nextLf;
                lfarray_lf<Tlfarray> *newLf = new lfarray_lf<Tlfarray>;
                tnowLf->nextLf = newLf;
                tnowLf->value = nowLf->value;
                this->lastLf = tnowLf;
                tnowLf = newLf;
                nowLf = nextLf;
                this->_length++;
            }
            delete tnowLf;
            this->lastLf->nextLf = this->lastLf;
            this->lock->give();
            p.lock->give();
        }
        
       
        ~lfArray(){
            this->lock->take();
            switch (this->_length){
                case 0:
                    this->lock->give();
                    break;
                case 1:
                    delete this->firstLf;
                    this->firstLf = 0;
                    this->lastLf = 0;
                    this->lock->give();
                    break;
                default:
                    lfarray_lf<Tlfarray> *nextLf;
                    lfarray_lf<Tlfarray> *nowLf = this->firstLf;
                    for (long i=0;i<this->_length;i++){
                        nextLf = nowLf->nextLf;
                        delete nowLf;
                        nowLf = nextLf;
                    }
                    this->lock->give();
                    break;
            }
            delete this->lock;
        }
        
        Tlfarray pop(){
            this->lock->take();
            if (this->_length==0){
                this->lock->give();
                throw "Empty lfArray";
            }
            Tlfarray result = this->lastLf->value;
            delete this->lastLf;
            this->_length--;
            if (this->_length==0){
                this->firstLf = 0;
                this->lastLf = 0;
            }else{
                lfarray_lf<Tlfarray> *t = this->getLf(this->_length - 1);
                t->nextLf = t;
                this->lastLf = t;
            }
            this->lock->give();
            return result;
        }
        
        long push(const Tlfarray &content){
            this->lock->take();
            lfarray_lf<Tlfarray> *newLf = new lfarray_lf<Tlfarray>;
            if (this->_length==0){
                this->firstLf = newLf;
            }else{
                this->lastLf->nextLf = newLf;
            }
            this->lastLf = newLf;
            newLf->value = content;
            newLf->nextLf = newLf;
            this->_length++;
            long result = this->_length;
            this->lock->give();
            return result;
        }
        
        long add(long pos,const Tlfarray &content){
            this->lock->take();
            if (pos<0 | pos>=this->_length){
                this->lock->give();
                throw "Out ofrang.";
            }
            lfarray_lf<Tlfarray> *newLf = new lfarray_lf<Tlfarray>;
            lfarray_lf<Tlfarray> *preLf = this->getLf(pos);
            lfarray_lf<Tlfarray> *nextLf = preLf->nextLf;
            newLf->value = content;
            switch (this->_length){
                case 0:
                    newLf->nextLf = this->firstLf;
                    this->firstLf = newLf;
                    break;
                default:
                    if (this->_length==(this->_length - 1)){
                        this->lastLf->nextLf = newLf;
                        newLf->nextLf = newLf;
                        this->lastLf = newLf;
                        break;
                    }
                    preLf->nextLf = newLf;
                    newLf->nextLf = nextLf;
                    break;
            }
            this->_length++;
            long result = this->_length;
            this->lock->give();
            return result;
        }
        
        Tlfarray remove(long pos){
            this->lock->take();
            if (pos<0 || pos>=this->_length){
                this->lock->give();
                throw "Out of range.";
            }
            lfarray_lf<Tlfarray> *preLf= this->getLf(pos - 1);
            lfarray_lf<Tlfarray> *Lf = preLf->nextLf;
            lfarray_lf<Tlfarray> *nextLf = Lf->nextLf;
            Tlfarray result = Lf->value;
            switch (pos){
                case 0:
                    this->firstLf = nextLf;
                    nextLf->nextLf = nextLf;
                    break;
                default:
                    if (pos==(this->_length - 1)){
                        this->lastLf = preLf;
                        preLf->nextLf = preLf;
                        break;
                    }
                    preLf->nextLf = nextLf;
                    break;
            }
            delete Lf;
            this->_length--;
            if(this->_length==0){
                this->firstLf = 0;
                this->lastLf = 0;
            }
            this->lock->give();
            return result;
        }
        
        long indexOf(const Tlfarray &content){
            this->lock->begin();
            for (long i=0;i<this->_length;i++){
                if (this->getLf(i)->value==content){
                    this->lock->end();
                    return i;
                }
            }
            this->lock->end();
            return -1;
        }

        bool includes(const Tlfarray &content){
            this->lock->begin();
            long pos = this->indexOf(content);
            this->lock->end();
            return ((pos==-1) ? false : true);
        }
        
        
        Tlfarray& operator[](long pos){
            this->lock->begin();
            if (pos<0 || pos>=this->_length){
                this->lock->end();
                throw "Out of range.";
            }
            Tlfarray &result = this->getLf(pos)->value;
            this->lock->end();
            return result;
        }
        
        void write(long pos,const Tlfarray &content){
            this->lock->take();
            if (pos<0 || pos>=this->_length){
                this->lock->give();
                throw "Out of range.";
            }
            this->getLf(pos)->value = content;
            this->lock->give();
        }

        void operator()(long pos,const Tlfarray &content){
            this->write(pos,content);
        }
        
        long length(){
            this->lock->begin();
            long result = this->_length;
            this->lock->end();
            return result;
        }

        long pushNew(long num = 1){
            this->lock->take();
            for (long i=1;i<=num;i++){
                lfarray_lf<Tlfarray> *newLf = new lfarray_lf<Tlfarray>;
                if (this->_length==0){
                    this->firstLf = newLf;
                }else{
                    this->lastLf->nextLf = newLf;
                }
                this->lastLf = newLf;
                newLf->nextLf = newLf;
                this->_length++;
            }
            long result = this->_length;
            this->lock->give();
            return result;
        }

        void pushNewToPos(long pos){
            this->lock->begin();
            if (pos<0 || pos<=this->_length){
                this->lock->end();
                throw "Invaild pos.";
            }
            this->lock->end();
            this->pushNew(pos - 1 - (this->length()));
        }

        void operator=(const lfArray<Tlfarray> &p){
            this->~lfArray();
            this->_length = 0;
            this->firstLf = 0;
            this->lastLf = 0;
            this->lock = new statusLock;
            this->lock->take();
            if (p._length==0){
                this->lock->give();
                return;
            }
            lfArray<Tlfarray> *copy = new lfArray<Tlfarray>(p);
            this->firstLf = copy->firstLf;
            this->lastLf = copy->lastLf;
            this->_length = copy->_length;
            copy->_length = 0;
            delete copy;
            this->lock->give();
        }
        
    private:
        long _length;
        lfarray_lf<Tlfarray> *firstLf;
        lfarray_lf<Tlfarray> *lastLf;
        statusLock *lock;
        
        lfarray_lf<Tlfarray> *getLf(long pos){
            if (pos<=0){
                return this->firstLf;
            }else if(pos>=this->_length){
                return this->lastLf;
            }
            lfarray_lf<Tlfarray> *nowLf = this->firstLf;
            for (long i=0;i<pos;i++){
                nowLf = nowLf->nextLf;
            }
            return nowLf;
        }
};

#endif
