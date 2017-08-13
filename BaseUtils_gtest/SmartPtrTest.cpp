#include <gtest/gtest.h>
#include <iostream>
#include <memory>

using namespace std;

class TestPtr {
public:
	TestPtr() { printf("TestPtr() Constructor\n"); }
	~TestPtr() { printf("TestPtr() Destructor\n"); }
};

TEST(SmartPtrTest, SharedPtr) {
	{
		shared_ptr<TestPtr> ptr1(new TestPtr);
		cout << "ptr1(new TestPtr" << endl;
		{
			shared_ptr<TestPtr> ptr2 = ptr1;
			cout << "ptr2 = ptr1" << endl;
			weak_ptr<TestPtr> wp = ptr2;
			cout << "wp = ptr2" << endl;
			shared_ptr<TestPtr> ptr3 = ptr1;
			ptr2.reset();
			ptr3 = ptr2;
			cout << "ptr3 = ptr2" << endl;
		}

	}
	EXPECT_EQ(true, true);
}


TEST(SmartPtrTest, WeakPtr)
{
    shared_ptr<int> sp1(new int(5));

    weak_ptr<int> wp1 = sp1;
    {
        shared_ptr<int> sp2 = wp1.lock();
        if (sp2) {
            EXPECT_NE(nullptr, sp2);
        }
    }
    sp1.reset();

    shared_ptr<int> sp3 = wp1.lock();

    bool result = (sp3 == NULL);
   	EXPECT_EQ(true, result);
}
