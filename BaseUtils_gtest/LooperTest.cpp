
#include <gtest/gtest.h>
#include <Looper.h>
#include <Handler.h>
#include <Message.h>
#include <Buffer.h>
#include <unistd.h>
#include <chrono>

#include "BaseTestThread.h"

using namespace std;
using namespace std::chrono;
using namespace utils;
using namespace utils::baseutils;

#define TEST_MESSAGE_A_INT_DATA		100
#define TEST_MESSAGE_A_DOUBLE_DATA	3.141592

class MessageReceiver : public Handler {
public:
    enum {
        kWhatMessage,
        kWhatEvent,
    };

    enum MessageType {
        kMessageA,
        kMessageB,
        kMessageC,
    };

    enum EventType {
        kEventStart,
        kEventStop,
        kEventPause,
        kEventResume,
        kEventUnsupported,
        kEventError,
        kEventEnd,
    };

    MessageReceiver();

    virtual ~MessageReceiver() = default;

    bool checkEndReceived() { return mEventEndReceived; }

	static const string kKeyMessageType;
	static const string kKeyEventType;

protected:
    virtual void onMessageReceived(const shared_ptr<Message> &msg);

private:
    bool mEventEndReceived;
};

const string MessageReceiver::kKeyMessageType			= "message-type";
const string MessageReceiver::kKeyEventType			= "event-type";

MessageReceiver::MessageReceiver()
	: mEventEndReceived(false) {
}

void
MessageReceiver::onMessageReceived(const shared_ptr<Message> &msg) {
	switch (msg->what()) {
		case kWhatMessage:
		{
			int32_t msgType;
			if(msg->findInt32(kKeyMessageType, msgType)) {
				switch (msgType) {
					case kMessageA: {
						int32_t intData;
						ASSERT_TRUE(msg->findInt32(string("int-data"), intData));
						ASSERT_EQ(TEST_MESSAGE_A_INT_DATA, intData);
						double doubleData;
						ASSERT_TRUE(msg->findDouble(string("double-data"), doubleData));
						ASSERT_EQ(TEST_MESSAGE_A_DOUBLE_DATA, doubleData);
						break;
					}
					case kMessageB: {
						shared_ptr<Buffer> buff;
						ASSERT_TRUE(msg->findBuffer(string("buffer"), buff));
						ASSERT_TRUE(nullptr != buff->data());
						ASSERT_EQ(100, buff->size());
						ASSERT_EQ(256, buff->capacity());
						break;
					}
					case kMessageC: {
						break;
					}
					default : {
						break;
					}
				}
			}
			break;
		}
		case kWhatEvent:
		{
			int32_t eventType;
			if(msg->findInt32(kKeyEventType, eventType)) {
				switch (eventType) {
					case kEventStart:
						mEventEndReceived = false;
						break;
					case kEventStop:
						break;
					case kEventPause:
						break;
					case kEventResume:
						break;
					case kEventUnsupported:
						break;
					case kEventError:
						break;
					case kEventEnd:
						mEventEndReceived = true;
						break;
					default :
						break;
				}
			}
			break;
		}
		default:
			break;
	}
}

struct MessageSenderThread {
	MessageSenderThread(shared_ptr<MessageReceiver> &receiver);
	~MessageSenderThread() {}

	Result    run();
	void        requestExit() { mThread->requestExit(); }
    Result    readyToRun();
    Result    requestExitAndWait() { return mThread->requestExitAndWait(); }
    Result    join() { return mThread->join(); }
    bool        isRunning() { return mThread->isRunning(); }

	bool        threadLoop();
	int         getCount()   { return mLoopCount; }
private:
	shared_ptr<BaseThread> mThread;
	shared_ptr<Message> mMessage;
	shared_ptr<Message> mEvent;
	int mLoopCount;
};


