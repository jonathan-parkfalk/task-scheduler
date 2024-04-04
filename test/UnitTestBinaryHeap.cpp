#include "mg/common/BinaryHeap.h"

#include "UnitTest.h"

#include <algorithm>

namespace mg {
namespace unittests {

	struct UTBHeapValue
	{
		UTBHeapValue() noexcept
			: myValue(-1)
			, myIndex(-1)
		{
			++ourConstrCount;
		}

		~UTBHeapValue() noexcept
		{
			++ourDestrCount;
			myValue = -2;
			myIndex = -2;
		}

		UTBHeapValue(
			const UTBHeapValue& aSrc) noexcept
			: myIndex(-1)
		{
			myValue = aSrc.myValue;
			++ourCopyConstrCount;
		}

		UTBHeapValue(
			UTBHeapValue&& aSrc) noexcept
			: myIndex(-1)
		{
			myValue = aSrc.myValue;
			++ourMoveConstrCount;
		}

		UTBHeapValue&
		operator=(
			const UTBHeapValue& aSrc) noexcept
		{
			myValue = aSrc.myValue;
			myIndex = -1;
			++ourCopyAssignCount;
			return *this;
		}

		UTBHeapValue&
		operator=(
			UTBHeapValue&& aSrc) noexcept
		{
			myValue = aSrc.myValue;
			myIndex = -1;
			++ourMoveAssignCount;
			return *this;
		}

		static void
		UseCopyConstrCount(
			int aToUse,
			const char* testName)
		{
			ourCopyConstrCount -= aToUse;
			MG_COMMON_ASSERT_F(ourCopyConstrCount >= 0, "(Failed)%s]]", testName);
		}

		static void
		UseCopyAssignCount(
			int aToUse, const char* testName)
		{
			ourCopyAssignCount -= aToUse;
			MG_COMMON_ASSERT_F(ourCopyAssignCount >= 0, "(Failed)%s]]", testName);
		}

		static void
		UseMoveConstrCount(
			int aToUse, const char* testName)
		{
			ourMoveConstrCount -= aToUse;
			MG_COMMON_ASSERT_F(ourMoveConstrCount >= 0, "(Failed)%s]]", testName);
		}

		static void
		UseMoveAssignCount(
			int aToUse, const char* testName)
		{
			ourMoveAssignCount -= aToUse;
			MG_COMMON_ASSERT_F(ourMoveAssignCount >= 0, "(Failed)%s]]", testName);
		}

		static void
		UseConstrCount(
			int aToUse, const char* testName)
		{
			ourConstrCount -= aToUse;
			MG_COMMON_ASSERT_F(ourConstrCount >= 0, "(Failed)%s]]", testName);
		}

		static void
		UseDestrCount(
			int aToUse, const char* testName)
		{
			ourDestrCount -= aToUse;
			MG_COMMON_ASSERT_F(ourDestrCount >= 0, "(Failed)%s]]", testName);
		}

		static void
		ResetCounters()
		{
			ourCopyConstrCount = 0;
			ourCopyAssignCount = 0;
			ourMoveConstrCount = 0;
			ourMoveAssignCount = 0;
			ourConstrCount = 0;
			ourDestrCount = 0;
		}

		static void
		CheckCounters(const char* testName)
		{
			MG_COMMON_ASSERT_F(
				ourCopyConstrCount == 0 && ourCopyAssignCount == 0 &&
				ourMoveConstrCount == 0 && ourMoveAssignCount == 0 &&
				ourConstrCount == 0 && ourDestrCount == 0, "(Failed)%s]]", testName
			);
		}

		int myValue;
		int myIndex;

		static int ourCopyConstrCount;
		static int ourCopyAssignCount;
		static int ourMoveConstrCount;
		static int ourMoveAssignCount;
		static int ourConstrCount;
		static int ourDestrCount;

		inline bool
		operator<=(
			const UTBHeapValue& aValue) const
		{
			return myValue <= aValue.myValue;
		}

		inline bool
		operator>=(
			const UTBHeapValue& aValue) const
		{
			return myValue >= aValue.myValue;
		}
	};

