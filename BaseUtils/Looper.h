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

/** @file Looper.h
 *
 *  Brief description.
 *
 *  @author            Dongwon, Kim (dongwon00.kim@gmail.com)
 *  @version           1.0
 *  @date              2016.05.11
 *  @note
 *  @see
 */

#ifndef _LOOPER_H_
#define _LOOPER_H_

#include <memory>
#include <string>
#include <list>
#include <cstdint>
#include <chrono>

#include <Base.h>
#include <BaseThread.h>

using namespace std;

namespace utils {
namespace baseutils {

class Handler;
class Message;

class Looper : virtual public enable_shared_from_this<Looper> {
public:
    typedef int32_t event_id;
    typedef int32_t handler_id;

    Looper();

    virtual ~Looper();

    // Takes effect in a subsequent call to start().
    void setName(const string& name);

    handler_id registerHandler(const shared_ptr<Handler>& handler);
    void unregisterHandler(handler_id handlerID);

    Result start(bool runOnCallingThread = false);

    Result stop();

    static int64_t GetNowUs();

    static chrono::system_clock::duration GetNow();

private:
    friend class LooperRoster;

    struct Event {
        chrono::system_clock::duration mWhen;
        shared_ptr<Message> mMessage;
    };

    Mutex mLock;
    Condition mQueueChangedCondition;

    string mName;

    list<Event> mEventQueue;

    class LooperThread;
    shared_ptr<LooperThread> mThread;
    bool mRunningLocally;

    void post(const shared_ptr<Message>& msg, const chrono::system_clock::duration& delay);

    Result cancel(const shared_ptr<Message>& msg);

    bool loop();

    DISALLOW_EVIL_CONSTRUCTORS(Looper);
};

}; // namespace baseutils
}; // namespace utils

#endif  // _LOOPER_H_