MessageSenderThread::MessageSenderThread(shared_ptr<MessageReceiver>& receiver)
	: mMessage(make_shared<Message>(receiver->id(), MessageReceiver::kWhatMessage)),
	  mEvent(make_shared<Message>(receiver->id(), MessageReceiver::kWhatEvent)),
	  mLoopCount(0) {
}

Result
MessageSenderThread::readyToRun() {
	if(mThread == nullptr)
		mThread = make_shared<BaseTestThread<MessageSenderThread>>(this);
	return mThread->readyToRun();
}

Result
MessageSenderThread::run() {
	if(mThread == nullptr)
	    mThread = make_shared<BaseTestThread<MessageSenderThread>>(this);
	    Result err = mThread->run();
	if (err != Result::OK) {
        mThread.reset();
	}
	return err;
}

bool
MessageSenderThread::threadLoop() {
	//ASSERT_TRUE(mMessage == nullptr);

	if(mLoopCount == 0) {
		shared_ptr<Message> evt = mEvent->duplicate();
		evt->setInt32(MessageReceiver::kKeyEventType, MessageReceiver::kEventStart);
		evt->post();
	}

	shared_ptr<Message> msgA = mMessage->duplicate();
	msgA->setInt32(MessageReceiver::kKeyMessageType, MessageReceiver::kMessageA);
	msgA->setInt32("int-data", TEST_MESSAGE_A_INT_DATA);
	msgA->setDouble("double-data", TEST_MESSAGE_A_DOUBLE_DATA);
	msgA->post();

	auto buff(make_shared<Buffer>(256));
	uint8_t *data = buff->data();
	for(int i=0; i<100; i++)
		data[i] = mLoopCount*100 + i;
	buff->setRange(0, 100);

	shared_ptr<Message> msgB = mMessage->duplicate();
	msgB->setInt32(MessageReceiver::kKeyMessageType, MessageReceiver::kMessageB);
	msgB->setBuffer("buffer", buff);
	EXPECT_EQ(Result::OK, msgB->post());

	++mLoopCount;
	if(mLoopCount == 3) {
		shared_ptr<Message> evt = mEvent->duplicate();
		evt->setInt32(MessageReceiver::kKeyEventType, MessageReceiver::kEventEnd);
		evt->post();

		mLoopCount = 0;
		return false;
	}

	this_thread::sleep_for(milliseconds(100));

	return true;
}


TEST(LooperTest, LooperReceiveEndEvent) {
	auto looper(make_shared<Looper>());
    looper->setName("LooperTest");

    // In order to get handler id from looper, should register handler before creating message.
	auto receiver(make_shared<MessageReceiver>());
    auto id = looper->registerHandler(receiver);
    ASSERT_EQ(id, receiver->id());
    ASSERT_GT(receiver->id(), 0);
    ASSERT_EQ(Result::OK, looper->start());

	auto sender(make_shared<MessageSenderThread>(receiver));
    sender->run();

    while (!receiver->checkEndReceived()) {
        this_thread::sleep_for(milliseconds(100));
    }

    ASSERT_EQ(true, receiver->checkEndReceived());
}

TEST(LooperTest, DelayedPost) {
    auto looper(make_shared<Looper>());
    looper->setName("LooperTest");

    // In order to get handler id from looper, should register handler before creating message.
    auto receiver(make_shared<MessageReceiver>());
    auto id = looper->registerHandler(receiver);
    ASSERT_EQ(id, receiver->id());
    ASSERT_GT(receiver->id(), 0);
    ASSERT_EQ(Result::OK, looper->start());

    auto msg(make_shared<Message>(receiver->id(), MessageReceiver::kWhatEvent));
    msg->setInt32(MessageReceiver::kKeyEventType, MessageReceiver::kEventEnd);
    ASSERT_EQ(Result::OK, msg->post(milliseconds(10)));

    while (!receiver->checkEndReceived()) {
        this_thread::sleep_for(milliseconds(10));
    }

    ASSERT_EQ(true, receiver->checkEndReceived());
}
