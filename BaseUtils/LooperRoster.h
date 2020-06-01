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

#ifndef LOOPER_ROSTER_H_
#define LOOPER_ROSTER_H_

#include <condition_variable>
#include <map>
#include <memory>
#include <baseutils/Looper.h>

namespace baseutils {

class LooperRoster {
public:
    static LooperRoster* getInstance();

    Looper::handler_id registerHandler(const std::shared_ptr<Looper>& looper, const std::shared_ptr<Handler>& handler);

    void unregisterHandler(Looper::handler_id handlerId);

    Result postMessage(const std::shared_ptr<Message>& msg, const std::chrono::system_clock::duration& delay);

    Result cancelMessage(const std::shared_ptr<Message>& msg);

    void deliverMessage(const std::shared_ptr<Message>& msg);

    Result postAndAwaitResponse(const std::shared_ptr<Message>& msg, std::shared_ptr<Message>& response);

    void postReply(uint32_t replyID, const std::shared_ptr<Message>& reply);

    std::shared_ptr<Looper> findLooper(Looper::handler_id handlerId);

private:
    class HandlerInfo {
    public:
        std::weak_ptr<Looper> mLooper;
        std::weak_ptr<Handler> mHandler;
    };

    static LooperRoster* mInstance;

    std::mutex mLock;

    /**
     * key : handler ID
     * value : HandlerInfo
     */
    std::map<Looper::handler_id, HandlerInfo> mHandlers;
    Looper::handler_id mNextHandlerId;
    uint32_t mNextReplyId;
    std::condition_variable mRepliesCondition;

    /**
     * key : replyId
     * value : Message
     */
    std::map<uint32_t, std::shared_ptr<Message>> mReplies;

    // Singleton
    LooperRoster();

    ~LooperRoster();

    LooperRoster(const LooperRoster&) = delete;

    LooperRoster& operator=(const LooperRoster&) = delete;

    Result postMessage_l(const std::shared_ptr<Message>& msg, const std::chrono::system_clock::duration& delay);

    Result cancelMessage_l(const std::shared_ptr<Message>& msg);

};

} // namespace baseutils

#endif  // LOOPER_ROSTER_H_
