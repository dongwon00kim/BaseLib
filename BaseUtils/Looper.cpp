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

#include <baseutils/Handler.h>
#include <baseutils/Looper.h>
#include <baseutils/Message.h>
#include "BaseThread.h"
#include "LooperRoster.h"

using namespace std;
using namespace std::chrono;

namespace baseutils {

class Looper::LooperThread : public BaseThread {
public:
    LooperThread(Looper* looper)
        : BaseThread(),
          mLooper(looper),
          mThreadId() {
    }

    virtual ~LooperThread() { }

    virtual Result readyToRun() {
        mThreadId = getThreadId();

        return BaseThread::readyToRun();
    }

    virtual bool threadLoop() {
        return mLooper->loop();
    }

    bool isCurrentThread() const {
        return mThreadId == getThreadId();
    }

private:
    LooperThread(const LooperThread&) = delete;

    LooperThread& operator=(const LooperThread&) = delete;

    Looper* mLooper;
    thread::id mThreadId;
};

// static
int64_t Looper::GetNowUs() {
    time_point<high_resolution_clock> now = high_resolution_clock::now();
    high_resolution_clock::duration time = now.time_since_epoch();
    return duration_cast<microseconds>(time).count();
}

// static
chrono::system_clock::duration Looper::GetNow() {
    time_point<system_clock> now = system_clock::now();
    return now.time_since_epoch();
}

Looper::Looper()
    : mRunningLocally(false) {
}

Looper::~Looper() {
	stop();
}

void Looper::setName(const std::string& name) {
    mName = name;
}

Looper::handler_id Looper::registerHandler(const shared_ptr<Handler>& handler) {
    return LooperRoster::getInstance()->registerHandler(shared_from_this(), handler);
}

void Looper::unregisterHandler(handler_id handlerID) {
    LooperRoster::getInstance()->unregisterHandler(handlerID);
}

Result Looper::start(bool runOnCallingThread) {

    if (runOnCallingThread) {
        {
            unique_lock<mutex> autoLock(mLock);

            if (mThread != NULL || mRunningLocally) {
                return Result::ER_INVALID_OPERATION;
            }

            mRunningLocally = true;
        }

        do {
        } while (loop());

        return Result::OK;
    }

    unique_lock<mutex> autoLock(mLock);

    if (mThread != NULL || mRunningLocally) {
        return Result::ER_ALREADY_OPERATED;
    }

    mThread = make_shared<LooperThread>(this);

    Result err = mThread->run();
    if (err != Result::OK) {
        mThread.reset();
    }

    return err;
}

Result Looper::stop() {
    shared_ptr<LooperThread> thread;
    bool runningLocally;

    {
        unique_lock<mutex> autoLock(mLock);

        thread = mThread;
        runningLocally = mRunningLocally;
        mThread.reset();
        mRunningLocally = false;
    }

    if (thread == NULL && !runningLocally) {
        return Result::ER_INVALID_OPERATION;
    }

    if (thread != NULL) {
        thread->requestExit();
    }

    mQueueChangedCondition.notify_one();

    if (!runningLocally && !thread->isCurrentThread()) {
        // If not running locally and this thread _is_ the looper thread,
        // the loop() function will return and never be called again.
        thread->requestExitAndWait();
    }

    return Result::OK;
}

void Looper::post(const shared_ptr<Message>& msg, const system_clock::duration& delay) {
    unique_lock<mutex> autoLock(mLock);

    system_clock::duration when;
    if (delay > system_clock::duration(0)) {
        when = GetNow() + delay;
    } else {
        when = GetNow();
    }

    list<Event>::iterator itr = mEventQueue.begin();
    while (itr != mEventQueue.end() && (*itr).mWhen <= when) {
        ++itr;
    }

    Event event;
    event.mWhen = when;
    event.mMessage = msg;

    if (itr == mEventQueue.begin()) {
        mQueueChangedCondition.notify_one();
    }

    mEventQueue.insert(itr, event);
}

Result Looper::cancel(const shared_ptr<Message>& msg) {
    unique_lock<mutex> autoLock(mLock);
    Result ret = Result::ER_NAME_NOT_FOUND;

    for(auto itr = mEventQueue.begin(); itr != mEventQueue.end(); ++itr) {
    	if(itr->mMessage == msg) {
        	mEventQueue.erase(itr);
			ret = Result::OK;
			break;
    	}
    }
    return ret;
}

bool Looper::loop() {
    Event event;

    {
        unique_lock<mutex> autoLock(mLock);
        if (mThread == NULL && !mRunningLocally) {
            return false;
        }
        if (mEventQueue.empty()) {
            mQueueChangedCondition.wait(autoLock);
            return true;
        }

        system_clock::duration when = (*mEventQueue.begin()).mWhen;
        system_clock::duration now = GetNow();

        if (when > now) {
        	system_clock::duration delay = when - now;
            mQueueChangedCondition.wait_for(autoLock, delay);
            return true;
        }

        event = *mEventQueue.begin();
        mEventQueue.erase(mEventQueue.begin());
    }

    LooperRoster::getInstance()->deliverMessage(event.mMessage);


    // NOTE: It's important to note that at this point our "Looper" object
    // may no longer exist (its final reference may have gone away while
    // delivering the message). We have made sure, however, that loop()
    // won't be called again.

    return true;
}

} // namespace baseutils
