#include "mg/common/Assert.h"
#include "mg/common/MultiConsumerQueue.h"
#include "mg/common/QPTimer.h"
#include "mg/common/Thread.h"

#include "UnitTest.h"

namespace mg {
namespace unittests {

	struct UTMCQValue
	{
		UTMCQValue(
			int aValue)
			: myValue(aValue)
		{
		}

		UTMCQValue()
			: myValue(-1)
		{
		}

		~UTMCQValue()
		{
			myValue = -2;
		}

		int myValue;
		UTMCQValue* myNext;
	};

	// Multi-Consumer-Multi-Producer queue, but with a mutex lock
	// on each push/pop. The queue is used to compare its speed
	// with the Multi-Consumer queue from mg.
	// This implementation is the slowest possible, and anything
	// else shall be faster. Otherwise something is very wrong.
	//
	class UTMCQueueLocked
	{
	public:
		UTMCQueueLocked()
			: myHead(nullptr)
			, myTail(nullptr)
		{
		}

		bool
		Push(
			UTMCQValue* aObject)
		{
			myLock.Lock();
			bool wasEmpty = myHead == nullptr;
			if (wasEmpty)
				myHead = aObject;
			else
				myTail->myNext = aObject;
			myTail = aObject;
			aObject->myNext = nullptr;
			myLock.Unlock();
			return wasEmpty;
		}

		UTMCQValue*
		Pop()
		{
			myLock.Lock();
			UTMCQValue* res = myHead;
			if (res != nullptr)
				myHead = myHead->myNext;
			myLock.Unlock();
			return res;
		}

	private:
		mg::common::Mutex myLock;
		UTMCQValue* myHead;
		UTMCQValue* myTail;
	};

	using UTMCQueue = mg::common::MultiConsumerQueue<UTMCQValue>;
	using UTMCQueueConsumer = mg::common::MultiConsumerQueueConsumer<UTMCQValue>;

