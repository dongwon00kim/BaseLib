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

#include <cassert>

#include <baseutils/Handler.h>
#include <baseutils/Message.h>
#include "LooperRoster.h"

using namespace std;
using namespace chrono;

namespace baseutils {

//static variable
LooperRoster* LooperRoster::mInstance = NULL;

LooperRoster* LooperRoster::getInstance()
{
    if (!mInstance) {
        mInstance = new LooperRoster();
    }
    return mInstance;
}

LooperRoster::LooperRoster()
    : mNextHandlerId(1),
      mNextReplyId(1) {
}

LooperRoster::~LooperRoster() {
}

Looper::handler_id LooperRoster::registerHandler(
        const shared_ptr<Looper> looper, const shared_ptr<Handler>& handler) {
    unique_lock<mutex> autoLock(mLock);

    if (handler->id() != 0) {
        assert(!"A handler must only be registered once.");
        return 0;
    }

    HandlerInfo info;
    info.mLooper = looper;
    info.mHandler = handler;
    Looper::handler_id handlerId = mNextHandlerId++;
    mHandlers.insert(pair<Looper::handler_id, HandlerInfo>(handlerId, info));

    handler->setID(handlerId);

    return handlerId;
}

void LooperRoster::unregisterHandler(Looper::handler_id handlerId) {
    unique_lock<mutex> autoLock(mLock);


    auto search = mHandlers.find(handlerId);
    if (search == mHandlers.end()) {
        return;
    }

    shared_ptr<Handler> handler = search->second.mHandler.lock();
    if (handler != NULL) {
        handler->setID(0);
    }

    mHandlers.erase(search);
}

Result LooperRoster::postMessage(const shared_ptr<Message>& msg, const system_clock::duration& delay) {
    unique_lock<mutex> autoLock(mLock);
    return postMessage_l(msg, delay);
}

Result LooperRoster::postMessage_l(const shared_ptr<Message>& msg, const system_clock::duration& delay) {
    auto search = mHandlers.find(msg->target());
    if (search == mHandlers.end()) {
//        ALOGW("failed to post message '%s'. Target handler not registered.",
//              msg->debugString().c_str());
        return Result::ER_NAME_NOT_FOUND;
    }

    shared_ptr<Looper> looper = search->second.mLooper.lock();
    if (NULL == looper.get()) {
//        ALOGW("failed to post message. "
//             "Target handler %d still registered, but object gone.",
//             msg->target());
        mHandlers.erase(search);
        return Result::ER_NAME_NOT_FOUND;
    }

    looper->post(msg, delay);

    return Result::OK;
}

Result LooperRoster::cancelMessage(const shared_ptr<Message>& msg) {
    unique_lock<mutex> autoLock(mLock);
    return cancelMessage_l(msg);
}


Result LooperRoster::cancelMessage_l(const shared_ptr<Message>& msg) {
    auto search = mHandlers.find(msg->target());
    if (search == mHandlers.end()) {
//        ALOGW("failed to post message '%s'. Target handler not registered.",
//              msg->debugString().c_str());
        return Result::ER_NAME_NOT_FOUND;
    }

    shared_ptr<Looper> looper = search->second.mLooper.lock();
    if (NULL == looper.get()) {
//        ALOGW("failed to post message. "
//             "Target handler %d still registered, but object gone.",
//             msg->target());
        mHandlers.erase(search);
        return Result::ER_NAME_NOT_FOUND;
    }

    return looper->cancel(msg);
}


void LooperRoster::deliverMessage(const shared_ptr<Message>& msg) {
    shared_ptr<Handler> handler;

    {
        unique_lock<mutex> autoLock(mLock);

        auto search = mHandlers.find(msg->target());
        if (mHandlers.end() == search) {
//            ALOGW("failed to deliver message. Target handler not registered.");
            return;
        }

        handler = search->second.mHandler.lock();
        if (handler == NULL) {
//            ALOGW("failed to deliver message. "
//                 "Target handler %d registered, but object gone.",
//                 msg->target());
            mHandlers.erase(search);
            return;
        }
    }

    handler->onMessageReceived(msg);
}

shared_ptr<Looper> LooperRoster::findLooper(Looper::handler_id handlerId) {
    unique_lock<mutex> autoLock(mLock);

    auto search = mHandlers.find(handlerId);
    if (search == mHandlers.end()) {
        return NULL;
    }

    shared_ptr<Looper> looper = search->second.mLooper.lock();
    if (looper == NULL) {
        mHandlers.erase(search);
        return NULL;
    }

    return looper;
}

Result LooperRoster::postAndAwaitResponse(const shared_ptr<Message>& msg, shared_ptr<Message>& response) {
    unique_lock<mutex> autoLock(mLock);

    uint32_t replyId = mNextReplyId++;

    msg->setInt32(string("replyId"), replyId);

    Result status = postMessage_l(msg, system_clock::duration(0));

    if (status != Result::OK) {
        response.reset();
        return status;
    }

    map<uint32_t, shared_ptr<Message> >::iterator itr;
    while ((itr = mReplies.find(replyId)) == mReplies.end()) {
        mRepliesCondition.wait(autoLock);
    }

    response = itr->second;
    mReplies.erase(itr);

    return Result::OK;
}

void LooperRoster::postReply(uint32_t replyId, const shared_ptr<Message>& reply) {
    unique_lock<mutex> autoLock(mLock);

    assert(mReplies.find(replyId) == mReplies.end());

    mReplies.insert(pair<uint32_t, shared_ptr<Message> >(replyId, reply));
    mRepliesCondition.notify_all();
}

} // namespace baseutils
