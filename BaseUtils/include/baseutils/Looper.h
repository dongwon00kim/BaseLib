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

#ifndef LOOPER_H_
#define LOOPER_H_

#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <baseutils/Result.h>

namespace baseutils {

class Handler;
class Message;

class Looper : virtual public std::enable_shared_from_this<Looper> {
public:
    typedef int32_t event_id;
    typedef int32_t handler_id;

    Looper();

    virtual ~Looper();

    // Takes effect in a subsequent call to start().
    void setName(const std::string& name);

    handler_id registerHandler(const std::shared_ptr<Handler>& handler);

    void unregisterHandler(handler_id handlerID);

    Result start(bool runOnCallingThread = false);

    Result stop();

    static int64_t GetNowUs();

    static std::chrono::system_clock::duration GetNow();

private:
    friend class LooperRoster;

    struct Event {
        std::chrono::system_clock::duration mWhen;
        std::shared_ptr<Message> mMessage;
    };

    std::mutex mLock;

    std::condition_variable mQueueChangedCondition;

    std::string mName;

    std::list<Event> mEventQueue;

    class LooperThread;

    std::shared_ptr<LooperThread> mThread;

    bool mRunningLocally;

    Looper(const Looper&) = delete;

    Looper& operator=(const Looper&) = delete;

    void post(const std::shared_ptr<Message>& msg, const std::chrono::system_clock::duration& delay);

    Result cancel(const std::shared_ptr<Message>& msg);

    bool loop();
};

} // namespace baseutils

#endif  // LOOPER_H_
