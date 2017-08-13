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

/** @file LooperRoster.h
 *
 *  Brief description.
 *
 *  @author            Dongwon, Kim (dongwon00.kim@gmail.com)
 *  @version           1.0
 *  @date              2016.05.11
 *  @note
 *  @see
 */

#ifndef _LOOPER_ROSTER_H_
#define _LOOPER_ROSTER_H_

#include <map>
#include <memory>

#include <Looper.h>

namespace utils {
namespace baseutils {

class LooperRoster {
public:
    static LooperRoster* getInstance();

    Looper::handler_id registerHandler(const shared_ptr<Looper> looper, const shared_ptr<Handler>& handler);

    void unregisterHandler(Looper::handler_id handlerId);

    Result postMessage(const shared_ptr<Message>& msg, const chrono::system_clock::duration& delay);

    Result cancelMessage(const shared_ptr<Message>& msg);

    void deliverMessage(const shared_ptr<Message>& msg);

    Result postAndAwaitResponse(const shared_ptr<Message>& msg, shared_ptr<Message>& response);

    void postReply(uint32_t replyID, const shared_ptr<Message>& reply);

    shared_ptr<Looper> findLooper(Looper::handler_id handlerId);

private:
    class HandlerInfo {
    public:
        weak_ptr<Looper> mLooper;
        weak_ptr<Handler> mHandler;
    };

    static LooperRoster* mInstance;

    Mutex mLock;

    /**
     * key : handler ID
     * value : HandlerInfo
     */
    map<Looper::handler_id, HandlerInfo> mHandlers;
    Looper::handler_id mNextHandlerId;
    uint32_t mNextReplyId;
    Condition mRepliesCondition;

    /**
     * key : replyId
     * value : Message
     */
    map<uint32_t, shared_ptr<Message> > mReplies;

    Result postMessage_l(const shared_ptr<Message>& msg, const chrono::system_clock::duration& delay);

    Result cancelMessage_l(const shared_ptr<Message>& msg);

    // Singleton
    LooperRoster();
    ~LooperRoster();

    DISALLOW_EVIL_CONSTRUCTORS(LooperRoster);
};

}; // namespace baseutils
}; // namespace utils

#endif  // _LOOPER_ROSTER_H_
