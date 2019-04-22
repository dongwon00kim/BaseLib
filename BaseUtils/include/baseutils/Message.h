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

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <chrono>
#include <string>
#include <memory>
#include <vector>
#include <baseutils/Looper.h>

namespace baseutils {

class Buffer;
class Parcelable;

/**
 *  @class Message
 *  @brief Message for looper and handler
  */
class Message : virtual public std::enable_shared_from_this<Message>  {
public:
    Message(const Looper::handler_id target = 0);
    Message(const Looper::handler_id target, const uint32_t what);

    virtual ~Message();

    void setWhat(const uint32_t what);
    uint32_t what() const;

    void setTarget(const Looper::handler_id target);
    Looper::handler_id target() const;

    void clear();

    void setBoolean(const std::string& name, bool value);
    void setInt32(const std::string& name, int32_t value);
    void setInt64(const std::string& name, int64_t value);
    void setSize(const std::string& name, size_t value);
    void setFloat(const std::string& name, float value);
    void setDouble(const std::string& name, double value);
    void setPointer(const std::string& name, void* value);
    void setString(const std::string& name, const std::string& str);
    void setBuffer(const std::string& name, const std::shared_ptr<Buffer>& buffer);
    void setMessage(const std::string& name, const std::shared_ptr<Message>& message);
    void setObject(const std::string& name, const std::shared_ptr<Parcelable>& object);
    bool findBoolean(const std::string& name, bool* const value) const;
    bool findInt32(const std::string& name, int32_t* const value) const;
    bool findInt64(const std::string& name, int64_t* const value) const;
    bool findSize(const std::string& name, size_t* const value) const;
    bool findFloat(const std::string& name, float* const value) const;
    bool findDouble(const std::string& name, double* const value) const;
    bool findPointer(const std::string& name, void** const value) const;
    bool findString(const std::string& name, std::string* const value) const;
    bool findBuffer(const std::string& name, std::shared_ptr<Buffer>* const buffer) const;
    bool findMessage(const std::string& name, std::shared_ptr<Message>* const message) const;
    bool findObject(const std::string& name, std::shared_ptr<Parcelable>* const obj) const;

    Result post(const int64_t delayUs = 0);

    template<typename T>
    Result post(const T& delay) {
        return postMessage(std::chrono::duration_cast<std::chrono::system_clock::duration>(delay));
    }

    Result cancel();

    // Posts the message to its target and waits for a response (or error)
    // before returning.
    Result postAndAwaitResponse(std::shared_ptr<Message>& response);

    // If this returns true, the sender of this message is synchronously
    // awaiting a response, the "replyID" can be used to send the response
    // via "postReply" below.
    bool senderAwaitsResponse(uint32_t& replyID) const;

    void postReply(uint32_t replyID);

    // Performs a deep-copy of "this", contained messages are in turn "dup'ed".
    std::shared_ptr<Message> duplicate() const;

    std::string debugString(int32_t indent = 0) const;

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
    const std::string getEntryNameAt(const size_t index, Type& type) const;

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
            void* ptrValue;
        } value;
        std::string stringValue;
        std::shared_ptr<Message> messagePtr;
        std::shared_ptr<Buffer> bufferPtr;
        std::shared_ptr<Parcelable> objectPtr;

        std::string mName;
        Type mType;

        Item() : mType(kTypeUnknown) {}
        ~Item() = default;
    };

    enum {
        kMaxNumItems = 64
    };

    std::vector<std::shared_ptr<Item>> mItems;

    Message(const Message&) = delete;

    Message& operator=(const Message&) = delete;

    void clearItem(std::shared_ptr<Item>& item);

    std::shared_ptr<Item> allocateItem(const std::string& name);

    const std::shared_ptr<Item> findItem(const std::string& name, Type type) const;

    Result postMessage(const std::chrono::system_clock::duration& delay);
};

} // namespace baseutils

#endif  // MESSAGE_H_
