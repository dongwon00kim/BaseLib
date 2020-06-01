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
#include <string.h>
#include <baseutils/Buffer.h>
#include <baseutils/Looper.h>
#include <baseutils/Message.h>

using namespace std;

namespace baseutils {

Buffer::Buffer(const size_t capacity) :
        mData(malloc(capacity)),
        mCapacity(capacity),
        mRangeOffset(0),
        mRangeLength(0),
        mInt32Data(0) {
}

Buffer::Buffer(const void* data, const size_t capacity) :
        mData(malloc(capacity)),
        mCapacity(capacity),
        mRangeOffset(0),
        mRangeLength(capacity),
        mInt32Data(0) {
    memcpy(mData, data, capacity);
}

Buffer::~Buffer() {
    if (mData != NULL) {
        free(mData);
        mData = NULL;
    }

    if (mFarewell != NULL) {
        mFarewell->post();
    }
}

void Buffer::consume(const size_t size) {
    assert(size <= mRangeLength);
    mRangeOffset += size;
    mRangeLength -= size;
    if (mRangeLength <= 0) {
        mRangeLength = 0;
        mRangeOffset = 0;
    }
}

void Buffer::setRange(const size_t offset, const size_t size) {
    assert(offset <= mCapacity);
    assert(offset + size <= mCapacity);

    mRangeOffset = (size == 0) ? 0 : offset;
    mRangeLength = size;
}

void Buffer::setSize(const size_t size) {
    assert(size <= mCapacity);
    mRangeOffset = 0;
    mRangeLength = size;
}


void Buffer::setFarewellMessage(const shared_ptr<Message>& msg) {
    mFarewell = msg;
}

shared_ptr<Message> Buffer::meta() {
    if (mMeta == NULL) {
        mMeta = make_shared<Message>();
    }
    return mMeta;
}

bool
Buffer::operator==(const Buffer &other) const {
    bool same = false;
    if (mData == other.mData && mCapacity == other.mCapacity && mRangeOffset == other.mRangeOffset && mRangeLength == other.mRangeLength) {
        same = true;
    }
    return same;
}

bool
Buffer::operator!=(const Buffer &other) const {
    return !(*this == other);
}

} // namespace baseutils
