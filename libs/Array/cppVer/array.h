#ifndef ELEMENTOS_ARRAY_H
#define ELEMENTOS_ARRAY_H

#include <string>

template <class T>
class Array{
    public:

        Array();
        Array(const Array &rhs);
        ~Array();

        int push(T content);
        T pop();
        int indexOf(T content);
        bool includes(T content);
        int length();

        Array<T>& operator=(Array&);
        T& operator[](int offset);

    private:

        T *arrayContext;
        int _length;

        T* initArray(int size);
        void freeArray(T* obj);
        void copyArray(T* source,T* destination,int source_length,int destination_length);
        void resize(int size);

};


template <class T>
T* Array<T>::initArray(int size){
    T* obj = new T[size];
    for (int i=0;i<size;i++){
        obj[i] = 0;
    }
    return obj;
}

template <class T>
void Array<T>::freeArray(T* obj){
    delete [] obj;
}

template <class T>
Array<T>::Array(){
    this->arrayContext=initArray(1);
    this->_length = 0;
}

template <class T>
Array<T>::~Array(){
    delete [] this->arrayContext;
}

template <class T>
int Array<T>::length(){
    return this->_length;
}

template <class T>
void Array<T>::copyArray(T* source,T* destination,int source_length,int destination_length){
    for (int i=0;i<destination_length;i++){
        //destination[i] = ((i+1<source_length) ? source[i] : 0);
        if(i<source_length)
        destination[i]=source[i];
    }
}


template <class T>
Array<T>::Array(const Array &rhs){
    this->_length = rhs.length();
    this->arrayContext = this->initArray(((this->_length==0) ? 1 : this->_length));
    for (int i=0;i<this->_length;i++){
        this->arrayContext[i] = rhs[i];
    }
}

template <class T>
void Array<T>::resize(int size){
    if (size==this->_length){
        return;
    }
    T *newArrayContext;
    newArrayContext = this->initArray(size);
    this->copyArray(this->arrayContext,newArrayContext,this->_length,size);
    this->freeArray(this->arrayContext);
    this->arrayContext = newArrayContext;
    this->_length = size;
}

template <class T>
Array<T>& Array<T>::operator=(Array& rhs){
    if (this==&rhs){
        return *this;
    }
    this->freeArray(this->arrayContext);
    this->_length = rhs.length();
    this->arrayContext = this->initArray(this->_length);
    for (int i=0;i<this->_length;i++){
        this->arrayContext[i] = rhs[i];
    }
    return *this;
}

template <class T>
T& Array<T>::operator[](int offset){
    if (offset<0 || offset+1>this->_length){
        throw "Offser is out of the area of this array.";
    }
    return this->arrayContext[offset];
}

template <class T>
int Array<T>::push(T content){
    this->resize(this->_length+1);
    this->arrayContext[this->_length-1] = content;
    return this->_length;
}

template <class T>
T Array<T>::pop(){
    T old = this->arrayContext[this->_length-1];
    this->resize(this->_length-1);
    return old;
}

template <class T>
int Array<T>::indexOf(T content){
    for(int i=0;i<this->_length;i++){
        if (this->arrayContext[i]==content){
            return i;
        }
    }
    return -1;
}

template <class T>
bool Array<T>::includes(T content){
    return ((this->indexOf(content)>-1) ? true : false);
}

#endif