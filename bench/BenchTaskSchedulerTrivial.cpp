#include "Bench.h"

#include "mg/common/Callback.h"
#include "mg/common/ConditionVariable.h"
#include "mg/common/ForwardList.h"
#include "mg/common/Mutex.h"
#include "mg/common/Thread.h"

namespace mg {
namespace bench {

	class Task;
	class TaskScheduler;
	class TaskSchedulerThread;

	using TaskCallback = mg::common::Callback<void(Task*)>;
	using TaskCallbackOneShot = mg::common::Callback<void()>;

	class Task
	{
	public:
		Task();

		template<typename... Args>
		void SetCallback(
			Args&&... aArgs);

		bool ReceiveSignal();

		Task* myNext;
	private:
		TaskCallback myCallback;

		friend TaskScheduler;
		friend TaskSchedulerThread;
	};

	using TaskList = mg::common::ForwardList<Task>;

	// Trivial task scheduler takes a mutex lock on each task post and pop. The simplest
	// possible implementation and the most typical one. For the sake of further
	// simplicity and speed it doesn't support any features except just task execution:
	// no deadline, no wakeup, no signal, nothing else. Their addition using just mutex
	// locks would make this thing awfully slow, but the point is to compare the
	// feature-full original task scheduler vs this features-less one having some unfair
	// simplifications.
	//
	class TaskScheduler
	{
	public:
		TaskScheduler(
			const char* aName,
			uint32 aThreadCount,
			uint32 aSubQueueSize);

		~TaskScheduler();

		void Post(
			Task* aTask);

		template<typename... Args>
		void PostOneShot(
			Args&&... aArgs);

		void PostDelay(
			Task* aTask,
			uint32 aDelay);

		void Wakeup(
			Task* aTask);

		void Signal(
			Task* aTask);

		void Reserve(
			uint32 aCount);

		TaskSchedulerThread* const* GetThreads(
			uint32& aOutCount) const;

	private:
		mg::common::Mutex myMutex;
		mg::common::ConditionVariable myCond;
		bool myIsStopped;
		TaskList myQueue;
		std::vector<TaskSchedulerThread*> myWorkers;

		friend TaskSchedulerThread;
	};

	class TaskSchedulerThread
		: private mg::common::Thread
	{
	public:
		TaskSchedulerThread(
			const char* aName,
			TaskScheduler* aScheduler);

		~TaskSchedulerThread() override;

		uint64 StatPopExecuteCount();

		uint64 StatPopScheduleCount();

	private:
		void Run() override;

		TaskScheduler* myScheduler;
		std::atomic<uint64> myExecuteCount;
	};

	//////////////////////////////////////////////////////////////////////////////////////

	Task::Task()
		: myNext(nullptr)
	{
	}

	template<typename... Args>
	void
	Task::SetCallback(
		Args&&... aArgs)
	{
		myCallback.Set(mg::common::Forward<Args>(aArgs)...);
	}

	bool
	Task::ReceiveSignal()
	{
		MG_COMMON_ASSERT(!"Not implemented");
		return false;
	}

	TaskScheduler::TaskScheduler(
		const char* aName,
		uint32 aThreadCount,
		uint32 /*aSubQueueSize*/)
		: myIsStopped(false)
	{
		myWorkers.resize(aThreadCount);
		for (TaskSchedulerThread*& w : myWorkers)
			w = new TaskSchedulerThread(aName, this);
	}

	TaskScheduler::~TaskScheduler()
	{
		myMutex.Lock();
		myIsStopped = true;
		myCond.Broadcast();
		myMutex.Unlock();
		for (TaskSchedulerThread* w : myWorkers)
			delete w;
	}

	void
	TaskScheduler::Post(
		Task* aTask)
	{
		myMutex.Lock();
		bool wasEmpty = myQueue.IsEmpty();
		myQueue.Append(aTask);
		// Broadcast on each post wouldn't make sense - the workers start waiting on it
		// only when the ready-queue becomes empty. Enough to light it up when the queue
		// state transitioned "empty" -> "non-empty".
		if (wasEmpty)
			myCond.Broadcast();
		myMutex.Unlock();
	}

	template<typename... Args>
	void
	TaskScheduler::PostOneShot(
		Args&&... aArgs)
	{
		Task* t = new Task();
		TaskCallbackOneShot* cb =
			new TaskCallbackOneShot(mg::common::Forward<Args>(aArgs)...);
		t->SetCallback([cb](Task* aTask) {
			(*cb)();
			delete cb;
			delete aTask;
		});
		Post(t);
	}

	void
	TaskScheduler::PostDelay(
		Task* /*aTask*/,
		uint32 /*aDelay*/)
	{
		MG_COMMON_ASSERT(!"Not implemented");
	}

	void
	TaskScheduler::Wakeup(
		Task* /*aTask*/)
	{
		MG_COMMON_ASSERT(!"Not implemented");
	}

	void
	TaskScheduler::Signal(
		Task* /*aTask*/)
	{
		MG_COMMON_ASSERT(!"Not implemented");
	}

	void
	TaskScheduler::Reserve(
		uint32 /*aCount*/)
	{
	}

	TaskSchedulerThread* const*
	TaskScheduler::GetThreads(
		uint32& aOutCount) const
	{
		aOutCount = myWorkers.size();
		return myWorkers.data();
	}

	TaskSchedulerThread::TaskSchedulerThread(
		const char* aName,
		TaskScheduler* aScheduler)
		: Thread(mg::common::StringFormat(
			"mgsb.tsksch%s", aName).c_str())
		, myScheduler(aScheduler)
	{
		Start();
	}

	TaskSchedulerThread::~TaskSchedulerThread()
	{
		BlockingStop();
	}

	uint64
	TaskSchedulerThread::StatPopExecuteCount()
	{
		return myExecuteCount.exchange(0);
	}

	uint64
	TaskSchedulerThread::StatPopScheduleCount()
	{
		return 0;
	}

	void
	TaskSchedulerThread::Run()
	{
		mg::common::Mutex& mutex = myScheduler->myMutex;
		mg::common::ConditionVariable& cond = myScheduler->myCond;
		bool& isStopped = myScheduler->myIsStopped;
		TaskList& queue = myScheduler->myQueue;

		mutex.Lock();
		while (true)
		{
			uint64 batch = 0;
			while (!queue.IsEmpty())
			{
				Task* t = queue.PopFirst();
				mutex.Unlock();
				++batch;
				t->myCallback(t);
				mutex.Lock();
			}
			myExecuteCount.fetch_add(batch);
			if (isStopped)
				break;
			cond.Wait(mutex);
		}
		mutex.Unlock();
	}

}
}

#include "BenchTaskSchedulerTemplate.hpp"
