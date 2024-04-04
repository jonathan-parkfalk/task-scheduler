#include "mg/common/Assert.h"
#include "mg/common/Atomic.h"

#include "UnitTest.h"

#define TEST_CHECK MG_COMMON_ASSERT_F

namespace mg {
namespace unittests {

	static void
	UnitTestAtomicConstructor()
	{
		const char* testName = "AtomicConstructor";
		// Default.
		{
			mg::common::Atomic<int> value;
			value.Store(123);
			TEST_CHECK(value.Load() == 123, "(Failed)%s]]", testName);
		}
		// With a value
		{
			mg::common::Atomic<int> value(123);
			TEST_CHECK(value.Load() == 123, "(Failed)%s]]", testName);
		}
		Report("(Passed)%s]]", testName);
	}

	static void
	UnitTestAtomicLoad()
	{
		const char* testName = "AtomicLoad";
		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.LoadRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadAcquire() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(value.Load() == 123, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicStore()
	{
		const char* testName = "AtomicStore";
		mg::common::Atomic<int> value(123);
		value.StoreRelaxed(4);
		TEST_CHECK(value.LoadRelaxed() == 4, "(Failed)%s]]", testName);
		value.StoreRelease(5);
		TEST_CHECK(value.LoadRelaxed() == 5, "(Failed)%s]]", testName);
		value.Store(6);
		TEST_CHECK(value.LoadRelaxed() == 6, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicExchange()
	{
		const char* testName = "AtomicExchange";
		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.ExchangeRelaxed(4) == 123, "(Failed)%s]]", testName);
		TEST_CHECK(value.ExchangeAcqRel(5) == 4, "(Failed)%s]]", testName);
		TEST_CHECK(value.ExchangeRelease(6) == 5, "(Failed)%s]]", testName);
		TEST_CHECK(value.Exchange(7) == 6, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 7, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicFetchAdd()
	{
				const char* testName = "AtomicFetchAdd";
		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.FetchAddRelaxed(2) == 123, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchAddAcqRel(3) == 125, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchAddRelease(4) == 128, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchAdd(5) == 132, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 137, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicAddFetch()
	{
				const char* testName = "AtomicAddFetch";

		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.AddFetchRelaxed(2) == 125, "(Failed)%s]]", testName);
		TEST_CHECK(value.AddFetchAcqRel(3) == 128, "(Failed)%s]]", testName);
		TEST_CHECK(value.AddFetchRelease(4) == 132, "(Failed)%s]]", testName);
		TEST_CHECK(value.AddFetch(5) == 137, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 137, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicAdd()
	{
				const char* testName = "AtomicAdd";

		mg::common::Atomic<int> value(123);
		value.AddRelaxed(1);
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		value.AddAcqRel(2);
		TEST_CHECK(value.LoadRelaxed() == 126, "(Failed)%s]]", testName);
		value.AddRelease(3);
		TEST_CHECK(value.LoadRelaxed() == 129, "(Failed)%s]]", testName);
		value.Add(4);
		TEST_CHECK(value.LoadRelaxed() == 133, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicFetchSub()
	{
				const char* testName = "AtomicFetchSub";

		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.FetchSubRelaxed(2) == 123, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchSubAcqRel(3) == 121, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchSubRelease(4) == 118, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchSub(5) == 114, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 109, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicSubFetch()
	{
				const char* testName = "AtomicSubFetch";

		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.SubFetchRelaxed(2) == 121, "(Failed)%s]]", testName);
		TEST_CHECK(value.SubFetchAcqRel(3) == 118, "(Failed)%s]]", testName);
		TEST_CHECK(value.SubFetchRelease(4) == 114, "(Failed)%s]]", testName);
		TEST_CHECK(value.SubFetch(5) == 109, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 109, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicSub()
	{
				const char* testName = "AtomicSub";

		mg::common::Atomic<int> value(123);
		value.SubRelaxed(1);
		TEST_CHECK(value.LoadRelaxed() == 122, "(Failed)%s]]", testName);
		value.SubAcqRel(2);
		TEST_CHECK(value.LoadRelaxed() == 120, "(Failed)%s]]", testName);
		value.SubRelease(3);
		TEST_CHECK(value.LoadRelaxed() == 117, "(Failed)%s]]", testName);
		value.Sub(4);
		TEST_CHECK(value.LoadRelaxed() == 113, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicIncrement()
	{
				const char* testName = "AtomicIncrement";

		mg::common::Atomic<int> value(123);
		value.IncrementRelaxed();
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		value.IncrementAcqRel();
		TEST_CHECK(value.LoadRelaxed() == 125, "(Failed)%s]]", testName);
		value.IncrementRelease();
		TEST_CHECK(value.LoadRelaxed() == 126, "(Failed)%s]]", testName);
		value.Increment();;
		TEST_CHECK(value.LoadRelaxed() == 127, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicFetchIncrement()
	{
				const char* testName = "AtomicFetchIncrement";

		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.FetchIncrementRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchIncrementAcqRel() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchIncrementRelease() == 125, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchIncrement() == 126, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 127, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicIncrementFetch()
	{
				const char* testName = "AtomicIncrementFetch";

		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.IncrementFetchRelaxed() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(value.IncrementFetchAcqRel() == 125, "(Failed)%s]]", testName);
		TEST_CHECK(value.IncrementFetchRelease() == 126, "(Failed)%s]]", testName);
		TEST_CHECK(value.IncrementFetch() == 127, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 127, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicDecrement()
	{
				const char* testName = "AtomicDecrement";

		mg::common::Atomic<int> value(123);
		value.DecrementRelaxed();
		TEST_CHECK(value.LoadRelaxed() == 122, "(Failed)%s]]", testName);
		value.DecrementAcqRel();
		TEST_CHECK(value.LoadRelaxed() == 121, "(Failed)%s]]", testName);
		value.DecrementRelease();
		TEST_CHECK(value.LoadRelaxed() == 120, "(Failed)%s]]", testName);
		value.Decrement();;
		TEST_CHECK(value.LoadRelaxed() == 119, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicFetchDecrement()
	{
				const char* testName = "AtomicFetchDecrement";

		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.FetchDecrementRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchDecrementAcqRel() == 122, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchDecrementRelease() == 121, "(Failed)%s]]", testName);
		TEST_CHECK(value.FetchDecrement() == 120, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 119, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicDecrementFetch()
	{
				const char* testName = "AtomicDecrementFetch";

		mg::common::Atomic<int> value(123);
		TEST_CHECK(value.DecrementFetchRelaxed() == 122, "(Failed)%s]]", testName);
		TEST_CHECK(value.DecrementFetchAcqRel() == 121, "(Failed)%s]]", testName);
		TEST_CHECK(value.DecrementFetchRelease() == 120, "(Failed)%s]]", testName);
		TEST_CHECK(value.DecrementFetch() == 119, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 119, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicCmpExchgWeak()
	{
				const char* testName = "AtomicCmpExchgWeak";

		mg::common::Atomic<int> value(123);
		int expected = 100;
		TEST_CHECK(!value.CmpExchgWeakAcquire(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgWeakAcquire(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		value.StoreRelaxed(123);
		expected = 100;
		TEST_CHECK(!value.CmpExchgWeakAcqRel(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgWeakAcqRel(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		value.StoreRelaxed(123);
		expected = 100;
		TEST_CHECK(!value.CmpExchgWeakRelease(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgWeakRelease(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		value.StoreRelaxed(123);
		expected = 100;
		TEST_CHECK(!value.CmpExchgWeak(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgWeak(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicCmpExchgStrong()
	{
				const char* testName = "AtomicCmpExchgStrong";

		mg::common::Atomic<int> value(123);
		int expected = 100;
		TEST_CHECK(!value.CmpExchgStrongAcquire(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgStrongAcquire(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		value.StoreRelaxed(123);
		expected = 100;
		TEST_CHECK(!value.CmpExchgStrongAcqRel(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgStrongAcqRel(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		value.StoreRelaxed(123);
		expected = 100;
		TEST_CHECK(!value.CmpExchgStrongRelease(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgStrongRelease(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		value.StoreRelaxed(123);
		expected = 100;
		TEST_CHECK(!value.CmpExchgStrong(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 123, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgStrong(expected, 124), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == 124, "(Failed)%s]]", testName);
		TEST_CHECK(expected == 123, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicBool()
	{
				const char* testName = "AtomicBool";

		mg::common::Atomic<bool> value(true);
		TEST_CHECK(value.LoadRelaxed(), "(Failed)%s]]", testName);

		value.StoreRelaxed(false);
		TEST_CHECK(!value.LoadRelaxed(), "(Failed)%s]]", testName);

		TEST_CHECK(!value.ExchangeRelaxed(true), "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed(), "(Failed)%s]]", testName);

		bool expected = false;
		TEST_CHECK(!value.CmpExchgWeakRelaxed(expected, false), "(Failed)%s]]", testName);
		TEST_CHECK(expected, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed(), "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgWeakRelaxed(expected, false), "(Failed)%s]]", testName);
		TEST_CHECK(expected, "(Failed)%s]]", testName);
		TEST_CHECK(!value.LoadRelaxed(), "(Failed)%s]]", testName);

		expected = false;
		value.StoreRelaxed(true);
		TEST_CHECK(!value.CmpExchgStrongRelaxed(expected, false), "(Failed)%s]]", testName);
		TEST_CHECK(expected, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed(), "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgStrongRelaxed(expected, false), "(Failed)%s]]", testName);
		TEST_CHECK(expected, "(Failed)%s]]", testName);
		TEST_CHECK(!value.LoadRelaxed(), "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestAtomicPtr()
	{
				const char* testName = "AtomicPtr";

		int i1, i2;
		void* val1 = (void*)&i1;
		void* val2 = (void*)&i2;

		mg::common::Atomic<void*> value(val1);
		TEST_CHECK(value.LoadRelaxed() == val1, "(Failed)%s]]", testName);

		value.StoreRelaxed(val2);
		TEST_CHECK(value.LoadRelaxed() == val2, "(Failed)%s]]", testName);

		TEST_CHECK(value.ExchangeRelaxed(val1) == val2, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == val1, "(Failed)%s]]", testName);

		void* expected = val2;
		TEST_CHECK(!value.CmpExchgWeakRelaxed(expected, val2), "(Failed)%s]]", testName);
		TEST_CHECK(expected == val1, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == val1, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgWeakRelaxed(expected, val2), "(Failed)%s]]", testName);
		TEST_CHECK(expected == val1, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == val2, "(Failed)%s]]", testName);

		expected = val2;
		value.StoreRelaxed(val1);
		TEST_CHECK(!value.CmpExchgStrongRelaxed(expected, val2), "(Failed)%s]]", testName);
		TEST_CHECK(expected == val1, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == val1, "(Failed)%s]]", testName);

		TEST_CHECK(value.CmpExchgStrongRelaxed(expected, val2), "(Failed)%s]]", testName);
		TEST_CHECK(expected == val1, "(Failed)%s]]", testName);
		TEST_CHECK(value.LoadRelaxed() == val2, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	void
	UnitTestAtomic()
	{
		TestSuiteGuard suite("Atomic");

		UnitTestAtomicConstructor();
		UnitTestAtomicLoad();
		UnitTestAtomicStore();
		UnitTestAtomicExchange();
		UnitTestAtomicFetchAdd();
		UnitTestAtomicAddFetch();
		UnitTestAtomicAdd();
		UnitTestAtomicFetchSub();
		UnitTestAtomicSubFetch();
		UnitTestAtomicSub();
		UnitTestAtomicIncrement();
		UnitTestAtomicFetchIncrement();
		UnitTestAtomicIncrementFetch();
		UnitTestAtomicDecrement();
		UnitTestAtomicFetchDecrement();
		UnitTestAtomicDecrementFetch();
		UnitTestAtomicCmpExchgWeak();
		UnitTestAtomicCmpExchgStrong();
		UnitTestAtomicBool();
		UnitTestAtomicPtr();
	}

}
}
