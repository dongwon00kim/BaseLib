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

#include <cstdio>
#include <cctype>
#include <cassert>
#include <cstring>
#include <cstdarg>
#include <string>
#include <memory>

#include <baseutils/Buffer.h>
#include <baseutils/Message.h>
#include <baseutils/Parcelable.h>
#include "LooperRoster.h"

using namespace std;
using namespace chrono;

namespace baseutils {

Message::Message(const Looper::handler_id target)
    : mWhat(0),
      mTarget(target) {
}

Message::Message(const Looper::handler_id target, const uint32_t what)
    : mWhat(what),
      mTarget(target) {
}

Message::~Message() {
    clear();
}


void Message::setWhat(const uint32_t what) {
    mWhat = what;
}

uint32_t Message::what() const {
    return mWhat;
}

void Message::setTarget(const Looper::handler_id handlerID) {
    mTarget = handlerID;
}

Looper::handler_id Message::target() const {
    return mTarget;
}

void Message::clear() {
    mItems.clear();
}

void Message::clearItem(shared_ptr<Item>& item) {
    switch (item->mType) {
        case kTypeString:
            item->stringValue.clear();
            break;
        case kTypeMessage:
            if (item->messagePtr != nullptr) {
                item->messagePtr.reset();
            }
            break;
        case kTypeBuffer:
            if (item->bufferPtr != nullptr) {
                item->bufferPtr.reset();
            }
        case kTypeObject:
            if(item->objectPtr != nullptr) {
                item->objectPtr.reset();
            }
            break;
        default:
            break;
    }
}

shared_ptr<Message::Item> Message::allocateItem(const string& name) {
    bool found = false;
    shared_ptr<Item> item;
    for(auto& temp : mItems) {
        if(!temp->mName.compare(name)) {
            clearItem(temp);
            item = temp;
            found = true;
            break;
        }
    }
    if(!found) {
        item = make_shared<Item>();
        item->mName = name;
        mItems.push_back(item);
    }
    return item;
}

const shared_ptr<Message::Item> Message::findItem(const string& name, Type type) const {
    shared_ptr<Item> item;
    for(auto& temp : mItems) {
        if(temp->mType == type && !temp->mName.compare(name)) {
            item = temp;
            break;
        }
    }
    return item;
}

#define BASIC_TYPE(NAME,FIELDNAME,TYPENAME)                                 \
void Message::set##NAME(const string& name, TYPENAME value) {         \
    auto item(allocateItem(name));                                          \
    item->mType = kType##NAME;                                              \
    item->value.FIELDNAME = value;                                          \
}                                                                           \
                                                                            \
