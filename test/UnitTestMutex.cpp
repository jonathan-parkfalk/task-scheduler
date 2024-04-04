#include "mg/common/ThreadFunc.h"
#include "mg/common/Time.h"

#include "UnitTest.h"

#include <vector>

namespace mg {
namespace unittests {

	static void
	UnitTestMutexBasic()
	{
				const char* testName = "MutexBasic";

		mg::common::Mutex mutex;
		uint32_t counter = 0;
		const uint32_t threadCount = 10;
		std::vector<mg::common::ThreadFunc*> threads;
		threads.reserve(threadCount);
		for (uint32_t i = 0; i < threadCount; ++i)
		{
			threads.push_back(new mg::common::ThreadFunc([&]() {
				uint64_t deadline = mg::common::GetMilliseconds() + 2000;
				uint64_t yield = 0;
				while (mg::common::GetMilliseconds() < deadline)
				{
					mg::common::MutexLock lock(mutex);
					MG_COMMON_ASSERT_F(counter == 0, "(Failed)%s]]", testName);
					counter++;
					MG_COMMON_ASSERT_F(counter == 1, "(Failed)%s]]", testName);
					counter--;
					MG_COMMON_ASSERT_F(counter == 0, "(Failed)%s]]", testName);
					if (++yield % 1000 == 0)
						mg::common::Sleep(1);
				}
			}));
		}
		for (mg::common::ThreadFunc* f : threads)
			f->Start();
		for (mg::common::ThreadFunc* f : threads)
			delete f;
		MG_COMMON_ASSERT_F(counter == 0, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	void
	UnitTestMutex()
	{
		TestSuiteGuard suite("Mutex");

		UnitTestMutexBasic();
	}

}
}
