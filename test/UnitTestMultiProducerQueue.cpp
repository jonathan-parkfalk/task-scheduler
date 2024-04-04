#include "mg/common/MultiProducerQueueIntrusive.h"
#include "mg/common/ThreadFunc.h"

#include "mg/test/Random.h"

#include "UnitTest.h"

#include <vector>

namespace mg {
namespace unittests {

	static void
	UnitTestMPQIBasic()
	{
				const char* testName = "MPQIBasic";

		{
			struct Entry
			{
				int myValue;
				Entry* myNext;
			};

			Entry e1;
			e1.myValue = 1;
			Entry e2;
			e2.myValue = 2;
			Entry e3;
			e3.myValue = 3;

			mg::common::MultiProducerQueueIntrusive<Entry> queue;
			Entry* tail;
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.PopAll(tail) == nullptr && tail == nullptr, "(Failed)%s]]", testName);
			Entry* garbage = (Entry*) &queue;
			e1.myNext = garbage;
			e2.myNext = garbage;
			e3.myNext = garbage;

			MG_COMMON_ASSERT_F(queue.Push(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			Entry* res = queue.PopAll(tail);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(res->myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(tail == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.PopAll(tail) == nullptr && tail == nullptr, "(Failed)%s]]", testName);
			e1.myNext = garbage;

			MG_COMMON_ASSERT_F(queue.Push(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.Push(&e2), "(Failed)%s]]", testName);
			res = queue.PopAll(tail);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNext == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(tail == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.PopAll(tail) == nullptr && tail == nullptr, "(Failed)%s]]", testName);
			e1.myNext = garbage;
			e2.myNext = garbage;

			MG_COMMON_ASSERT_F(queue.Push(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.Push(&e2), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.Push(&e3), "(Failed)%s]]", testName);
			res = queue.PopAll(tail);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNext == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == &e3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e3.myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(tail == &e3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.PopAll(tail) == nullptr && tail == nullptr, "(Failed)%s]]", testName);

			// Push empty reversed.
			MG_COMMON_ASSERT_F(queue.PushManyFastReversed(nullptr), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);

			// Push one reversed.
			e1.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushManyFastReversed(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1 && res->myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);

			// Push 2 reversed.
			e2.myNext = &e1;
			e1.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushManyFastReversed(&e2), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNext == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == nullptr, "(Failed)%s]]", testName);

			// Push 3 reversed.
			e3.myNext = &e2;
			e2.myNext = &e1;
			e1.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushManyFastReversed(&e3), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNext == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == &e3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e3.myNext == nullptr, "(Failed)%s]]", testName);

			// Push 4 reversed.
			Entry e4;
			e4.myValue = 4;
			e4.myNext = &e3;
			e3.myNext = &e2;
			e2.myNext = &e1;
			e1.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushManyFastReversed(&e4), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNext == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == &e3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e3.myNext == &e4, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e4.myNext == nullptr, "(Failed)%s]]", testName);

			// Push empty reversed range.
			MG_COMMON_ASSERT_F(queue.PushManyFastReversed(nullptr, nullptr), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);

			// Push one item in a reversed range.
			e1.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushManyFastReversed(&e1, &e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1 && res->myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);

			// Push 2 in a reversed range.
			e2.myNext = &e1;
			e1.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushManyFastReversed(&e2, &e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNext == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == nullptr, "(Failed)%s]]", testName);

			// Make reversed range with non empty last next.
			e4.myNext = &e3;
			e3.myNext = &e2;
			e2.myNext = &e1;
			e1.myNext = nullptr;
			// Should cut e4.
			MG_COMMON_ASSERT_F(queue.PushManyFastReversed(&e4, &e2), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == &e3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e3.myNext == &e4, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e4.myNext == nullptr, "(Failed)%s]]", testName);

			// Push many empty.
			MG_COMMON_ASSERT_F(queue.PushMany(nullptr), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);

			// Push 1 as many.
			e1.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushMany(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1 && res->myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);

			// Push 2 as many.
			e1.myNext = &e2;
			e2.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushMany(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNext == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == nullptr, "(Failed)%s]]", testName);

			// Push 3 as many.
			e1.myNext = &e2;
			e2.myNext = &e3;
			e3.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushMany(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNext == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == &e3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e3.myNext == nullptr, "(Failed)%s]]", testName);

			// Push 4 as many.
			e1.myNext = &e2;
			e2.myNext = &e3;
			e3.myNext = &e4;
			e4.myNext = nullptr;
			MG_COMMON_ASSERT_F(queue.PushMany(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll();
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNext == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNext == &e3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e3.myNext == &e4, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e4.myNext == nullptr, "(Failed)%s]]", testName);
		}
		{
			// Try non-default link name.
			struct Entry
			{
				int myValue;
				Entry* myNextInQueue;
			};

			Entry e1;
			e1.myValue = 1;
			Entry e2;
			e2.myValue = 2;

			mg::common::MultiProducerQueueIntrusive<Entry, &Entry::myNextInQueue> queue;
			Entry* tail;
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.PopAll(tail) == nullptr && tail == nullptr, "(Failed)%s]]", testName);
			Entry* garbage = (Entry*) &queue;
			e1.myNextInQueue = garbage;
			e2.myNextInQueue = garbage;

			MG_COMMON_ASSERT_F(queue.Push(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.Push(&e2), "(Failed)%s]]", testName);
			Entry* res = queue.PopAll(tail);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNextInQueue == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNextInQueue == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(tail == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(queue.PopAll(tail) == nullptr && tail == nullptr, "(Failed)%s]]", testName);

			e1.myNextInQueue = &e2;
			e2.myNextInQueue = nullptr;
			MG_COMMON_ASSERT_F(queue.PushMany(&e1), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!queue.IsEmpty(), "(Failed)%s]]", testName);
			res = queue.PopAll(tail);
			MG_COMMON_ASSERT_F(queue.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(res == &e1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e1.myNextInQueue == &e2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(e2.myNextInQueue == nullptr, "(Failed)%s]]", testName);
		}
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestMPQIStress()
	{
				const char* testName = "MPQIStress";

		struct Entry
		{
			Entry(
				uint32_t aThreadId,
				uint32_t aId)
				: myThreadId(aThreadId)
				, myId(aId)
				, myNext(nullptr)
			{
			}

			uint32_t myThreadId;
			uint32_t myId;
			Entry* myNext;
		};

		const uint32_t itemCount = 100000;
		const uint32_t threadCount = 10;

		mg::common::MultiProducerQueueIntrusive<Entry> queue;
		std::vector<Entry*> data;
		data.reserve(threadCount * itemCount);

		std::vector<mg::common::ThreadFunc*> threads;
		threads.reserve(threadCount);
		mg::common::AtomicU32 readyCount(0);
		for (uint32_t ti = 0; ti < threadCount; ++ti)
		{
			threads.push_back(new mg::common::ThreadFunc([&]() {

				const uint32_t packMaxSize = 5;

				uint32_t threadId = readyCount.FetchIncrementRelaxed();
				while (readyCount.LoadRelaxed() != threadCount)
					mg::common::Sleep(1);

				uint32_t i = 0;
				while (i < itemCount)
				{
					uint32_t packSize = mg::test::RandomUInt32() % packMaxSize + 1;
					if (i + packSize > itemCount)
						packSize = itemCount - i;

					Entry* head = new Entry(threadId, i++);
					Entry* pos = head;
					for (uint32_t j = 1; j < packSize; ++j)
					{
						pos->myNext = new Entry(threadId, i++);
						pos = pos->myNext;
					}
					if (packSize == 1)
						queue.Push(head);
					else
						queue.PushMany(head);
				}
			}));
			threads.back()->Start();
		}

		bool done = false;
		uint64_t yield = 0;
		while (!done)
		{
			done = true;
			for (mg::common::ThreadFunc* f : threads)
				done &= !f->IsRunning();
			Entry* head = queue.PopAll();
			while (head != nullptr)
			{
				data.push_back(head);
				head = head->myNext;
			}
			if (++yield % 1000 == 0)
				mg::common::Sleep(1);
		}
		MG_COMMON_ASSERT_F(data.size() == itemCount * threadCount, "(Failed)%s]]", testName);
		for (mg::common::ThreadFunc* f : threads)
			delete f;
		std::vector<uint32_t> counters;
		counters.reserve(threadCount);
		for (uint32_t i = 0; i < threadCount; ++i)
			counters.push_back(0);
		for (Entry* e : data)
			MG_COMMON_ASSERT_F(e->myId == counters[e->myThreadId]++, "(Failed)%s]]", testName);
		for (uint32_t count : counters)
			MG_COMMON_ASSERT_F(count == itemCount, "(Failed)%s]]", testName);
		Report("(Passed)%s]]", testName);
	}

	void
	UnitTestMultiProducerQueue()
	{
		TestSuiteGuard suite("MultiProducerQueue");

		UnitTestMPQIBasic();
		UnitTestMPQIStress();
	}

}
}
