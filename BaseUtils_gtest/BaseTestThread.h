#ifndef _BASE_TEST_THREAD_H
#define _BASE_TEST_THREAD_H

#include <BaseThread.h>

using namespace utils;
using namespace utils::baseutils;

template<typename T>
class BaseTestThread : public BaseThread {
public:
	BaseTestThread(T* imp) : mImp(imp) {}

	virtual ~BaseTestThread() = default;
private:
    T* mImp;

    virtual bool threadLoop() { return mImp->threadLoop(); }
};

#endif // _BASE_TEST_THREAD_H
