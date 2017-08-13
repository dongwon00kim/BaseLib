#include <gtest/gtest.h>

#include "BaseTestThread.h"

using namespace utils;
using namespace utils::baseutils;

struct TestThreadCount {
	TestThreadCount() : mCount(0) {}
	~TestThreadCount() {}

	Result    run();
	void        requestExit() { mThread->requestExit(); }
    Result    readyToRun();
    Result    requestExitAndWait() { return mThread->requestExitAndWait(); }
    Result    join() { return mThread->join(); }
    bool        isRunning() { return mThread->isRunning(); }

	bool threadLoop() { ++mCount; usleep(100*1000); if(mCount==10) return false; else return true; }
	int  getCount()   { return mCount; }
private:
	int mCount;
	shared_ptr<BaseThread> mThread;
};

Result
TestThreadCount::readyToRun() {
	if(mThread == nullptr)
		mThread = make_shared<BaseTestThread<TestThreadCount>>(this);
	return mThread->readyToRun();
}


Result
TestThreadCount::run() {
	if(mThread == nullptr)
		mThread = make_shared<BaseTestThread<TestThreadCount>>(this);
	Result err = mThread->run();
	if (err != Result::OK) {
		mThread.reset();
	}
	return err;
}

//////////////////////////////////////////////////////////////////

TEST(ThreadTest, TestThreadCount)
{
	TestThreadCount threadCount;
	threadCount.run();
	usleep(1000);
	threadCount.join();
	EXPECT_EQ(threadCount.getCount(), 10);
}

TEST(ThreadTest, TestThreadCountGt)
{
	TestThreadCount threadCount;
	threadCount.run();
	usleep(1000);
	threadCount.requestExitAndWait();
	EXPECT_GT(threadCount.getCount(), 0);
}

TEST(ThreadTest, TestThreadCountRunning)
{
	TestThreadCount threadCount;
	threadCount.run();
	EXPECT_EQ(threadCount.isRunning(), true);
	threadCount.requestExitAndWait();
}

TEST(ThreadTest, TestThreadCountReady)
{
	TestThreadCount threadCount;
	EXPECT_EQ(threadCount.readyToRun(), Result::OK);
	threadCount.run();
	threadCount.requestExitAndWait();
}