bool Message::find##NAME(const string& name, TYPENAME* const value) const { \
    auto item(findItem(name, kType##NAME));                                 \
    if (item) {                                                             \
        (*value) = item->value.FIELDNAME;                                   \
        return true;                                                        \
    }                                                                       \
    return false;                                                           \
}

BASIC_TYPE(Boolean,boolValue,bool)
BASIC_TYPE(Int32,int32Value,int32_t)
BASIC_TYPE(Int64,int64Value,int64_t)
BASIC_TYPE(Size,sizeValue,size_t)
BASIC_TYPE(Float,floatValue,float)
BASIC_TYPE(Double,doubleValue,double)
BASIC_TYPE(Pointer,ptrValue,void*)

#undef BASIC_TYPE

void Message::setString(const string& name, const string& str) {
    auto item(allocateItem(name));
    item->mType = kTypeString;
    item->stringValue = str;
}

void Message::setBuffer(const string& name, const shared_ptr<Buffer>& buffer) {
    auto item(allocateItem(name));
    item->mType = kTypeBuffer;
    item->bufferPtr = buffer;
}

void Message::setMessage(const string& name, const shared_ptr<Message>& message) {
    auto item(allocateItem(name));
    item->mType = kTypeMessage;
    item->messagePtr = message;
}

void Message::setObject(const string& name, const shared_ptr<Parcelable>& object) {
    auto item(allocateItem(name));
    item->mType = kTypeObject;
    item->objectPtr = object;
}

bool Message::findString(const string& name, string* const value) const {
    auto item(findItem(name, kTypeString));
    if (item) {
        (*value) = item->stringValue;
        return true;
    }
    return false;
}

bool Message::findBuffer(const string& name, shared_ptr<Buffer>* const buf) const {
    auto item(findItem(name, kTypeBuffer));
    if (item) {
        (*buf) = item->bufferPtr;
        return true;
    }
    return false;
}

bool Message::findMessage(const string& name, shared_ptr<Message>* const message) const {
    auto item(findItem(name, kTypeMessage));
    if (item) {
        (*message) = item->messagePtr;
        return true;
    }
    return false;
}

bool Message::findObject(const string& name, shared_ptr<Parcelable>* const object) const {
    auto item(findItem(name, kTypeObject));
    if (item) {
        (*object) = item->objectPtr;
        return true;
    }
    return false;
}

Result Message::post(const int64_t delayUs) {
    microseconds delay(delayUs);
	return LooperRoster::getInstance()->postMessage(shared_from_this(), duration_cast<system_clock::duration>(delay));
}

Result Message::cancel() {
    return LooperRoster::getInstance()->cancelMessage(shared_from_this());
}

Result Message::postAndAwaitResponse(shared_ptr<Message>& response) {
    return LooperRoster::getInstance()->postAndAwaitResponse(shared_from_this(), response);
}

void Message::postReply(const uint32_t replyId) {
    LooperRoster::getInstance()->postReply(replyId, shared_from_this());
}

bool Message::senderAwaitsResponse(uint32_t& replyId) const {
    int32_t tmp;
    bool found = findInt32("replyId", &tmp);
    if (!found) {
        return false;
    }

    replyId = static_cast<uint32_t>(tmp);
    return true;
}

shared_ptr<Message> Message::duplicate() const {
    auto msg(make_shared<Message>(mTarget, mWhat));

    for (auto& from : mItems) {
        auto to(make_shared<Item>());
        to->mName = from->mName;
        to->mType = from->mType;

        switch (from->mType) {
            case kTypeString:
                to->stringValue = from->stringValue;
                break;
            case kTypeBuffer:
                to->bufferPtr = from->bufferPtr;
                break;
            case kTypeMessage:
                to->messagePtr = from->messagePtr;
                break;
            default:
                to->value = from->value;
                break;
        }

        msg->mItems.push_back(to);
    }

    return msg;
}

static void appendIndent(string* str, int32_t indent) {
    static const char kWhitespace[] =
        "                                        "
        "                                        ";

    assert((size_t)indent < sizeof(kWhitespace));

    str->append(kWhitespace, indent);
}

static bool isFourcc(uint32_t what) {
    return isprint(what & 0xff)
        && isprint((what >> 8) & 0xff)
        && isprint((what >> 16) & 0xff)
        && isprint((what >> 24) & 0xff);
}

static string stringPrintf(const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    char *buffer;
    vasprintf(&buffer, format, ap);

    va_end(ap);

    string result(buffer);

    free(buffer);
    buffer = nullptr;

    return result;
}


static void hexDump(const void* _data, size_t size, size_t indent, string* appendTo) {
    const uint8_t *data = (const uint8_t*)_data;

    size_t offset = 0;
    while (offset < size) {
        string line;

        appendIndent(&line, indent);

        char tmp[32];
        sprintf(tmp, "%08lx:  ", (unsigned long)offset);

        line.append(tmp);

        for (size_t i = 0; i < 16; ++i) {
            if (i == 8) {
                line.push_back(' ');
            }
            if (offset + i >= size) {
                line.append("   ");
            } else {
                sprintf(tmp, "%02x ", data[offset + i]);
                line.append(tmp);
            }
        }

        line.push_back(' ');

        for (size_t i = 0; i < 16; ++i) {
            if (offset + i >= size) {
                break;
            }

            if (isprint(data[offset + i])) {
                line.push_back((char)data[offset + i]);
            } else {
                line.push_back('.');
            }
        }

        if (appendTo != nullptr) {
            appendTo->append(line);
            appendTo->append("\n");
        } else {
            //ALOGI("%s", line.c_str());
        }

        offset += 16;
    }
}

string Message::debugString(int32_t indent) const {
    string s = "Message(what = ";

    string tmp;
    if (isFourcc(mWhat)) {
        tmp = stringPrintf(
                "'%c%c%c%c'",
                (char)(mWhat >> 24),
                (char)((mWhat >> 16) & 0xff),
                (char)((mWhat >> 8) & 0xff),
                (char)(mWhat & 0xff));
    } else {
        tmp = stringPrintf("0x%08x", mWhat);
    }
    s.append(tmp);

    if (mTarget != 0) {
        tmp = stringPrintf(", target = %d", mTarget);
        s.append(tmp);
    }
    s.append(") = {\n");

    for (auto& item : mItems) {
        switch (item->mType) {
            case kTypeInt32:
                tmp = stringPrintf(
                        "int32_t %s = %d", item->mName.c_str(), item->value.int32Value);
                break;
            case kTypeInt64:
                tmp = stringPrintf(
                        "int64_t %s = %lld", item->mName.c_str(), item->value.int64Value);
                break;
            case kTypeSize:
                tmp = stringPrintf(
                        "size_t %s = %d", item->mName.c_str(), item->value.sizeValue);
                break;
            case kTypeFloat:
                tmp = stringPrintf(
                        "float %s = %f", item->mName.c_str(), item->value.floatValue);
                break;
            case kTypeDouble:
                tmp = stringPrintf(
                        "double %s = %f", item->mName.c_str(), item->value.doubleValue);
                break;
            case kTypePointer:
                tmp = stringPrintf(
                        "void *%s = %p", item->mName.c_str(), item->value.ptrValue);
                break;
            case kTypeString:
                tmp = stringPrintf(
                        "string %s = \"%s\"",
                        item->mName.c_str(),
                        item->stringValue.c_str());
                break;
            case kTypeBuffer:
                if (item->bufferPtr != nullptr && item->bufferPtr->size() <= 64) {
                    tmp = stringPrintf("Buffer %s = {\n", item->mName.c_str());
                    hexDump(item->bufferPtr->data(), item->bufferPtr->size(), indent + 4, &tmp);
                    appendIndent(&tmp, indent + 2);
                    tmp.append("}");
                } else {
                    tmp = stringPrintf("Buffer *%s = %p", item->mName.c_str(), item->bufferPtr.get());
                }
                break;
            case kTypeMessage:
                tmp = stringPrintf(
                        "Message %s = %s",
                        item->mName.c_str(), item->messagePtr->debugString(indent + item->mName.size() + 14).c_str());
                break;
            default:
                break;
        }

        appendIndent(&s, indent);
        s.append("  ");
        s.append(tmp);
        s.append("\n");
    }

    appendIndent(&s, indent);
    s.append("}");

    return s;
}

size_t Message::countEntries() const {
    return mItems.size();
}

const string Message::getEntryNameAt(const size_t index, Type& type) const {
    if (index >= mItems.size()) {
        type = kTypeUnknown;
        return string("Unknown");
    }

    type = mItems[index]->mType;
    return mItems[index]->mName;
}

Result Message::postMessage(const chrono::system_clock::duration& delay) {
    return LooperRoster::getInstance()->postMessage(shared_from_this(), delay);
}

} // namespace baseutils
