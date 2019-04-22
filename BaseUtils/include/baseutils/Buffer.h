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

#ifndef BUFFER_H_
#define BUFFER_H_

//#include <sys/types.h>
//#include <stdint.h>
#include <memory>
#include <baseutils/Message.h>

namespace baseutils {

class Message;

class Buffer{
public:
    Buffer(const size_t capacity);

    Buffer(const void* data, const size_t capacity);

    virtual ~Buffer();

    void setFarewellMessage(const std::shared_ptr<Message>& msg);

    uint8_t* base() { return (uint8_t*)mData; }

    uint8_t* data() { return (uint8_t*)mData + mRangeOffset; }

    const size_t capacity() const { return mCapacity; }

    const size_t size() const { return mRangeLength; }

    const size_t offset() const { return mRangeOffset; }

    void setRange(const size_t offset, const size_t size);

    void setInt32Data(const int32_t data) { mInt32Data = data; }

    const int32_t int32Data() const { return mInt32Data; }

    std::shared_ptr<Message> meta();

private:
    std::shared_ptr<Message> mFarewell;
    std::shared_ptr<Message> mMeta;

    void *mData;
    size_t mCapacity;
    size_t mRangeOffset;
    size_t mRangeLength;
    int32_t mInt32Data;
};

} // namespace baseutils

#endif  // BUFFER_H_
