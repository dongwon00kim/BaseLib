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

/** @file Handler.h
 *
 *  Brief description.
 *
 *  @author            Dongwon, Kim (dongwon00.kim@gmail.com)
 *  @version           1.0
 *  @date              2016.05.11
 *  @note
 *  @see
 */

#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <Base.h>
#include <Looper.h>
#include <LooperRoster.h>

namespace utils {
namespace baseutils {

class Message;

class Handler {
public:
    Handler() : mID(0) { }

    Looper::handler_id id() const { return mID; }

    shared_ptr<Looper> looper() { return LooperRoster::getInstance()->findLooper(id()); }

protected:
    virtual void onMessageReceived(const shared_ptr<Message>& msg) = 0;

private:
    friend class LooperRoster;

    Looper::handler_id mID;

    void setID(Looper::handler_id id) { mID = id; }

    DISALLOW_EVIL_CONSTRUCTORS(Handler);
};

}; // namespace baseutils
}; // namespace utils

#endif  // _HANDLER_H_
