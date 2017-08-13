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

/** @file Message.h
 *
 *  Brief description.
 *
 *  @author            Dongwon, Kim (dongwon00.kim@gmail.com)
 *  @version           1.0
 *  @date              2016.05.11
 *  @note
 *  @see
 */


#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <string>
#include <memory>
#include <chrono>
#include <vector>

#include <Looper.h>

using namespace std;

namespace utils {
namespace baseutils {

class Buffer;
class SendableObject;

/**
 *  @class Message
 *  @brief Message for looper and handler
  */
class Message : virtual public enable_shared_from_this<Message>  {
public:
    Message(const Looper::handler_id target = 0);
    Message(const Looper::handler_id target, const uint32_t what);

    virtual ~Message();

    void setWhat(const uint32_t what);
    uint32_t what() const;

    void setTarget(const Looper::handler_id target);
    Looper::handler_id target() const;

    void clear();

    void setBoolean(const string& name, bool value);
    void setInt32(const string& name, int32_t value);
    void setInt64(const string& name, int64_t value);
    void setSize(const string& name, size_t value);
    void setFloat(const string& name, float value);
    void setDouble(const string& name, double value);
    void setPointer(const string& name, void* value);
    void setString(const string& name, const string str);
    void setBuffer(const string& name, const shared_ptr<Buffer>& buffer);
    void setMessage(const string& name, const shared_ptr<Message>& message);
    void setObject(const string& name, const shared_ptr<SendableObject>& object);
    bool findBoolean(const string& name, bool& value) const;
    bool findInt32(const string& name, int32_t& value) const;
    bool findInt64(const string& name, int64_t& value) const;
    bool findSize(const string& name, size_t& value) const;
    bool findFloat(const string& name, float& value) const;
    bool findDouble(const string& name, double& value) const;
    bool findPointer(const string& name, void*& value) const;
    bool findString(const string& name, string& value) const;
    bool findBuffer(const string& name, shared_ptr<Buffer>& buffer) const;
    bool findMessage(const string& name, shared_ptr<Message>& message) const;
    bool findObject(const string& name, shared_ptr<SendableObject>& obj) const;

    Result post(const int64_t delayUs = 0);

    template<typename T>
    Result post(const T& delay) {
        return postMessage(chrono::duration_cast<chrono::system_clock::duration>(delay));
    }

    Result cancel();

    // Posts the message to its target and waits for a response (or error)
    // before returning.
    Result postAndAwaitResponse(shared_ptr<Message>& response);

    // If this returns true, the sender of this message is synchronously
    // awaiting a response, the "replyID" can be used to send the response
    // via "postReply" below.
    bool senderAwaitsResponse(uint32_t& replyID) const;

    void postReply(uint32_t replyID);

    // Performs a deep-copy of "this", contained messages are in turn "dup'ed".
    shared_ptr<Message> duplicate() const;

    string debugString(int32_t indent = 0) const;

    enum Type {
        kTypeBoolean,
        kTypeInt32,
        kTypeInt64,
        kTypeSize,
        kTypeFloat,
        kTypeDouble,
        kTypePointer,
        kTypeString,
        kTypeMessage,
        kTypeBuffer,
        kTypeObject,
        kTypeUnknown,
    };

    size_t countEntries() const;
    const string getEntryNameAt(const size_t index, Type& type) const;

private:
    uint32_t mWhat;
    Looper::handler_id mTarget;

    class Item {
    public:
        union {
            bool boolValue;
            int32_t int32Value;
            int64_t int64Value;
            size_t sizeValue;
            float floatValue;
            double doubleValue;
            void *ptrValue;
        } value;
        string stringValue;
        shared_ptr<Message> messagePtr;
        shared_ptr<Buffer> bufferPtr;
        shared_ptr<SendableObject> objectPtr;

        string mName;
        Type mType;

        Item() : mType(kTypeUnknown) {}
        ~Item() = default;
    };

    enum {
        kMaxNumItems = 64
    };

    vector<shared_ptr<Item>> mItems;

    void clearItem(shared_ptr<Item>& item);

    shared_ptr<Item> allocateItem(const string& name);

    const shared_ptr<Item> findItem(const string& name, Type type) const;

    Result postMessage(const chrono::system_clock::duration& delay);

    DISALLOW_EVIL_CONSTRUCTORS(Message);
};

}; // namespace baseutils
}; // namespace utils

#endif  // _MESSAGE_H_
