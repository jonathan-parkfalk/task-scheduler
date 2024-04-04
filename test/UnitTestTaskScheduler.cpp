#include "UnitTest.h"

#include "mg/common/QPTimer.h"
#include "mg/common/Time.h"

#include "mg/serverbox/TaskScheduler.h"

#include "mg/test/Random.h"

namespace mg {
namespace unittests {

	static void
	UnitTestTaskSchedulerBasic()
	{
		TestCaseGuard guard("Basic");
		const char* testName = "SchedulerBasic";
		mg::serverbox::TaskScheduler sched("tst", 1, 5);
		mg::serverbox::TaskCallback cb;
		mg::serverbox::Task* tp;
		mg::common::AtomicU32 progress;

		// Simple test for a task being executed 3 times.
		progress.StoreRelaxed(0);
		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask == tp, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			if (progress.IncrementFetchRelaxed() < 3)
				sched.Post(aTask);
		};
		mg::serverbox::Task t(cb);
		tp = &t;
		sched.Post(&t);
		while (progress.LoadRelaxed() != 3)
			mg::common::Sleep(1);

		// Delay should be respected.
		uint64_t timestamp = mg::common::GetMilliseconds();
		progress.StoreRelaxed(false);
		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask == tp, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(mg::common::GetMilliseconds() >= timestamp + 5, "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		};
		t.SetCallback(cb);
		t.SetDelay(5);
		sched.Post(&t);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Deadline should be respected.
		timestamp = 0;
		progress.StoreRelaxed(false);
		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask == tp, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			if (timestamp == 0)
			{
				timestamp = mg::common::GetMilliseconds() + 5;
				return sched.PostDeadline(aTask, timestamp);
			}
			MG_COMMON_ASSERT_F(mg::common::GetMilliseconds() >= timestamp, "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		};
		t.SetCallback(cb);
		sched.Post(&t);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Task can delete itself before return.
		progress.StoreRelaxed(false);
		tp = new mg::serverbox::Task();
		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask == tp, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
			delete aTask;
		};
		tp->SetCallback(cb);
		sched.Post(tp);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Task's callback can be created in-place.
		progress.StoreRelaxed(false);
		tp = new mg::serverbox::Task([&](mg::serverbox::Task* aTask) {
			cb(aTask);
		});
		sched.Post(tp);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerOrder()
	{
		TestCaseGuard guard("Order");
		const char* testName = "SchedulerOrder";
		// Order is never guaranteed in a multi-thread system. But
		// at least it should be correct when the thread is just
		// one.
		mg::serverbox::TaskScheduler sched("tst", 1, 5);
		mg::serverbox::TaskCallback cb;
		mg::serverbox::Task t1;
		mg::serverbox::Task t2;
		mg::serverbox::Task t3;
		mg::common::AtomicU32 progress(0);
		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(progress.LoadRelaxed() == 0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.IncrementRelaxed();
		};
		t1.SetCallback(cb);

		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(progress.LoadRelaxed() == 1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.IncrementRelaxed();
		};
		t2.SetCallback(cb);

		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(progress.LoadRelaxed() == 2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.IncrementRelaxed();
		};
		t3.SetCallback(cb);

		sched.Post(&t1);
		sched.PostDelay(&t2, 3);
		sched.PostDelay(&t3, 5);

		while (progress.LoadRelaxed() != 3)
			mg::common::Sleep(1);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerDomino()
	{
		TestCaseGuard guard("Domino");
		const char* testName = "SchedulerDomino";

		// Ensure all the workers wakeup each other if necessary.
		// The test is called 'domino', because the worker threads
		// are supposed to wake each other on demand.
		mg::serverbox::TaskScheduler sched("tst", 3, 5);
		mg::serverbox::TaskCallback cb;
		mg::serverbox::Task t1;
		mg::serverbox::Task t2;
		mg::serverbox::Task t3;
		mg::common::AtomicU32 progress(0);
		mg::common::AtomicBool finish(false);
		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.IncrementRelaxed();
			while (!finish.LoadAcquire())
				mg::common::Sleep(1);
			progress.IncrementRelaxed();
		};
		t1.SetCallback(cb);
		t2.SetCallback(cb);
		t3.SetCallback(cb);

		sched.Post(&t1);
		sched.Post(&t2);
		sched.Post(&t3);

		while (progress.LoadRelaxed() != 3)
			mg::common::Sleep(1);
		progress.StoreRelaxed(0);
		finish.StoreRelease(true);
		while (progress.LoadRelaxed() != 3)
			mg::common::Sleep(1);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerWakeup()
	{
		TestCaseGuard guard("Wakeup");
		const char* testName = "SchedulerWakeup";

		mg::serverbox::TaskScheduler sched("tst", 1, 5);
		mg::serverbox::TaskCallback cb;
		mg::serverbox::Task t1;
		mg::common::AtomicU32 progress(false);
		cb = [&](mg::serverbox::Task*) {
			progress.StoreRelaxed(true);
		};
		t1.SetCallback(cb);

		// Wakeup while the task is in the front queue. Should be
		// dispatched immediately to the ready queue.
		sched.Post(&t1);
		sched.Wakeup(&t1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Works for deadlined tasks too.
		progress.StoreRelaxed(false);
		sched.PostDeadline(&t1, MG_DEADLINE_INFINITE - 1);
		sched.Wakeup(&t1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// If a task was woken up, it should not reuse the old
		// deadline after rescheduling.
		progress.StoreRelaxed(0);
		cb = [&](mg::serverbox::Task* aTask) {
			if (progress.IncrementFetchRelaxed() == 1)
				sched.Post(aTask);
		};
		t1.SetCallback(cb);
		sched.PostWait(&t1);
		sched.Wakeup(&t1);
		while (progress.LoadRelaxed() != 2)
			mg::common::Sleep(1);

		// Wakeup works even if the task is being executed now.
		progress.StoreRelaxed(0);
		cb = [&](mg::serverbox::Task* aTask) {
			if (progress.LoadRelaxed() == 0)
			{
				progress.StoreRelaxed(1);
				// Wakeup will happen here, and will make the task
				// executed early despite PostWait() below.
				while (progress.LoadRelaxed() != 2)
					mg::common::Sleep(1);
				return sched.PostWait(aTask);
			}
			progress.StoreRelaxed(3);
		};
		t1.SetCallback(cb);
		sched.Post(&t1);
		while (progress.LoadRelaxed() != 1)
			mg::common::Sleep(1);
		sched.Wakeup(&t1);
		progress.StoreRelaxed(2);
		while (progress.LoadRelaxed() != 3)
			mg::common::Sleep(1);

		// Wakeup for signaled task does not work.
		progress.StoreRelaxed(0);
		cb = [&](mg::serverbox::Task* aTask) {
			if (aTask->IsSignaled())
			{
				progress.StoreRelaxed(1);
				// Wakeup will happen here, and will not affect
				// PostWait(), because the signal is still active.
				while (progress.LoadRelaxed() != 2)
					mg::common::Sleep(1);
				MG_COMMON_ASSERT_F(aTask->ReceiveSignal(), "(Failed)%s]]", testName);
				return sched.PostWait(aTask);
			}
			progress.StoreRelaxed(3);
		};
		t1.SetCallback(cb);
		sched.PostWait(&t1);
		sched.Signal(&t1);
		while (progress.LoadRelaxed() != 1)
			mg::common::Sleep(1);

		sched.Wakeup(&t1);
		progress.StoreRelaxed(2);
		mg::common::Sleep(1);
		MG_COMMON_ASSERT_F(progress.LoadRelaxed() == 2, "(Failed)%s]]", testName);
		while (t1.IsSignaled())
			mg::common::Sleep(1);

		sched.Wakeup(&t1);
		while (progress.LoadRelaxed() != 3)
			mg::common::Sleep(1);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerExpiration()
	{
		TestCaseGuard guard("Expiration");
		const char* testName = "SchedulerExpiration";

		mg::serverbox::TaskScheduler sched("tst", 1, 100);
		mg::serverbox::Task t1;
		mg::common::AtomicU32 progress;

		// Expiration check for non-woken task not having a
		// deadline.
		progress.StoreRelaxed(false);
		t1.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(!aTask->ReceiveSignal(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		});
		sched.Post(&t1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Expiration check for woken task not having a deadline.
		progress.StoreRelaxed(false);
		sched.Wakeup(&t1);
		sched.Post(&t1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Expiration check for non-woken task having a deadline.
		progress.StoreRelaxed(false);
		sched.PostDelay(&t1, 1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Expiration check for woken task having a deadline.
		progress.StoreRelaxed(false);
		t1.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(!aTask->ReceiveSignal(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		});
		sched.Wakeup(&t1);
		sched.PostDeadline(&t1, MG_DEADLINE_INFINITE - 1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Expiration check for signaled task not having a
		// deadline.
		progress.StoreRelaxed(false);
		t1.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask->ReceiveSignal(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		});
		sched.Signal(&t1);
		sched.Post(&t1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Expiration check for signaled task having a deadline.
		progress.StoreRelaxed(false);
		t1.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask->ReceiveSignal(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		});
		sched.Signal(&t1);
		sched.PostDeadline(&t1, MG_DEADLINE_INFINITE - 1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Expiration check for woken task which was in infinite
		// wait.
		progress.StoreRelaxed(false);
		t1.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(!aTask->ReceiveSignal(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		});
		sched.Wakeup(&t1);
		sched.PostWait(&t1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Expiration check for signaled task which was in
		// infinite wait.
		progress.StoreRelaxed(false);
		t1.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask->ReceiveSignal(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		});
		sched.Signal(&t1);
		sched.PostWait(&t1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Deadline adjustment.
		progress.StoreRelaxed(false);
		t1.SetDeadline(MG_DEADLINE_INFINITE);
		MG_COMMON_ASSERT_F(t1.GetDeadline() == MG_DEADLINE_INFINITE, "(Failed)%s]]", testName);
		t1.AdjustDeadline(1);
		MG_COMMON_ASSERT_F(t1.GetDeadline() == 1, "(Failed)%s]]", testName);
		t1.AdjustDeadline(MG_DEADLINE_INFINITE);
		MG_COMMON_ASSERT_F(t1.GetDeadline() == 1, "(Failed)%s]]", testName);
		t1.AdjustDelay(1000000);
		// Does not adjust to a bigger value.
		MG_COMMON_ASSERT_F(t1.GetDeadline() == 1, "(Failed)%s]]", testName);
		t1.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		});
		sched.Post(&t1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerReschedule()
	{
		TestCaseGuard guard("Reschedule");
		const char* testName = "SchedulerReschedule";

		mg::serverbox::TaskScheduler sched1("tst1", 2, 100);
		mg::serverbox::TaskScheduler sched2("tst2", 2, 100);
		mg::serverbox::TaskCallback cb;
		mg::serverbox::Task t1;
		mg::serverbox::Task t2;
		mg::serverbox::Task t3;
		mg::common::AtomicU32 progress(0);
		// A task can schedule another task into the same or
		// different scheduler.
		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask == &t1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			sched1.Post(&t2);
			progress.IncrementRelaxed();
		};
		t1.SetCallback(cb);

		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask == &t2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			sched2.Post(&t3);
			progress.IncrementRelaxed();
		};
		t2.SetCallback(cb);
		t2.SetDelay(3);

		cb = [&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask == &t3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			progress.IncrementRelaxed();
		};
		t3.SetCallback(cb);
		t3.SetDelay(5);

		sched1.Post(&t1);

		while (progress.LoadRelaxed() != 3)
			mg::common::Sleep(1);

		// A task can be signaled while not scheduled. But the
		// signal won't be consumed until the task is posted
		// again.
		progress.StoreRelaxed(false);
		t1.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask == &t1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
			if (progress.ExchangeRelaxed(true))
				MG_COMMON_ASSERT_F(aTask->ReceiveSignal(), "(Failed)%s]]", testName);
		});
		sched1.Post(&t1);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		sched1.Signal(&t1);
		mg::common::Sleep(1);
		MG_COMMON_ASSERT_F(t1.IsSignaled(), "(Failed)%s]]", testName);
		sched1.Post(&t1);
		while (t1.IsSignaled())
			mg::common::Sleep(1);

		// A task can be woken up while not scheduled. But the
		// task is not executed until posted again.
		progress.StoreRelaxed(0);
		t1.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask == &t1, "(Failed)%s]]", testName);
			if (progress.IncrementFetchRelaxed() != 1)
				MG_COMMON_ASSERT_F(!aTask->IsExpired(), "(Failed)%s]]", testName);
			else
				MG_COMMON_ASSERT_F(aTask->IsExpired(), "(Failed)%s]]", testName);
		});
		sched1.Post(&t1);
		while (progress.LoadRelaxed() != 1)
			mg::common::Sleep(1);

		sched1.Wakeup(&t1);
		mg::common::Sleep(1);
		MG_COMMON_ASSERT_F(progress.LoadRelaxed() == 1, "(Failed)%s]]", testName);
		sched1.PostWait(&t1);
		while (progress.LoadRelaxed() != 2)
			mg::common::Sleep(1);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerSignal()
	{
		TestCaseGuard guard("Signal");
		const char* testName = "SchedulerSignal";

		mg::serverbox::TaskScheduler sched("tst", 1, 2);

		// Signal works during execution.
		mg::common::AtomicU32 progress(0);
		mg::serverbox::Task t([&](mg::serverbox::Task* aTask) {
			bool isSignaled = aTask->ReceiveSignal();
			progress.IncrementRelaxed();
			if (isSignaled)
				return;
			// Inject a sleep to ensure the signal works
			// during execution.
			while (progress.LoadRelaxed() != 2)
				mg::common::Sleep(1);
			sched.PostWait(aTask);
		});
		sched.Post(&t);
		while (progress.LoadRelaxed() != 1)
			mg::common::Sleep(1);
		sched.Signal(&t);
		progress.IncrementRelaxed();
		while (progress.LoadRelaxed() != 3)
			mg::common::Sleep(1);

		// Signal works for tasks in the front queue.
		progress.StoreRelaxed(false);
		t.SetCallback([&](mg::serverbox::Task*) {
			MG_COMMON_ASSERT_F(t.ReceiveSignal(), "(Failed)%s]]", testName);
			progress.StoreRelaxed(true);
		});
		sched.PostDeadline(&t, MG_DEADLINE_INFINITE);
		sched.Signal(&t);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Signal works for tasks in the waiting queue. Without
		// -1 the task won't be saved to the waiting queue.
		progress.StoreRelaxed(false);
		sched.PostDeadline(&t, MG_DEADLINE_INFINITE - 1);
		mg::common::Sleep(1);
		sched.Signal(&t);
		while (!progress.LoadRelaxed())
			mg::common::Sleep(1);

		// Signal can be checked before receipt.
		t.SetCallback([&](mg::serverbox::Task* aTask) {
			if (!aTask->IsSignaled())
				return sched.PostWait(aTask);
			MG_COMMON_ASSERT_F(aTask->ReceiveSignal(), "(Failed)%s]]", testName);
		});
		sched.Post(&t);
		sched.Signal(&t);
		while (t.IsSignaled())
			mg::common::Sleep(1);

		// Double signal works.
		progress.StoreRelaxed(false);
		t.SetCallback([&](mg::serverbox::Task* aTask) {
			while (!progress.LoadRelaxed())
				mg::common::Sleep(1);
			MG_COMMON_ASSERT_F(aTask->IsSignaled(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->ReceiveSignal(), "(Failed)%s]]", testName);
		});
		sched.Post(&t);
		sched.Signal(&t);
		sched.Signal(&t);
		progress.StoreRelaxed(true);
		while (t.IsSignaled())
			mg::common::Sleep(1);

		// Task is rescheduled until signal is received.
		progress.StoreRelaxed(0);
		t.SetCallback([&](mg::serverbox::Task* aTask) {
			MG_COMMON_ASSERT_F(aTask->IsSignaled(), "(Failed)%s]]", testName);
			uint32_t p = progress.IncrementFetchRelaxed();
			if (p == 1)
				return sched.PostWait(aTask);
			if (p == 2)
				return sched.PostDeadline(aTask, MG_DEADLINE_INFINITE - 1);
			if (p == 3)
				return sched.PostDelay(aTask, 1000000);
			if (p == 4)
				return sched.Post(aTask);
			MG_COMMON_ASSERT_F(aTask->ReceiveSignal(), "(Failed)%s]]", testName);
		});
		sched.Signal(&t);
		sched.Post(&t);
		while (t.IsSignaled())
			mg::common::Sleep(1);
		MG_COMMON_ASSERT_F(progress.LoadRelaxed() == 5, "(Failed)%s]]", testName);
		Report("(Passed)%s]]", testName);
	}

	struct UTTSchedulerTask;

	struct UTTSchedulerTaskCtx
	{
		UTTSchedulerTaskCtx(
			uint32_t aTaskCount,
			uint32_t aExecuteCount,
			mg::serverbox::TaskScheduler* aScheduler);

		void CreateHeavy(const char* testName);

		void CreateMicro(const char* testName);

		void CreateSignaled(const char* testName);

		void WaitAllExecuted(const char* testName);

		void WaitExecuteCount(
			uint64_t aCount);

		void WaitAllStopped(const char* testName);

		void PostAll();

		~UTTSchedulerTaskCtx();

		UTTSchedulerTask* myTasks;
		const uint32_t myTaskCount;
		const uint32_t myExecuteCount;
		mg::common::AtomicU32 myCurrentParallel;
		mg::common::AtomicU32 myMaxParallel;
		mg::common::AtomicU32 myStopCount;
		mg::common::AtomicU64 myTotalExecuteCount;
		mg::serverbox::TaskScheduler* myScheduler;
	};

	struct UTTSchedulerTask
		: public mg::serverbox::Task
	{
		UTTSchedulerTask()
			: myCtx(nullptr)
		{
		}

		void
		CreateHeavy(
			UTTSchedulerTaskCtx* aCtx, const char* testName)
		{
			myExecuteCount = 0;
			myCtx = aCtx;
			SetCallback(std::bind(
				&UTTSchedulerTask::ExecuteHeavy,
				this, std::placeholders::_1, testName));
		}

		void
		CreateMicro(
			UTTSchedulerTaskCtx* aCtx, const char* testName)
		{
			myExecuteCount = 0;
			myCtx = aCtx;
			SetCallback(std::bind(
				&UTTSchedulerTask::ExecuteMicro,
				this, std::placeholders::_1, testName));
		}

		void
		CreateSignaled(
			UTTSchedulerTaskCtx* aCtx, const char* testName)
		{
			myExecuteCount = 0;
			myCtx = aCtx;
			SetCallback(std::bind(
				&UTTSchedulerTask::ExecuteSignaled, this,
				std::placeholders::_1, testName));
			SetWait();
		}

		void
		ExecuteHeavy(
			mg::serverbox::Task* aTask, const char* testName)
		{
			MG_COMMON_ASSERT_F(aTask == this, "(Failed)%s]]", testName);
			aTask->ReceiveSignal();
			++myExecuteCount;
			myCtx->myTotalExecuteCount.IncrementRelaxed();

			uint32_t i;
			// Tracking max parallel helps to ensure the tasks
			// really use all the worker threads.
			uint32_t old = myCtx->myCurrentParallel.IncrementFetchRelaxed();
			uint32_t max = myCtx->myMaxParallel.LoadRelaxed();
			if (old > max)
				myCtx->myMaxParallel.CmpExchgStrong(max, old);
			// Simulate heavy work.
			for (i = 0; i < 100; ++i)
				mg::test::RandomBool();
			myCtx->myCurrentParallel.DecrementRelaxed();

			bool isLast = myExecuteCount >= myCtx->myExecuteCount;
			if (myExecuteCount % 10 == 0)
			{
				i = mg::test::RandomUniformUInt32(0, myCtx->myTaskCount - 1);
				myCtx->myScheduler->Wakeup(&myCtx->myTasks[i]);
				i = mg::test::RandomUniformUInt32(0, myCtx->myTaskCount - 1);
				myCtx->myScheduler->Signal(&myCtx->myTasks[i]);
				i = mg::test::RandomUniformUInt32(0, myCtx->myTaskCount - 1);
				myCtx->myScheduler->Wakeup(&myCtx->myTasks[i]);
				i = mg::test::RandomUniformUInt32(0, myCtx->myTaskCount - 1);
				myCtx->myScheduler->Signal(&myCtx->myTasks[i]);
				return isLast ? Stop() : myCtx->myScheduler->Post(aTask);
			}
			if (myExecuteCount % 3 == 0)
				return isLast ? Stop() : myCtx->myScheduler->PostDelay(aTask, 2);
			if (myExecuteCount % 2 == 0)
				return isLast ? Stop() : myCtx->myScheduler->PostDelay(aTask, 1);
			return isLast ? Stop() : myCtx->myScheduler->Post(aTask);
		}

		void
		ExecuteMicro(
			mg::serverbox::Task* aTask, const char* testName)
		{
			MG_COMMON_ASSERT_F(aTask == this, "(Failed)%s]]", testName);
			++myExecuteCount;
			myCtx->myTotalExecuteCount.IncrementRelaxed();
			if (myExecuteCount >= myCtx->myExecuteCount)
				return Stop();
			return myCtx->myScheduler->Post(aTask);
		}

		void
		ExecuteSignaled(
			mg::serverbox::Task* aTask, const char* testName)
		{
			MG_COMMON_ASSERT_F(aTask == this, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!aTask->IsExpired(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(aTask->ReceiveSignal(), "(Failed)%s]]", testName);
			++myExecuteCount;
			myCtx->myTotalExecuteCount.IncrementRelaxed();
			if (myExecuteCount >= myCtx->myExecuteCount)
				return Stop();
			return myCtx->myScheduler->PostWait(aTask);
		}

		void
		Stop()
		{
			myCtx->myStopCount.IncrementRelaxed();
		}

		uint32_t myExecuteCount;
		UTTSchedulerTaskCtx* myCtx;
	};

	UTTSchedulerTaskCtx::UTTSchedulerTaskCtx(
		uint32_t aTaskCount,
		uint32_t aExecuteCount,
		mg::serverbox::TaskScheduler* aScheduler)
		: myTasks(new UTTSchedulerTask[aTaskCount])
		, myTaskCount(aTaskCount)
		, myExecuteCount(aExecuteCount)
		, myCurrentParallel(0)
		, myMaxParallel(0)
		, myStopCount(0)
		, myTotalExecuteCount(0)
		, myScheduler(aScheduler)
	{
	}

	void
	UTTSchedulerTaskCtx::CreateHeavy(const char* testName)
	{
		for (uint32_t i = 0; i < myTaskCount; ++i)
			myTasks[i].CreateHeavy(this, testName);
	}

	void
	UTTSchedulerTaskCtx::CreateMicro(const char* testName)
	{
		for (uint32_t i = 0; i < myTaskCount; ++i)
			myTasks[i].CreateMicro(this,testName);
	}

	void
	UTTSchedulerTaskCtx::CreateSignaled(const char* testName)
	{
		for (uint32_t i = 0; i < myTaskCount; ++i)
			myTasks[i].CreateSignaled(this, testName);
	}

	void
	UTTSchedulerTaskCtx::WaitAllExecuted(const char* testName)
	{
		uint64_t total = myExecuteCount * myTaskCount;
		WaitExecuteCount(total);
		MG_COMMON_ASSERT_F(total == myTotalExecuteCount.LoadRelaxed(), "(Failed)%s]]", testName);
	}

	void
	UTTSchedulerTaskCtx::WaitExecuteCount(
		uint64_t aCount)
	{
		while (myTotalExecuteCount.LoadRelaxed() < aCount)
			mg::common::Sleep(1);
	}

	void
	UTTSchedulerTaskCtx::WaitAllStopped(const char* testName)
	{
		while (myStopCount.LoadRelaxed() != myTaskCount)
			mg::common::Sleep(1);
		for (uint32_t i = 0; i < myTaskCount; ++i)
			MG_COMMON_ASSERT_F(myTasks[i].myExecuteCount == myExecuteCount, "(Failed)%s]]", testName);
	}

	void
	UTTSchedulerTaskCtx::PostAll()
	{
		for (uint32_t i = 0; i < myTaskCount; ++i)
			myScheduler->Post(&myTasks[i]);
	}

	UTTSchedulerTaskCtx::~UTTSchedulerTaskCtx()
	{
		delete[] myTasks;
	}

	static void
	UnitTestTaskSchedulerPrintStat(
		const mg::serverbox::TaskScheduler* aSched)
	{
		uint32_t count;
		mg::serverbox::TaskSchedulerThread*const* threads = aSched->GetThreads(count);
		Report("Scheduler stat:");
		for (uint32_t i = 0; i < count; ++i)
		{
			uint64_t execCount = threads[i]->StatPopExecuteCount();
			uint64_t schedCount = threads[i]->StatPopScheduleCount();
			Report("Thread %2u: exec: %12llu, sched: %9llu", i, (unsigned long long)execCount,
				(unsigned long long)schedCount);
		}
		Report("");
	}

	static void
	UnitTestTaskSchedulerBatch(
		uint32_t aThreadCount,
		uint32_t aTaskCount,
		uint32_t aExecuteCount)
	{
		TestCaseGuard guard("Batch");
		const char* testName = "SchedulerBatch";
		Report("Batch test: %u threads, %u tasks, %u executes", aThreadCount, aTaskCount,
			aExecuteCount);
		mg::serverbox::TaskScheduler sched("tst", aThreadCount, 5000);
		UTTSchedulerTaskCtx ctx(aTaskCount, aExecuteCount, &sched);

		ctx.CreateHeavy(testName);
		ctx.PostAll();
		ctx.WaitAllStopped(testName);

		UnitTestTaskSchedulerPrintStat(&sched);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerMicro(
		uint32_t aThreadCount,
		uint32_t aTaskCount)
	{
		TestCaseGuard guard("Micro");
		const char* testName = "TaskSchedulerMicro";

		// Micro test uses super lightweight tasks to check how
		// fast is the scheduler itself, almost not affected by
		// the task bodies.
		Report("Micro test: %u threads, %u tasks", aThreadCount, aTaskCount);
		mg::serverbox::TaskScheduler sched("tst", aThreadCount, 5000);
		UTTSchedulerTaskCtx ctx(aTaskCount, 1, &sched);
		mg::common::QPTimer timer;

		ctx.CreateMicro(testName);
		timer.Start();
		ctx.PostAll();
		ctx.WaitAllExecuted(testName);
		double duration = timer.GetMilliSeconds();
		Report("Duration: %lf ms", duration);
		ctx.WaitAllStopped(testName);

		UnitTestTaskSchedulerPrintStat(&sched);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerMicroNew(
		uint32_t aThreadCount,
		uint32_t aTaskCount)
	{
		TestCaseGuard guard("Micro new");
		const char* testName = "TaskSchedulerMicroNew";

		// Checkout speed of one-shot tasks allocated manually, so
		// it is -1 virtual call compared to automatic one-shot
		// tasks.
		Report("Micro new test: %u threads, %u tasks", aThreadCount, aTaskCount);
		mg::serverbox::TaskScheduler sched("tst", aThreadCount, 5000);
		mg::common::AtomicU64 executeCount(0);
		mg::serverbox::TaskCallback cb(
			[&](mg::serverbox::Task* aTask) {
				executeCount.IncrementRelaxed();
				delete aTask;
			}
		);
		mg::common::QPTimer timer;

		timer.Start();
		for (uint32_t i = 0; i < aTaskCount; ++i)
			sched.Post(new mg::serverbox::Task(cb));
		while (executeCount.LoadRelaxed() != aTaskCount)
			mg::common::Sleep(1);
		double duration = timer.GetMilliSeconds();
		Report("Duration: %lf ms", duration);

		UnitTestTaskSchedulerPrintStat(&sched);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerMicroOneShot(
		uint32_t aThreadCount,
		uint32_t aTaskCount)
	{
		TestCaseGuard guard("Micro one shot");
		const char* testName = "TaskSchedulerMicroOneShot";

		// Checkout speed of one-shot tasks, which are allocated
		// automatically inside of the scheduler.
		Report("Micro one shot test: %u threads, %u tasks", aThreadCount, aTaskCount);
		mg::serverbox::TaskScheduler sched("tst", aThreadCount, 5000);
		mg::common::AtomicU64 executeCount(0);
		mg::serverbox::TaskCallbackOneShot cb([&](void) -> void {
			executeCount.IncrementRelaxed();
		});
		mg::common::QPTimer timer;

		timer.Start();
		for (uint32_t i = 0; i < aTaskCount; ++i)
			sched.PostOneShot(cb);
		while (executeCount.LoadRelaxed() != aTaskCount)
			mg::common::Sleep(1);
		double duration = timer.GetMilliSeconds();
		Report("Duration: %lf ms", duration);

		UnitTestTaskSchedulerPrintStat(&sched);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerPortions(
		uint32_t aThreadCount,
		uint32_t aTaskCount,
		uint32_t aExecuteCount)
	{
		TestCaseGuard guard("Portions");
		const char* testName = "TaskSchedulerPortions";

		// See what happens when tasks are added in multiple
		// steps, not in a single sleep-less loop.
		Report("Portions test: %u threads, %u tasks, %u executes", aThreadCount,
			aTaskCount, aExecuteCount);
		mg::serverbox::TaskScheduler sched("tst", aThreadCount, 5000);
		UTTSchedulerTaskCtx ctx(aTaskCount, aExecuteCount, &sched);

		ctx.CreateHeavy(testName);
		for (uint32_t i = 0; i < aTaskCount; ++i)
		{
			sched.Post(&ctx.myTasks[i]);
			if (i % 10000 == 0)
				mg::common::Sleep(5);
		}
		ctx.WaitAllStopped(testName);

		Report("Max parallel: %u", ctx.myMaxParallel.LoadRelaxed());
		UnitTestTaskSchedulerPrintStat(&sched);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerMildLoad(
		uint32_t aThreadCount,
		uint32_t aTaskCount,
		uint32_t aExecuteCount,
		uint32_t aDuration)
	{
		TestCaseGuard guard("Mild load");
		const char* testName = "TaskSchedulerMildLoad";

		// Try to simulate load close to reality, when RPS may be
		// relatively stable, and not millions.
		MG_COMMON_ASSERT_F(aTaskCount % aDuration == 0, "(Failed)%s]]", testName);
		uint32_t tasksPer50ms = aTaskCount / aDuration * 50;
		Report("Mild load test: %u threads, %u tasks, %u executes, %u per 50ms",
			aThreadCount, aTaskCount, aExecuteCount, tasksPer50ms);
		mg::serverbox::TaskScheduler sched("tst", aThreadCount, 5000);
		UTTSchedulerTaskCtx ctx(aTaskCount, aExecuteCount, &sched);

		ctx.CreateHeavy(testName);
		for (uint32_t i = 0; i < aTaskCount; ++i)
		{
			sched.Post(&ctx.myTasks[i]);
			if (i % tasksPer50ms == 0)
				mg::common::Sleep(50);
		}
		ctx.WaitAllStopped(testName);

		Report("Max parallel: %u", ctx.myMaxParallel.LoadRelaxed());
		UnitTestTaskSchedulerPrintStat(&sched);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerTimeouts(
		uint32_t aTaskCount)
	{
		TestCaseGuard guard("Timeouts");
		const char* testName = "TaskSchedulerTimeouts";

		// The test checks how slow is the scheduler on the
		// slowest case - when tons of tasks are woken up in the
		// order reversed from their insertion. It would cost
		// logarithmic time to remove each of them from the
		// waiting tasks queue's root, which is a binary heap.
		Report("Timeouts test: %u tasks", aTaskCount);
		mg::serverbox::TaskScheduler sched("tst", 1, 5000);
		UTTSchedulerTaskCtx ctx(aTaskCount, 1, &sched);

		ctx.CreateMicro(testName);
		for (uint32_t i = 0; i < aTaskCount; ++i)
			ctx.myTasks[i].SetDelay(1000000 + i);

		// Create one special task pushed last, whose execution
		// would mean all the previous tasks surely ended up in
		// the waiting queue.
		mg::common::Atomic<mg::serverbox::Task*> t;
		t.StoreRelaxed(new mg::serverbox::Task(
			[&](mg::serverbox::Task* aTask) {
				t.StoreRelaxed(nullptr);
				delete aTask;
			}
		));

		mg::common::QPTimer timer;
		timer.Start();
		ctx.PostAll();
		sched.Post(t.LoadRelaxed());

		while (t.LoadRelaxed() != nullptr)
			mg::common::Sleep(1);

		// Wakeup so as the scheduler would remove tasks from the
		// binary heap from its root - the slowest case. For that
		// wakeup from the end, because the front queue is
		// reversed.
		for (int i = aTaskCount - 1; i >= 0; --i)
			sched.Wakeup(&ctx.myTasks[i]);
		ctx.WaitAllExecuted(testName);

		double duration = timer.GetMilliSeconds();
		Report("Duration: %lf ms", duration);

		ctx.WaitAllStopped(testName);
		UnitTestTaskSchedulerPrintStat(&sched);
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestTaskSchedulerSignalStress(
		uint32_t aThreadCount,
		uint32_t aTaskCount,
		uint32_t aExecuteCount)
	{
		TestCaseGuard guard("Signal stress");
		const char* testName = "TaskSchedulerSignalStress";
		// Ensure the tasks never stuck when signals are used.
		Report("Signal stress test: %u threads, %u tasks, %u executes", aThreadCount,
			aTaskCount, aExecuteCount);
		mg::serverbox::TaskScheduler sched("tst", aThreadCount, 5000);
		UTTSchedulerTaskCtx ctx(aTaskCount, aExecuteCount, &sched);

		ctx.CreateSignaled(testName);
		ctx.PostAll();
		for (uint32_t i = 0; i < aExecuteCount; ++i)
		{
			for (uint32_t j = 0; j < aTaskCount; ++j)
				sched.Signal(&ctx.myTasks[j]);
			uint64_t total = aTaskCount * (i + 1);
			ctx.WaitExecuteCount(total);
		}
		ctx.WaitAllStopped(testName);

		UnitTestTaskSchedulerPrintStat(&sched);
		Report("(Passed)%s]]", testName);
	}

	void
	UnitTestTaskScheduler()
	{
		TestSuiteGuard suite("TaskScheduler");

		UnitTestTaskSchedulerBasic();
		UnitTestTaskSchedulerOrder();
		UnitTestTaskSchedulerDomino();
		UnitTestTaskSchedulerWakeup();
		UnitTestTaskSchedulerExpiration();
		UnitTestTaskSchedulerReschedule();
		UnitTestTaskSchedulerSignal();
		UnitTestTaskSchedulerMicro(5, 10000000);
		UnitTestTaskSchedulerMicroNew(5, 10000000);
		UnitTestTaskSchedulerMicroOneShot(5, 10000000);
		UnitTestTaskSchedulerBatch(5, 100000, 100);
		UnitTestTaskSchedulerPortions(5, 100000, 100);
		UnitTestTaskSchedulerMildLoad(5, 100000, 1, 10000);
		UnitTestTaskSchedulerTimeouts(1000000);
		UnitTestTaskSchedulerSignalStress(5, 1000000, 5);
	}

}
}
