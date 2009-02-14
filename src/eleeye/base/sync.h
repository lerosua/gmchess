#include "base2.h"
#include "x86asm.h"

#ifndef SYNC_H
#define SYNC_H

inline bool Lock(volatile int &nLock) {
  return Exchange(&nLock, 1) == 0;
}

inline void TryLock(volatile int &nLock) {
  while (!Lock(nLock)) {
    Idle();
  }
}

inline void Unlock(volatile int &nLock) {
  nLock = 0;
}

template <class T> class Stack {
  volatile int nLock;
  int nSize, nTop;
  T *Elements;

public:
  Stack(int nSize_) {
    nLock = 0;
    nSize = nSize_;
    nTop = 0;
    Elements = new T[nSize];
  }
  ~Stack(void) {
    delete[] Elements;
  }
  bool Push(T o) {
    TryLock(nLock);
    if (nTop == nSize) {
      Unlock(nLock);
      return false;
    }
    Elements[nTop] = o;
    nTop ++;
    Unlock(nLock);
    return true;
  }
  bool Pop(T &o) {
    TryLock(nLock);
    if (nTop == 0) {
      Unlock(nLock);
      return false;
    }
    nTop --;
    o = Elements[nTop];
    Unlock(nLock);
    return true;
  }
};

template <class T> class Queue {
  volatile int nLock;
  int nSize, nHead, nTail;
  T *Elements;

public:
  Queue(int nSize_) {
    nLock = 0;
    nSize = nSize_;
    nHead = nTail = 0;
    Elements = new T[nSize];
  }
  ~Queue(void) {
    delete[] Elements;
  }
  bool Offer(T o) {
    TryLock(nLock);
    int nNewTail = (nTail + 1) % nSize;
    if (nNewTail == nHead) {
      Unlock(nLock);
      return false;
    }
    Elements[nTail] = o;
    nTail = nNewTail;
    Unlock(nLock);
    return true;
  }
  bool Poll(T &o) {
    TryLock(nLock);
    if (nTail == nHead) {
      Unlock(nLock);
      return false;
    }
    int nOldHead = nHead;
    nHead = (nHead + 1) % nSize;
    o = Elements[nOldHead];
    Unlock(nLock);
    return true;
  }
};

#endif