	static void
	UnitTestMCQLockedBasic()
	{
		const char* testName = "MCQLockedBasic";
		UTMCQueueLocked queue;

		UTMCQValue v1;
		UTMCQValue v2;
		UTMCQValue v3;

		MG_COMMON_ASSERT_F(queue.Pop() == nullptr, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(queue.Push(&v1), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == nullptr, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(queue.Push(&v1), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(!queue.Push(&v2), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == nullptr, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(queue.Push(&v1), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(!queue.Push(&v2), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(!queue.Push(&v3), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == &v3, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == nullptr, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(queue.Push(&v1), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(!queue.Push(&v2), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(!queue.Push(&v3), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == &v3, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Pop() == nullptr, "(Failed)%s]]", testName);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestMCQBasic()
	{
		const char* testName = "MCQBasic";
		UTMCQueue queue(5);
		MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.ConsumerCount() == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.SubQueueCount() == 1, "(Failed)%s]]", testName);

		{
			// Ensure empty consumer is destroyed fine.
			UTMCQueueConsumer c2;
		}

		UTMCQueueConsumer c1;
		c1.Attach(&queue);
		MG_COMMON_ASSERT_F(queue.ConsumerCount() == 1, "(Failed)%s]]", testName);

		UTMCQValue v1;
		UTMCQValue v2;
		UTMCQValue v3;

		MG_COMMON_ASSERT_F(c1.Pop() == nullptr, "(Failed)%s]]", testName);

		// Push-pop single.
		MG_COMMON_ASSERT_F(queue.Push(&v1), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == nullptr, "(Failed)%s]]", testName);

		// Push-pop 2 elements.
		MG_COMMON_ASSERT_F(queue.Push(&v1), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(!queue.Push(&v2), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == nullptr, "(Failed)%s]]", testName);

		// Push-pop 3 elements. Number of sub-queues becomes 2,
		// because the first one was of size 5, and next 2 pushes
		// fill it.
		MG_COMMON_ASSERT_F(queue.Push(&v1), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(!queue.Push(&v2), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.SubQueueCount() == 1, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(!queue.Push(&v3), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 3, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.SubQueueCount() == 2, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == &v3, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.SubQueueCount() == 2, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == nullptr, "(Failed)%s]]", testName);

		// Ensure the sub-queues are recycled. Push-pop 5 elements
		// 2 times - the sub-queue count does not change.
		const int count = 5;
		UTMCQValue vs[count];
		for (int i = 0; i < 2; ++i)
		{
			for (int j = 0; j < count; ++j)
				queue.Push(&vs[j]);
			MG_COMMON_ASSERT_F(queue.Count() == count, "(Failed)%s]]", testName);
			for (int j = 0; j < count; ++j)
				MG_COMMON_ASSERT_F(c1.Pop() == &vs[j], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(queue.SubQueueCount() == 2, "(Failed)%s]]", testName);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestMCQPending()
	{
		const char* testName = "MCQPending";
		UTMCQValue v[10];
		{
			UTMCQueue queue(5);
			UTMCQueueConsumer c;
			c.Attach(&queue);

			// Push one.
			MG_COMMON_ASSERT_F(!queue.PushPending(&v[0]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);

			MG_COMMON_ASSERT_F(queue.FlushPending(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[0], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);
		}
		{
			UTMCQueue queue(5);
			UTMCQueueConsumer c;
			c.Attach(&queue);

			// Flush on empty queue does not do anything.
			MG_COMMON_ASSERT_F(!queue.FlushPending(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);

			// Push 2 so the first one is saved, and the second is
			// written but not available yet.
			MG_COMMON_ASSERT_F(!queue.PushPending(&v[0]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.PushPending(&v[1]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);

			MG_COMMON_ASSERT_F(queue.FlushPending(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[0], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[1], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);
		}
		{
			UTMCQueue queue(5);
			UTMCQueueConsumer c;
			c.Attach(&queue);

			// Pending push does the flush when wpos is switched.
			for (int i = 0; i < 5; ++i)
				MG_COMMON_ASSERT_F(!queue.PushPending(&v[i]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);

			MG_COMMON_ASSERT_F(queue.PushPending(&v[5]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 5, "(Failed)%s]]", testName);
			for (int i = 0; i < 5; ++i)
				MG_COMMON_ASSERT_F(c.Pop() == &v[i], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);

			MG_COMMON_ASSERT_F(queue.FlushPending(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[5], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);
		}
		{
			UTMCQueue queue(5);
			UTMCQueueConsumer c;
			c.Attach(&queue);

			// Double-flush does not change anything.
			MG_COMMON_ASSERT_F(!queue.PushPending(&v[0]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.FlushPending(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.FlushPending(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[0], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);
		}
		{
			UTMCQueue queue(5);
			UTMCQueueConsumer c;
			c.Attach(&queue);

			// Normal push does the flush.
			MG_COMMON_ASSERT_F(!queue.PushPending(&v[0]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Push(&v[1]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[0], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[1], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);
		}
		{
			UTMCQueue queue(5);
			UTMCQueueConsumer c;
			c.Attach(&queue);

			// Second flush-via-push won't return that the queue
			// was empty.
			MG_COMMON_ASSERT_F(queue.Push(&v[0]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.PushPending(&v[1]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.Push(&v[2]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[0], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[1], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == &v[2], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);
		}
		{
			UTMCQueue queue(5);
			UTMCQueueConsumer c;
			c.Attach(&queue);

			// Sub-queue switch when normal push happens, but
			// there is a pending item.
			for (int i = 0; i < 5; ++i)
				MG_COMMON_ASSERT_F(!queue.PushPending(&v[i]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Push(&v[5]), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 6, "(Failed)%s]]", testName);
			for (int i = 0; i < 6; ++i)
				MG_COMMON_ASSERT_F(c.Pop() == &v[i], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c.Pop() == nullptr, "(Failed)%s]]", testName);
		}
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestMCQSingleElement()
	{
		const char* testName = "MCQSingleElement";
		// Ensure the queue still works even with sub-queues of
		// size 1.
		UTMCQueue queue(1);
		MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.ConsumerCount() == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.SubQueueCount() == 1, "(Failed)%s]]", testName);

		UTMCQValue v1;
		UTMCQValue v2;
		UTMCQValue v3;
		queue.Push(&v1);
		queue.Push(&v2);
		queue.Push(&v3);
		MG_COMMON_ASSERT_F(queue.Count() == 3, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.ConsumerCount() == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.SubQueueCount() == 3, "(Failed)%s]]", testName);

		UTMCQueueConsumer c1(&queue);
		MG_COMMON_ASSERT_F(c1.Pop() == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(c1.Pop() == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(c1.Pop() == &v3, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(c1.Pop() == nullptr, "(Failed)%s]]", testName);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestMCQConsumerGap()
	{
		const char* testName = "MCQConsumerGap";
		// See what happens when consumers have a gap between
		// each other. So one of them is beyond the sub-queue list
		// head.
		UTMCQueue queue(2);

		UTMCQValue vs[8];
		UTMCQueueConsumer c1(&queue);
		UTMCQueueConsumer c2(&queue);

		for (int i = 0; i < 8; ++i)
			queue.Push(&vs[i]);

		// First 2 sub-queues are fully consumed, one is garbage collected.
		//
		// [0, 1] -> [2, 3] -> [4, 5] -> [6, 7]
		//  c1
		//  c2
		MG_COMMON_ASSERT_F(c1.Pop() == &vs[0], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 7, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(c1.Pop() == &vs[1], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 6, "(Failed)%s]]", testName);
		// [x, x] -> [2, 3] -> [4, 5] -> [6, 7]
		//  c2        c1
		MG_COMMON_ASSERT_F(c1.Pop() == &vs[2], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 5, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(c1.Pop() == &vs[3], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 4, "(Failed)%s]]", testName);
		// [x, x] -> [x, x] -> [4, 5] -> [6, 7]
		//  c2           c1
		MG_COMMON_ASSERT_F(c1.Pop() == &vs[4], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 3, "(Failed)%s]]", testName);
		// [x, x] -> [x, 5] -> [6, 7]
		//  c2        c1

		// But still is referenced by one of the consumers. And it
		// should recycle it and the next sub-queue now.
		MG_COMMON_ASSERT_F(c2.Pop() == &vs[5], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);
		//  [x, x] -> [6, 7]
		//   c1 c2
		MG_COMMON_ASSERT_F(c1.Pop() == &vs[6], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
		// [x, x] -> [x, 7]
		//     c2     c1
		MG_COMMON_ASSERT_F(c2.Pop() == &vs[7], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
		// [x, x]
		//  c1 c2
		MG_COMMON_ASSERT_F(c1.Pop() == nullptr, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(c2.Pop() == nullptr, "(Failed)%s]]", testName);

		// Ensure it is still functional.
		queue.Push(&vs[0]);
		queue.Push(&vs[1]);
		MG_COMMON_ASSERT_F(c1.Pop() == &vs[0], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(c2.Pop() == &vs[1], "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(c1.Pop() == nullptr, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(c2.Pop() == nullptr, "(Failed)%s]]", testName);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestMCQConsumerDetach()
	{
		const char* testName = "MCQConsumerDetach";
		// See what happens when detach happens for the consumer
		// keeping the head sub-queue, when it is already empty.
		UTMCQValue vs[8];

		// Detach can happen when the head is empty, but some next
		// sub-queue is used by another consumer. The recycling
		// should stop on it regardless of read position.
		{
			UTMCQueue queue(2);

			UTMCQueueConsumer c1(&queue);
			UTMCQueueConsumer c2(&queue);

			for (int i = 0; i < 6; ++i)
				queue.Push(&vs[i]);

			// The head is fully consumed.
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[0], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 5, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[1], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 4, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[2], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[3], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);

			// The consumer, keeping the head is detached. The
			// head still should be recycled. Either at detach, or
			// by another consumer later.
			c2.Detach();

			MG_COMMON_ASSERT_F(c1.Pop() == &vs[4], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[5], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);

			queue.Push(&vs[6]);
			queue.Push(&vs[7]);

			MG_COMMON_ASSERT_F(c1.Pop() == &vs[6], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[7], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);

			// The head was recycled at detach. So 3 sub-queues,
			// not 4.
			MG_COMMON_ASSERT_F(queue.SubQueueCount() == 3, "(Failed)%s]]", testName);
		}

		// Detach can happen when the head is empty, and no more
		// consumers. So the recycling should stop at wpos.
		{
			UTMCQueue queue(2);

			UTMCQueueConsumer c1(&queue);
			UTMCQueueConsumer c2(&queue);

			for (int i = 0; i < 6; ++i)
				queue.Push(&vs[i]);

			// The head is fully consumed.
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[0], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 5, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[1], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 4, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[2], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[3], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);

			// The leading consumer is detached. It can't recycle
			// anything because there is a second consumer staying
			// on the head.
			c1.Detach();

			// The head consumer should recycle until wpos.
			c2.Detach();

			// The head was recycled at detach. So 3 subqueues,
			// not 4.
			queue.Push(&vs[6]);
			queue.Push(&vs[7]);
			MG_COMMON_ASSERT_F(queue.SubQueueCount() == 3, "(Failed)%s]]", testName);

			// Validate.
			c1.Attach(&queue);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[4], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[5], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[6], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[7], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
		}

		// Recycle at detach should not do anything with
		// sub-queues which are not fully consumed. Even if they
		// are not referenced.
		{
			UTMCQueue queue(2);

			UTMCQueueConsumer c1(&queue);
			UTMCQueueConsumer c2(&queue);

			for (int i = 0; i < 4; ++i)
				queue.Push(&vs[i]);

			// The head is fully consumed, and another sub-queue
			// is consumed in half.
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[0], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[1], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[2], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.SubQueueCount() == 2, "(Failed)%s]]", testName);

			// The leading consumer is detached. It can't recycle
			// anything because there is a second consumer staying
			// on the head.
			c1.Detach();

			queue.Push(&vs[4]);
			queue.Push(&vs[5]);
			MG_COMMON_ASSERT_F(queue.SubQueueCount() == 3, "(Failed)%s]]", testName);

			// The head consumer should recycle *not* until wpos,
			// but until the last non-consumed sub-queue.
			c2.Detach();

			// The head was recycled at detach. So 3 subqueues,
			// not 4.
			queue.Push(&vs[6]);
			queue.Push(&vs[7]);
			MG_COMMON_ASSERT_F(queue.SubQueueCount() == 3, "(Failed)%s]]", testName);

			// Validate.
			c1.Attach(&queue);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[3], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 4, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[4], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[5], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[6], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(c1.Pop() == &vs[7], "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.Count() == 0, "(Failed)%s]]", testName);
		}
		Report("(Passed)%s]]", testName);
	}

	class UTMCQLockedConsumerThread
		: public mg::common::Thread
	{
	public:
		UTMCQLockedConsumerThread()
			: myQueue(nullptr)
		{
		}

		void
		Create(
			UTMCQueueLocked* aQueue,
			mg::common::AtomicU32* aTotalPopCount)
		{
			myQueue = aQueue;
			myPopCount = 0;
			myTotalPopCount = aTotalPopCount;
		}

		int
		GetPopCount()
		{
			return myPopCount;
		}

	private:
		void
		Run() override
		{
			UTMCQValue* v;
			uint64_t yield = 0;
			while (!StopRequested())
			{
				while ((v = myQueue->Pop()) != nullptr)
				{
					MG_COMMON_ASSERT_F(v->myValue >= 0, "(Failed)MCQLockedStress]]");
					v->myValue = -v->myValue;
					++myPopCount;
					myTotalPopCount->IncrementRelaxed();
					if (++yield % 10000 == 0)
						mg::common::Sleep(1);
				}
				if (++yield % 10000 == 0)
					mg::common::Sleep(1);
			}
		}

		UTMCQueueLocked* myQueue;
		uint32_t myPopCount;
		mg::common::AtomicU32* myTotalPopCount;
	};

	static void
	UnitTestMCQLockedStress(
		int aElementCount,
		int aThreadCount)
	{
		const char* testName = "MCQLockedStress";
		UTMCQueueLocked queue;
		mg::common::AtomicU32 popCount(0);
		UTMCQValue* values = new UTMCQValue[aElementCount];
		for (int i = 0; i < aElementCount; ++i)
			values[i].myValue = i;

		UTMCQLockedConsumerThread* threads = new UTMCQLockedConsumerThread[aThreadCount];
		for (int i = 0; i < aThreadCount; ++i)
		{
			threads[i].Create(&queue, &popCount);
			threads[i].Start();
		}

		mg::common::QPTimer timer;
		timer.Start();

		uint64_t yield = 0;
		for (int i = 0; i < aElementCount; ++i)
		{
			queue.Push(&values[i]);
			if (++yield % 10000 == 0)
				mg::common::Sleep(1);
		}

		while (popCount.LoadRelaxed() != (uint32_t)aElementCount)
			mg::common::Sleep(1);

		double duration = timer.GetMilliSeconds();

		for (int i = 0; i < aThreadCount; ++i)
			threads[i].BlockingStop();

		for (int i = 0; i < aElementCount; ++i)
			MG_COMMON_ASSERT_F(values[i].myValue == -i, "(Failed)%s]]", testName);

		Report(
			"Locked queue, elements = %d, threads = %d, duration = %lf ms",
			aElementCount, aThreadCount, duration
		);
		for (int i = 0; i < aThreadCount; ++i)
			Report("Thread %d did %d pops", i + 1, threads[i].GetPopCount());

		delete[] threads;
		delete[] values;
		Report("(Passed)%s]]", testName);
	}

	class UTMCQConsumerThread
		: public mg::common::Thread
	{
	public:
		void
		Create(
			UTMCQueue* aQueue,
			mg::common::AtomicU32* aTotalPopCount)
		{
			myConsumer.Attach(aQueue);
			myPopCount = 0;
			myTotalPopCount = aTotalPopCount;
		}

		int
		GetPopCount()
		{
			return myPopCount;
		}

	private:
		void
		Run() override
		{
			UTMCQValue* v;
			uint64_t yield = 0;
			while (!StopRequested())
			{
				while ((v = myConsumer.Pop()) != nullptr)
				{
					MG_COMMON_ASSERT_F(v->myValue >= 0, "(Failed)MCQStress]]");
					v->myValue = -v->myValue;
					++myPopCount;
					myTotalPopCount->IncrementRelaxed();
					if (++yield % 10000 == 0)
						mg::common::Sleep(1);
				}
				if (++yield % 10000 == 0)
					mg::common::Sleep(1);
			}
		}

		UTMCQueueConsumer myConsumer;
		uint32_t myPopCount;
		mg::common::AtomicU32* myTotalPopCount;
	};

	static void
	UnitTestMCQStress(
		bool aIsPushPending,
		bool aReserve,
		int aSubQueueSize,
		int aElementCount,
		int aThreadCount)
	{
		const char* testName = "MCQStress";
		UTMCQueue queue(aSubQueueSize);
		if (aReserve)
			queue.Reserve(aElementCount);

		mg::common::AtomicU32 popCount(0);
		UTMCQValue* values = new UTMCQValue[aElementCount];
		for (int i = 0; i < aElementCount; ++i)
			values[i].myValue = i;

		UTMCQConsumerThread* threads = new UTMCQConsumerThread[aThreadCount];
		for (int i = 0; i < aThreadCount; ++i)
		{
			threads[i].Create(&queue, &popCount);
			threads[i].Start();
		}

		mg::common::QPTimer timer;
		timer.Start();

		if (aIsPushPending)
		{
			for (int i = 0; i < aElementCount; ++i)
				queue.PushPending(&values[i]);
			queue.FlushPending();
		}
		else
		{
			for (int i = 0; i < aElementCount; ++i)
				queue.Push(&values[i]);
		}
		double durationPush = timer.GetMilliSeconds();

		while (popCount.LoadRelaxed() != (uint32_t)aElementCount)
			mg::common::Sleep(1);

		double duration = timer.GetMilliSeconds();

		for (int i = 0; i < aThreadCount; ++i)
			threads[i].BlockingStop();

		for (int i = 0; i < aElementCount; ++i)
			MG_COMMON_ASSERT_F(values[i].myValue == -i, "(Failed)%s]]", testName);

		uint32_t subQueueCountMax =
			aElementCount / aSubQueueSize + (aElementCount % aSubQueueSize != 0);
		MG_COMMON_ASSERT_F(queue.SubQueueCount() <= subQueueCountMax, "(Failed)%s]]", testName);

		Report(
			"Special queue, elements = %d, threads = %d, duration = %lf ms, "
			"push = %lf ms, subqueue count = %u, reserve = %d, pending = %d, "
			"subqueue size = %d", aElementCount, aThreadCount, duration, durationPush,
			queue.SubQueueCount(), (int)aReserve, (int)aIsPushPending, aSubQueueSize
		);
		for (int i = 0; i < aThreadCount; ++i)
			Report("Thread %d did %d pops", i + 1, threads[i].GetPopCount());

		delete[] threads;
		delete[] values;
		Report("(Passed)%s]]", testName);
	}

	void
	UnitTestMultiConsumerQueue()
	{
		TestSuiteGuard suite("MultiConsumerQueue");

		UnitTestMCQLockedBasic();
		UnitTestMCQLockedStress(100, 2);
		UnitTestMCQLockedStress(1000, 5);
		UnitTestMCQLockedStress(1000000, 6);

		UnitTestMCQBasic();
		UnitTestMCQSingleElement();
		UnitTestMCQConsumerGap();
		UnitTestMCQConsumerDetach();
		UnitTestMCQPending();

		UnitTestMCQStress(false, false, 100, 100, 2);
		UnitTestMCQStress(false, true, 100, 100, 2);

		UnitTestMCQStress(false, false, 1000, 1000, 5);
		UnitTestMCQStress(false, true, 1000, 1000, 5);

		UnitTestMCQStress(false, false, 1000, 1000000, 6);
		UnitTestMCQStress(false, true, 1000, 1000000, 6);

		UnitTestMCQStress(true, false, 1000, 1000000, 6);
		UnitTestMCQStress(true, true, 1000, 1000000, 6);
	}

}
}
