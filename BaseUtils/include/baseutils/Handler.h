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

#ifndef HANDLER_H_
#define HANDLER_H_

#include <baseutils/Looper.h>

namespace baseutils {

class Message;
class LooperRoster;

class Handler {
public:
    Handler() : mID(0) { }

    Looper::handler_id id() const { return mID; }

    std::shared_ptr<Looper> looper();

protected:
    virtual void onMessageReceived(const std::shared_ptr<Message>& msg) = 0;

private:
    friend class LooperRoster;

    Looper::handler_id mID;

    void setID(Looper::handler_id id) { mID = id; }

    Handler(const Handler&) = delete;

    Handler& operator=(const Handler&) = delete;
};

} // namespace baseutils

#endif  // HANDLER_H_
