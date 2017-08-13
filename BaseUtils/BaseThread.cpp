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

/** @file BaseThread.cpp
 *
 *  Brief description.
 *
 *  @author            Dongwon, Kim (dongwon00.kim@gmail.com)
 *  @version           1.0
 *  @date              2016.05.11
 *  @note
 *  @see
 */

#include <stddef.h>
#include <assert.h>
#include <mutex>
#include <condition_variable>

#include <BaseThread.h>

using namespace std;

namespace utils {
namespace baseutils {

BaseThread::BaseThread()
    :   mThreadId(-1),
        mLock(),
        mResult(Result::NO_ERROR),
        mExitPending(false),
        mRunning(false) {
}

Result BaseThread::readyToRun() {
    return Result::NO_ERROR;
}

Result BaseThread::run() {
    AutoLock lock(mLock);

    if (mRunning) {
        // thread already started
        return Result::ER_INVALID_OPERATION;
    }

    // reset status and exitPending to their default value, so we can
    // try again after an error happened (either below, or in readyToRun())
    mResult = Result::NO_ERROR;
    mExitPending = false;

    mRunning = true;

    mThread = make_shared<thread>(_threadLoop, shared_from_this());
    if (NULL == mThread.get()) {
        mResult = Result::ER_UNKNOWN_ERROR;
        mRunning = false;
        mThreadId = thread::id(-1);

        return Result::ER_UNKNOWN_ERROR;
    }

    mThreadId = mThread->get_id();
    mThread->detach();

    return Result::NO_ERROR;

    // Exiting scope of mLock is a memory barrier and allows new thread to run
}

int BaseThread::_threadLoop(shared_ptr<BaseThread> sharedSelf) {
    weak_ptr<BaseThread> weakSelf(sharedSelf);

    bool first = true;

    do {
        bool result = false;

        if (first) {
            first = false;
            sharedSelf->mResult = sharedSelf->readyToRun();
            result = (sharedSelf->mResult == Result::NO_ERROR);

            if (result && !sharedSelf->exitPending()) {
                result = sharedSelf->threadLoop();
            }
        } else {
            result = sharedSelf->threadLoop();
        }

        // establish a scope for mLock
        {
            AutoLock lock(sharedSelf->mLock);
            if (result == false || sharedSelf->mExitPending) {
                sharedSelf->mExitPending = true;
                sharedSelf->mRunning = false;
                // clear thread ID so that requestExitAndWait() does not exit if
                // called by a new thread using the same thread ID as this one.
                sharedSelf->mThread.reset();
                // note that interested observers blocked in requestExitAndWait are
                // awoken by notify_all, but blocked on mLock until break exits scope
                sharedSelf->mThreadExitedCondition.notify_all();
                break;
            }
        }

        // Release our strong reference, to let a chance to the thread
        // to die a peaceful death.
        sharedSelf.reset();
        // And immediately, re-acquire a strong reference for the next loop
        sharedSelf = weakSelf.lock();
    } while(NULL != sharedSelf);

    return 0;
}

void BaseThread::requestExit() {
    AutoLock lock(mLock);
    mExitPending = true;
}

Result BaseThread::requestExitAndWait() {
    AutoLock lock(mLock);

    if (mThreadId == getThreadId()) {
       // Don't call waitForExit() from this Thread object's thread. It's a guaranteed deadlock!
        return Result::ER_WOULD_BLOCK;
    }

    mExitPending = true;

    while (mRunning == true) {
        mThreadExitedCondition.wait(lock);
    }
    // This next line is probably not needed any more, but is being left for
    // historical reference. Note that each interested party will clear flag.
    mExitPending = false;

    return mResult;
}

Result BaseThread::join() {
    AutoLock lock(mLock);

    if (mThreadId == getThreadId()) {
       // Don't call join() from this Thread object's thread. It's a guaranteed deadlock!
        return Result::ER_WOULD_BLOCK;
    }

    while (mRunning == true) {
        mThreadExitedCondition.wait(lock);
    }

    return mResult;
}

bool BaseThread::isRunning() const {
    AutoLock lock(mLock);
    return mRunning;
}

thread::id BaseThread::getThreadId() const {
	return this_thread::get_id();
}

bool BaseThread::exitPending() const {
    AutoLock lock(mLock);
    return mExitPending;
}

}; // namespace baseutils
}; // namespace utils
