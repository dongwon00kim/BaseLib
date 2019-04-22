/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BASETHREAD_H_
#define BASETHREAD_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <baseutils/Result.h>

namespace baseutils {

class BaseThread : virtual public std::enable_shared_from_this<BaseThread>
{
public:
    // Create a Thread object, but doesn't create or start the associated
    // thread. See the run() method.
                        BaseThread();
    virtual             ~BaseThread() = default;

    // Start the thread in threadLoop() which needs to be implemented.
    virtual Result      run();

    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();

    // Good place to do one-time initializations
    virtual Result      readyToRun();

    // Call requestExit() and wait until this object's thread exits.
    // BE VERY CAREFUL of deadlocks. In particular, it would be silly to call
    // this function from this object's thread. Will return WOULD_BLOCK in
    // that case.
            Result      requestExitAndWait();

    // Wait until this object's thread exits. Returns immediately if not yet running.
    // Do not call from this object's thread; will return WOULD_BLOCK in that case.
            Result      join();

    // Indicates whether this thread is running or not.
            bool        isRunning() const;

    // Get Thread id handle.
            std::thread::id getThreadId() const;

protected:
    // exitPending() returns true if requestExit() has been called.
            bool        exitPending() const;

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop() = 0;

private:
    BaseThread&         operator=(const BaseThread&);
    static  int         _threadLoop(std::shared_ptr<BaseThread> sharedSelf);

            std::shared_ptr<std::thread> mThread;
            std::thread::id    mThreadId;
    // always hold mLock when reading or writing
    mutable std::mutex         mLock;
            std::condition_variable mThreadExitedCondition;
            Result             mResult;
    // note that all accesses of mExitPending and mRunning need to hold mLock
    volatile bool              mExitPending;
    volatile bool              mRunning;
};

template<typename T>
class ThreadLooper : public BaseThread {
public:
	ThreadLooper(T* imp) : mImp(imp) {}

    virtual ~ThreadLooper() {}

private:
    T* mImp;

    virtual bool threadLoop() { return mImp->threadLoop(); }
};

} // namespace baseutils

#endif // BASETHREAD_H_
