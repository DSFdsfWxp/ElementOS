#ifndef ELEMENTOS_ARRAY_H
#define ELEMENTOS_ARRAY_H

#include <string>

template <typename Tarray>
class Array {
  public:
    
    Array() {
      this->arrayContext = initArray(1);
      this->_length = 0;
    }

    ~Array() {
      delete [] this->arrayContext;
    }

    int length() {
      return this->_length;
    }

    Array(const Array &rhs) {
      this->_length = rhs.length();
      this->arrayContext = this->initArray(((this->_length == 0) ? 1 : this->_length));
      for (int i = 0; i < this->_length; i++) {
        this->arrayContext[i] = rhs[i];
      }
    }

    Array<Tarray>& operator=(Array& rhs) {
      if (this == &rhs) {
        return *this;
      }
      this->freeArray(this->arrayContext);
      this->_length = rhs.length();
      this->arrayContext = this->initArray(this->_length);
      for (int i = 0; i < this->_length; i++) {
        this->arrayContext[i] = rhs[i];
      }
      return *this;
    }

    Tarray& operator[](int offset) {
      if (offset < 0 || offset + 1 > this->_length) {
        throw "Offser is out of the area of this array.";
      }
      return this->arrayContext[offset];
    }

    int push(Tarray content) {
      this->resize(this->_length + 1);
      this->arrayContext[this->_length - 1] = content;
      return this->_length;
    }

    Tarray pop() {
      Tarray old = this->arrayContext[this->_length - 1];
      this->resize(this->_length - 1);
      return old;
    }

    int indexOf(Tarray content) {
      for (int i = 0; i < this->_length; i++) {
        if (this->arrayContext[i] == content) {
          return i;
        }
      }
      return -1;
    }

    bool includes(Tarray content) {
      return ((this->indexOf(content) > -1) ? true : false);
    }

  private:

    Tarray* arrayContext;
    int _length;


    Tarray* initArray(int size) {
      Tarray* obj = new Tarray[size];
      for (int i = 0; i < size; i++) {
        obj[i] = 0;
      }
      return obj;
    }

    void freeArray(Tarray* obj) {
      delete [] obj;
    }

    void resize(int size) {
      if (size == this->_length) {
        return;
      }
      Tarray* newArrayContext;
      newArrayContext = this->initArray(size);
      this->copyArray(this->arrayContext, newArrayContext, this->_length, size);
      this->freeArray(this->arrayContext);
      this->arrayContext = newArrayContext;
      this->_length = size;
    }

    void copyArray(Tarray* source, Tarray* destination, int source_length, int destination_length) {
      for (int i = 0; i < destination_length; i++) {
        //destination[i] = ((i+1<source_length) ? source[i] : 0);
        if (i < source_length)
          destination[i] = source[i];
      }
    }


};
#endif