	int UTBHeapValue::ourCopyConstrCount = 0;
	int UTBHeapValue::ourCopyAssignCount = 0;
	int UTBHeapValue::ourMoveConstrCount = 0;
	int UTBHeapValue::ourMoveAssignCount = 0;
	int UTBHeapValue::ourConstrCount = 0;
	int UTBHeapValue::ourDestrCount = 0;

	static void
	UnitTestBinaryHeapBasic()
	{
		const char* testName = "BinaryHeapBasic";
		// On 9 the permutation count is 362880.
		// On 10 it is 3628800 - the test becomes too long
		// (order of seconds instead of milliseconds).
		const int count = 9;
		UTBHeapValue values[count];
		{
			mg::common::BinaryHeapMinPtr<UTBHeapValue> heap;
			UTBHeapValue* pop = nullptr;
			MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
			for (int i = 0; i < count; ++i)
				values[i].myValue = i;

			for (int counti = 1; counti <= count; ++counti)
			{
				int indexes[count];
				for (int i = 0; i < counti; ++i)
					indexes[i] = i;
				do
				{
					for (int i = 0; i < counti; ++i)
						heap.Push(&values[indexes[i]]);
					MG_COMMON_ASSERT_F(heap.Count() == (uint32_t)counti, "(Failed)%s]]", testName);
					for (int i = 0; i < counti; ++i)
					{
						pop = nullptr;
						MG_COMMON_ASSERT_F(heap.GetTop() == &values[i], "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(pop == &values[i], "(Failed)%s]]", testName);
					}
					MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
					MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
				} while (std::next_permutation(&indexes[0], &indexes[counti]));
			}
		}
		{
			mg::common::BinaryHeapMaxPtr<UTBHeapValue> heap;
			UTBHeapValue* pop = nullptr;
			MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
			for (int i = 0; i < count; ++i)
				values[i].myValue = count - 1 - i;

			for (int counti = 1; counti <= count; ++counti)
			{
				int indexes[count];
				for (int i = 0; i < counti; ++i)
					indexes[i] = i;
				do
				{
					for (int i = 0; i < counti; ++i)
						heap.Push(&values[indexes[i]]);
					MG_COMMON_ASSERT_F(heap.Count() == (uint32_t)counti, "(Failed)%s]]", testName);
					for (int i = 0; i < counti; ++i)
					{
						pop = nullptr;
						MG_COMMON_ASSERT_F(heap.GetTop() == &values[i], "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(pop == &values[i], "(Failed)%s]]", testName);
					}
					MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
					MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
				} while (std::next_permutation(&indexes[0], &indexes[counti]));
			}
		}
		{
			mg::common::BinaryHeapMax<UTBHeapValue> heap;
			UTBHeapValue pop;
			pop.myValue = INT_MAX;
			MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
			for (int i = 0; i < count; ++i)
				values[i].myValue = count - 1 - i;

			for (int counti = 1; counti <= count; ++counti)
			{
				int indexes[count];
				for (int i = 0; i < counti; ++i)
					indexes[i] = i;
				do
				{
					for (int i = 0; i < counti; ++i)
						heap.Push(values[indexes[i]]);
					MG_COMMON_ASSERT_F(heap.Count() == (uint32_t)counti, "(Failed)%s]]", testName);
					for (int i = 0; i < counti; ++i)
					{
						pop.myValue = INT_MAX;
						MG_COMMON_ASSERT_F(heap.GetTop().myValue == values[i].myValue, "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(pop.myValue == values[i].myValue, "(Failed)%s]]", testName);
					}
					MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
					MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
				} while (std::next_permutation(&indexes[0], &indexes[counti]));
			}
		}
		{
			mg::common::BinaryHeapMin<UTBHeapValue> heap;
			UTBHeapValue pop;
			pop.myValue = INT_MAX;
			MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
			for (int i = 0; i < count; ++i)
				values[i].myValue = i;

			for (int counti = 1; counti <= count; ++counti)
			{
				int indexes[count];
				for (int i = 0; i < counti; ++i)
					indexes[i] = i;
				do
				{
					for (int i = 0; i < counti; ++i)
						heap.Push(values[indexes[i]]);
					MG_COMMON_ASSERT_F(heap.Count() == (uint32_t)counti, "(Failed)%s]]", testName);
					for (int i = 0; i < counti; ++i)
					{
						pop.myValue = INT_MAX;
						MG_COMMON_ASSERT_F(heap.GetTop().myValue == values[i].myValue, "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(pop.myValue == values[i].myValue, "(Failed)%s]]", testName);
					}
					MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
					MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
				} while (std::next_permutation(&indexes[0], &indexes[counti]));
			}
		}
		{
			mg::common::BinaryHeapMin<UTBHeapValue> heap;
			UTBHeapValue pop;
			pop.myValue = INT_MAX;
			MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);

			for (int counti = 1; counti <= count; ++counti)
			{
				int indexes[count];
				for (int i = 0; i < counti; ++i)
					indexes[i] = i;
				do
				{
					for (int i = 0; i < counti; ++i)
					{
						values[i].myValue = 0;
						heap.Push(values[i]);
					}
					for (int i = 0; i < counti; ++i)
					{
						heap.GetTop().myValue = indexes[i];
						heap.UpdateTop();
					}
					MG_COMMON_ASSERT_F(heap.Count() == (uint32_t)counti, "(Failed)%s]]", testName);
					for (int i = 0; i < counti; ++i)
					{
						pop.myValue = INT_MAX;
						MG_COMMON_ASSERT_F(heap.GetTop().myValue == i, "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(pop.myValue == i, "(Failed)%s]]", testName);
					}
					MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
					MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
				} while (std::next_permutation(&indexes[0], &indexes[counti]));
			}
		}
		{
			mg::common::BinaryHeapMinIntrusive<UTBHeapValue> heap;
			UTBHeapValue* pop = nullptr;

			// The idea is to try every possible update for every
			// possible element count in the given range.
			// For each count, each element is moved to each
			// possible place, including places between the other
			// elements, and the heap is checked to stay valid.
			for (int counti = 1; counti <= count; ++counti)
			{
				for (int srci = 0; srci < counti; ++srci)
				{
					for (int newv = 5, endv = srci * 10 + 5; newv <= endv; newv += 5)
					{
						for (int i = 0; i < counti; ++i)
						{
							values[i].myValue = (i + 1) * 10;
							heap.Push(&values[i]);
						}
						values[srci].myValue = newv;
						heap.Update(&values[srci]);
						MG_COMMON_ASSERT_F(values[srci].myIndex >= 0, "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(values[srci].myIndex < counti, "(Failed)%s]]", testName);
						int prev = -1;
						for (int i = 0; i < counti; ++i)
						{
							pop = nullptr;
							MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
							MG_COMMON_ASSERT_F(pop->myIndex == -1, "(Failed)%s]]", testName);
							MG_COMMON_ASSERT_F(pop->myValue >= prev, "(Failed)%s]]", testName);
							MG_COMMON_ASSERT_F(pop->myValue >= 0, "(Failed)%s]]", testName);
							prev = pop->myValue;
							pop->myValue = -1;
						}
						MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
					}
				}
			}
		}
		{
			mg::common::BinaryHeapMaxIntrusive<UTBHeapValue> heap;
			UTBHeapValue* pop = nullptr;

			// The idea is to try every possible delete for every
			// possible element count in the given range.
			// For each count, each element is removed and the
			// heap is checked to stay valid.
			for (int counti = 1; counti <= count; ++counti)
			{
				for (int srci = 0; srci < counti; ++srci)
				{
					for (int i = 0; i < counti; ++i)
					{
						values[i].myValue = i;
						heap.Push(&values[i]);
					}
					values[srci].myValue = INT_MAX;
					heap.Remove(&values[srci]);
					MG_COMMON_ASSERT_F(values[srci].myIndex == -1, "(Failed)%s]]", testName);
					int prev = INT_MAX;
					for (int i = 1; i < counti; ++i)
					{
						pop = nullptr;
						MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(pop->myIndex == -1, "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(pop->myValue < prev, "(Failed)%s]]", testName);
						MG_COMMON_ASSERT_F(pop->myValue >= 0, "(Failed)%s]]", testName);
						prev = pop->myValue;
						pop->myValue = -1;
					}
					MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
					MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
				}
			}
		}
		{
			// Check reservation.
			mg::common::BinaryHeapMaxIntrusive<UTBHeapValue> heap;
			heap.Reserve(1);
			MG_COMMON_ASSERT_F(heap.GetCapacity() >= 1, "(Failed)%s]]", testName);
			heap.Reserve(100);
			MG_COMMON_ASSERT_F(heap.GetCapacity() >= 100, "(Failed)%s]]", testName);
		}
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestBinaryHeapPushPop()
	{
		const char* testName = "BinaryHeapPushPop";
		UTBHeapValue::ResetCounters();
		{
			mg::common::BinaryHeapMax<UTBHeapValue> heap;
			// No static elements are created.
		}
		UTBHeapValue::CheckCounters(testName);
		{
			UTBHeapValue v1;
			UTBHeapValue v2;
			UTBHeapValue v3;
			UTBHeapValue v4;
			UTBHeapValue pop;
			v1.myValue = 1;
			v2.myValue = 2;
			v3.myValue = 3;
			v4.myValue = 4;
			mg::common::BinaryHeapMax<UTBHeapValue> heap;
			heap.Reserve(10);
			UTBHeapValue::UseConstrCount(5, testName);

			heap.Push(v1);
			UTBHeapValue::UseCopyConstrCount(1, testName);

			heap.Push(v2);
			// Copy the new value to the end.
			UTBHeapValue::UseCopyConstrCount(1, testName);
			// It should be on top, so moving procedure is
			// started. A temporary object with the new value is
			// created for that.
			UTBHeapValue::UseMoveConstrCount(1, testName);
			// Move current top down.
			UTBHeapValue::UseMoveAssignCount(1, testName);
			// Move the new value to the top.
			UTBHeapValue::UseMoveAssignCount(1, testName);
			// Temporary object is deleted.
			UTBHeapValue::UseDestrCount(1, testName);

			heap.Push(v3);
			// The same. Because the value 2 stays in place.
			UTBHeapValue::UseCopyConstrCount(1, testName);
			UTBHeapValue::UseMoveConstrCount(1, testName);
			UTBHeapValue::UseMoveAssignCount(2, testName);
			UTBHeapValue::UseDestrCount(1, testName);

			heap.Push(v4);
			// The same but +1 move, because the new element goes
			// up two times.
			UTBHeapValue::UseCopyConstrCount(1, testName);
			UTBHeapValue::UseMoveConstrCount(1, testName);
			UTBHeapValue::UseMoveAssignCount(3, testName);
			UTBHeapValue::UseDestrCount(1, testName);

			MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
			// Return result.
			UTBHeapValue::UseMoveAssignCount(1, testName);
			// Move the rightmost value to the root to start its
			// way down.
			UTBHeapValue::UseMoveAssignCount(1, testName);
			// Move down requires to create one temporary
			// variable.
			UTBHeapValue::UseMoveConstrCount(1, testName);
			// The new root goes down 2 times.
			UTBHeapValue::UseMoveAssignCount(2, testName);
			// Temporary variable is deleted.
			UTBHeapValue::UseDestrCount(1, testName);
			// Removed element is destroyed in the heap.
			UTBHeapValue::UseDestrCount(1, testName);

			MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
			// The tree becomes 2 level - root and 2 children. So
			// one move to return the old root, and one move to
			// set a new root to one of the children.
			UTBHeapValue::UseMoveAssignCount(2, testName);
			// Removed element is destroyed in the heap.
			UTBHeapValue::UseDestrCount(1, testName);

			MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
			// The same.
			UTBHeapValue::UseMoveAssignCount(2, testName);
			// Removed element is destroyed in the heap.
			UTBHeapValue::UseDestrCount(1, testName);

			MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
			// Move the single value to the out parameter.
			UTBHeapValue::UseMoveAssignCount(1, testName);
			// Removed element is destroyed in the heap.
			UTBHeapValue::UseDestrCount(1, testName);
		}
		// Stack variables are destroyed.
		UTBHeapValue::UseDestrCount(5, testName);
		UTBHeapValue::CheckCounters(testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestBinaryHeapUpdateTop()
	{
		const char* testName = "BinaryHeapUpdateTop";
		{
			UTBHeapValue v1;
			UTBHeapValue v2;
			UTBHeapValue v3;
			UTBHeapValue v4;
			UTBHeapValue pop;
			v1.myValue = 1;
			v2.myValue = 2;
			v3.myValue = 3;
			v4.myValue = 4;
			mg::common::BinaryHeapMax<UTBHeapValue> heap;
			heap.Reserve(10);
			heap.Push(v1);
			heap.Push(v2);
			heap.Push(v3);
			heap.Push(v4);
			UTBHeapValue::ResetCounters();

			heap.GetTop().myValue = 0;
			heap.UpdateTop();
			// Need to go down. The root is saved to a temporary
			// variable.
			UTBHeapValue::UseMoveConstrCount(1, testName);
			// Two elements are moved up to fill the root.
			UTBHeapValue::UseMoveAssignCount(2, testName);
			// One move to put the saved value to the end from the
			// temporary variable.
			UTBHeapValue::UseMoveAssignCount(1, testName);
			// Temporary variable is deleted.
			UTBHeapValue::UseDestrCount(1, testName);

			UTBHeapValue::CheckCounters(testName);
			heap.Pop(pop);
			UTBHeapValue::ResetCounters();
			heap.GetTop().myValue = -1;
			heap.UpdateTop();
			// Single swap via a temporary variable, since there
			// is just 1 level left.
			UTBHeapValue::UseMoveConstrCount(1, testName);
			UTBHeapValue::UseMoveAssignCount(2, testName);
			UTBHeapValue::UseDestrCount(1, testName);

			UTBHeapValue::CheckCounters(testName);
			heap.Pop(pop);
			UTBHeapValue::ResetCounters();
			heap.GetTop().myValue = -2;
			heap.UpdateTop();
			// The same.
			UTBHeapValue::UseMoveConstrCount(1, testName);
			UTBHeapValue::UseMoveAssignCount(2, testName);
			UTBHeapValue::UseDestrCount(1, testName);

			UTBHeapValue::CheckCounters(testName);
			heap.Pop(pop);
			UTBHeapValue::ResetCounters();
			heap.GetTop().myValue = -3;
			heap.UpdateTop();
			// No actions when 1 element is left, because don't
			// have elements to reorder with.
		}
		// 5 from the stack and 1 in the heap.
		UTBHeapValue::UseDestrCount(6, testName);
		UTBHeapValue::CheckCounters(testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestBinaryHeapUpdateAny()
	{
		const char* testName = "BinaryHeapUpdateAny";
		mg::common::BinaryHeapMinIntrusive<UTBHeapValue> heap;
		heap.Reserve(10);

		UTBHeapValue v1;
		UTBHeapValue v2;
		UTBHeapValue v3;
		UTBHeapValue v4;
		v1.myValue = 0;
		heap.Push(&v1);
		MG_COMMON_ASSERT_F(v1.myIndex == 0, "(Failed)%s]]", testName);

		heap.Update(&v1);
		MG_COMMON_ASSERT_F(v1.myIndex == 0, "(Failed)%s]]", testName);

		v2.myValue = 1;
		heap.Push(&v2);
		MG_COMMON_ASSERT_F(v2.myIndex == 1, "(Failed)%s]]", testName);
		heap.Update(&v1);
		heap.Update(&v2);

		v2.myValue = -1;
		heap.Update(&v2);
		MG_COMMON_ASSERT_F(v2.myIndex == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v1.myIndex == 1, "(Failed)%s]]", testName);

		UTBHeapValue* pop = nullptr;
		MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop->myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v1.myIndex == 0, "(Failed)%s]]", testName);

		MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop->myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);

		v1.myValue = 0;
		v2.myValue = 1;
		v3.myValue = 2;
		v4.myValue = 3;
		heap.Push(&v1);
		heap.Push(&v2);
		heap.Push(&v3);
		heap.Push(&v4);
		MG_COMMON_ASSERT_F(v1.myIndex == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v2.myIndex == 1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v3.myIndex == 2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v4.myIndex == 3, "(Failed)%s]]", testName);

		v4.myValue = -4;
		heap.Update(&v4);
		MG_COMMON_ASSERT_F(v4.myIndex == 0, "(Failed)%s]]", testName);

		v3.myValue = -3;
		heap.Update(&v3);
		MG_COMMON_ASSERT_F(v3.myIndex == 2, "(Failed)%s]]", testName);

		v2.myValue = -2;
		heap.Update(&v2);
		MG_COMMON_ASSERT_F(v2.myIndex == 1, "(Failed)%s]]", testName);

		v1.myValue = -1;
		heap.Update(&v1);
		MG_COMMON_ASSERT_F(v1.myIndex == 3, "(Failed)%s]]", testName);

		pop = nullptr;
		MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop->myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop == &v4, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop->myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop == &v3, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop->myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop->myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(!heap.Pop(pop), "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestBinaryHeapRemoveTop()
	{
		const char* testName = "BinaryHeapRemoveTop";
		{
			UTBHeapValue v1;
			UTBHeapValue v2;
			UTBHeapValue v3;
			UTBHeapValue v4;
			UTBHeapValue pop;
			v1.myValue = 1;
			v2.myValue = 2;
			v3.myValue = 3;
			v4.myValue = 4;
			mg::common::BinaryHeapMax<UTBHeapValue> heap;
			heap.Reserve(10);
			UTBHeapValue::ResetCounters();

			// NOP on empty.
			heap.RemoveTop();
			MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);

			// Single element remove does not copy nor move
			// anything.
			heap.Push(v1);
			MG_COMMON_ASSERT_F(heap.Count() == 1, "(Failed)%s]]", testName);
			UTBHeapValue::UseCopyConstrCount(1, testName);
			heap.RemoveTop();
			UTBHeapValue::UseDestrCount(1, testName);
			MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
			UTBHeapValue::CheckCounters(testName);

			// Remove() costs less than Pop().
			heap.Push(v1);
			heap.Push(v2);
			heap.Push(v3);
			heap.Push(v4);
			MG_COMMON_ASSERT_F(heap.Count() == 4, "(Failed)%s]]", testName);
			UTBHeapValue::ResetCounters();
			heap.RemoveTop();
			// Move end to the deleted root.
			UTBHeapValue::UseMoveAssignCount(1, testName);
			// Single moving down is required and is done via a
			// swap using a temporary variable.
			UTBHeapValue::UseMoveConstrCount(1, testName);
			UTBHeapValue::UseMoveAssignCount(2, testName);
			UTBHeapValue::UseDestrCount(1, testName);
			// The removed element is destroyed in the heap.
			UTBHeapValue::UseDestrCount(1, testName);
		}
		// 5 from the stack, 3 from the heap.
		UTBHeapValue::UseDestrCount(8, testName);
		UTBHeapValue::CheckCounters(testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestBinaryHeapRemoveAny()
	{
		const char* testName = "BinaryHeapRemoveAny";
		mg::common::BinaryHeapMinIntrusive<UTBHeapValue> heap;
		heap.Reserve(10);

		UTBHeapValue v1;
		UTBHeapValue v2;
		UTBHeapValue v3;
		UTBHeapValue v4;
		v1.myValue = 0;
		heap.Push(&v1);
		MG_COMMON_ASSERT_F(v1.myIndex == 0, "(Failed)%s]]", testName);

		heap.Remove(&v1);
		MG_COMMON_ASSERT_F(v1.myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);

		v2.myValue = 1;
		heap.Push(&v1);
		heap.Push(&v2);

		heap.Remove(&v1);
		MG_COMMON_ASSERT_F(v1.myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v2.myIndex == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(heap.Count() == 1, "(Failed)%s]]", testName);

		heap.Remove(&v2);
		MG_COMMON_ASSERT_F(v2.myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);

		v1.myValue = 0;
		v2.myValue = 1;
		v3.myValue = 2;
		v4.myValue = 3;
		heap.Push(&v1);
		heap.Push(&v2);
		heap.Push(&v3);
		heap.Push(&v4);
		MG_COMMON_ASSERT_F(v1.myIndex == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v2.myIndex == 1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v3.myIndex == 2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v4.myIndex == 3, "(Failed)%s]]", testName);

		heap.Remove(&v1);
		MG_COMMON_ASSERT_F(v1.myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v2.myIndex == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v3.myIndex == 2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v4.myIndex == 1, "(Failed)%s]]", testName);

		heap.Remove(&v2);
		MG_COMMON_ASSERT_F(v2.myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v3.myIndex == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v4.myIndex == 1, "(Failed)%s]]", testName);

		heap.Remove(&v3);
		MG_COMMON_ASSERT_F(v3.myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v4.myIndex == 0, "(Failed)%s]]", testName);

		heap.Remove(&v4);
		MG_COMMON_ASSERT_F(v4.myIndex == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(heap.Count() == 0, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestBinaryHeapIndexName()
	{
		const char* testName = "BinaryHeapIndexName";
		// Can use a non-default index name.
		struct TestValue
		{
			bool
			operator<=(
				const TestValue& aItem2) const
			{
				return myValue <= aItem2.myValue;
			}

			int myValue;
			int myIdx;
		};
		mg::common::BinaryHeapMinIntrusive<TestValue, &TestValue::myIdx> heap;
		heap.Reserve(10);
		TestValue v1;
		v1.myValue = 1;
		v1.myIdx = -1;
		TestValue v2;
		v2.myValue = 2;
		v2.myIdx = -1;
		heap.Push(&v1);
		heap.Push(&v2);
		MG_COMMON_ASSERT_F(v1.myIdx == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v2.myIdx == 1, "(Failed)%s]]", testName);
		TestValue* pop = nullptr;
		MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop->myIdx == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(heap.Pop(pop), "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop->myIdx == -1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(pop == &v2, "(Failed)%s]]", testName);

		heap.Push(&v1);
		heap.Push(&v2);
		v2.myValue = 0;
		heap.Update(&v2);
		MG_COMMON_ASSERT_F(v1.myIdx == 1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v2.myIdx == 0, "(Failed)%s]]", testName);

		heap.Remove(&v2);
		MG_COMMON_ASSERT_F(v1.myIdx == 0, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v2.myIdx == -1, "(Failed)%s]]", testName);

		heap.Remove(&v1);
		MG_COMMON_ASSERT_F(v1.myIdx == -1, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestBinaryHeapRealloc()
	{
		const char* testName = "BinaryHeapRealloc";
		{
			mg::common::BinaryHeapMin<UTBHeapValue> heap;
			UTBHeapValue v;
			v.myValue = 1;
			heap.Push(v);
			UTBHeapValue::ResetCounters();

			v.myValue = 2;
			heap.Push(v);
			// Old value move as a result of realloc.
			UTBHeapValue::UseMoveConstrCount(1, testName);
			// Old location destruction.
			UTBHeapValue::UseDestrCount(1, testName);
			// New value copy.
			UTBHeapValue::UseCopyConstrCount(1, testName);

			v.myValue = 3;
			heap.Push(v);
			// Old values move as a result of realloc.
			UTBHeapValue::UseMoveConstrCount(2, testName);
			// Old location destruction.
			UTBHeapValue::UseDestrCount(2, testName);
			// New value copy.
			UTBHeapValue::UseCopyConstrCount(1, testName);
#if IS_PLATFORM_WIN
			// Windows somewhy grows it not always x2.
			MG_COMMON_ASSERT_F(heap.GetCapacity() >= 3, "(Failed)%s]]", testName);
#else
			// Grows x2 at least.
			MG_COMMON_ASSERT_F(heap.GetCapacity() == 4, "(Failed)%s]]", testName);
#endif
		}
		UTBHeapValue::UseDestrCount(4, testName);
		UTBHeapValue::CheckCounters(testName);
				Report("(Passed)%s]]", testName);

	}

	void
	UnitTestBinaryHeap()
	{
		TestSuiteGuard suite("BinaryHeap");

		UnitTestBinaryHeapBasic();
		UnitTestBinaryHeapPushPop();
		UnitTestBinaryHeapUpdateTop();
		UnitTestBinaryHeapUpdateAny();
		UnitTestBinaryHeapRemoveTop();
		UnitTestBinaryHeapRemoveAny();
		UnitTestBinaryHeapIndexName();
		UnitTestBinaryHeapRealloc();
	}

}
}
