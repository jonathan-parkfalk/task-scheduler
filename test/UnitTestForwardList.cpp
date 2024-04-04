#include "mg/common/Assert.h"
#include "mg/common/ForwardList.h"

#include "UnitTest.h"

#include <utility>

namespace mg {
namespace unittests {

	struct UTFLValue
	{
		UTFLValue()
			: myValue(-1)
			// Garbage pointer to ensure it is nullified when the
			// element is added to a list.
			, myNext((UTFLValue*)0x123)
		{
		}

		UTFLValue(
			int aValue)
			: myValue(aValue)
			, myNext((UTFLValue*)0x123)
		{
		}

		int myValue;
		UTFLValue* myNext;
	};

	using UTFLList = mg::common::ForwardList<UTFLValue>;

	static void
	UnitTestForwardListBasic()
	{
		const char* testName = "ForwardListBasic";
		// Empty list.
		{
			UTFLList list;
			const UTFLList* clist = &list;
			MG_COMMON_ASSERT_F(list.GetFirst() == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(clist->GetFirst() == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(clist->GetLast() == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.IsEmpty(), "(Failed)%s]]", testName);
			list.Clear();
			list.Reverse();
		}
		// Has 1 element.
		{
			UTFLList list;
			const UTFLList* clist = &list;
			UTFLValue v(1);

			list.Append(&v);
			MG_COMMON_ASSERT_F(v.myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(clist->GetFirst() == &v, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(clist->GetLast() == &v, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!list.IsEmpty(), "(Failed)%s]]", testName);

			list.Reverse();
			MG_COMMON_ASSERT_F(list.GetFirst() == &v, "(Failed)%s]]", testName);

			list.Clear();
			MG_COMMON_ASSERT_F(list.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetFirst() == nullptr, "(Failed)%s]]", testName);

			list.Prepend(&v);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.PopFirst() == &v, "(Failed)%s]]", testName);
		}
		// Many elements.
		{
			UTFLList list;
			const UTFLList* clist = &list;
			UTFLValue v1(1);
			UTFLValue v2(2);
			// Fill with garbage to check the links are nullified.
			v1.myNext = (UTFLValue*) 0x123;
			v2.myNext = v1.myNext;

			list.Append(&v1);
			MG_COMMON_ASSERT_F(v1.myNext == nullptr, "(Failed)%s]]", testName);
			list.Append(&v2);

			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(clist->GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(clist->GetLast() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!list.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == nullptr, "(Failed)%s]]", testName);

			UTFLValue v0(0);
			list.Prepend(&v0);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v0.myNext == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == nullptr, "(Failed)%s]]", testName);

			list.Reverse();
			MG_COMMON_ASSERT_F(list.GetFirst() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v0, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v0.myNext == nullptr, "(Failed)%s]]", testName);

			list.Clear();
			MG_COMMON_ASSERT_F(list.IsEmpty(), "(Failed)%s]]", testName);
		}
		// Construct from a plain empty list.
		{
			UTFLList list(nullptr, nullptr);
			UTFLValue v1(1);
			MG_COMMON_ASSERT_F(list.IsEmpty(), "(Failed)%s]]", testName);
			list.Append(&v1);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v1, "(Failed)%s]]", testName);
		}
		// Construct from a plain one-element list.
		{
			UTFLValue v1(1);
			v1.myNext = &v1;
			UTFLList list(&v1, &v1);
			MG_COMMON_ASSERT_F(v1.myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!list.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v1, "(Failed)%s]]", testName);
		}
		// Construct from a plain two-element list.
		{
			UTFLValue v1(1);
			UTFLValue v2(2);
			v1.myNext = &v2;
			UTFLList list(&v1, &v2);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!list.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v2, "(Failed)%s]]", testName);
		}
		// Construct from a list object.
		{
			UTFLValue v1(1);
			UTFLValue v2(2);
			v1.myNext = &v2;
			UTFLList list1(&v1, &v2);
			UTFLList list2(std::move(list1));

			MG_COMMON_ASSERT_F(list1.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(!list2.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.GetLast() == &v2, "(Failed)%s]]", testName);
		}
		// Pop with tail.
		{
			UTFLList list;
			UTFLValue v1(1);
			UTFLValue v2(2);
			UTFLValue v3(3);
			list.Append(&v1);
			list.Append(&v2);
			list.Append(&v3);
			UTFLValue* tail = &v1;
			UTFLValue* head = list.PopAll(tail);
			MG_COMMON_ASSERT_F(list.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(tail == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(head == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetFirst() == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == nullptr, "(Failed)%s]]", testName);
		}
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestForwardListAppend()
	{
		const char* testName = "ForwardListAppend";
		// Append plain list.
		{
			UTFLList list;
			UTFLValue v1(1);
			UTFLValue v2(2);
			v1.myNext = &v2;

			list.Append(&v1, &v2);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);

			list.Clear();
			list.Append(&v1, &v1);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == nullptr, "(Failed)%s]]", testName);

			list.Clear();
			UTFLValue v3(3);

			v1.myNext = &v2;
			list.Append(&v1, &v2);
			list.Append(&v3, &v3);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v3.myNext == nullptr, "(Failed)%s]]", testName);

			list.Clear();
			UTFLValue v4(4);
			v1.myNext = &v2;
			v3.myNext = &v4;
			// Set to a bad value to see that it is nullified.
			v4.myNext = &v1;
			list.Append(&v1, &v2);
			list.Append(&v3, &v4);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v4, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v3.myNext == &v4, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v4.myNext == nullptr, "(Failed)%s]]", testName);

			list.Append(nullptr, nullptr);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v4, "(Failed)%s]]", testName);
		}
		// Append a list object.
		{
			UTFLList list1;
			UTFLList list2;
			UTFLValue v1(1);

			list1.Append(std::move(list2));
			MG_COMMON_ASSERT_F(list1.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);

			list2.Append(&v1);
			list1.Append(std::move(list2));
			MG_COMMON_ASSERT_F(list1.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetLast() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);

			UTFLValue v2(2);
			UTFLValue v3(2);
			list2.Append(&v2);
			list2.Append(&v3);
			list1.Append(std::move(list2));
			MG_COMMON_ASSERT_F(list1.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetLast() == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);
		}
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestForwardListPrepend()
	{
		const char* testName = "ForwardListPrepend";
		// Prepend multiple.
		{
			UTFLList list;
			UTFLValue v1(1);
			UTFLValue v2(2);

			list.Prepend(&v2);
			list.Prepend(&v1);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == nullptr, "(Failed)%s]]", testName);
		}
		// Prepend a plain list.
		{
			UTFLList list;
			UTFLValue v1(1);
			UTFLValue v2(2);

			list.Prepend(nullptr, nullptr);
			MG_COMMON_ASSERT_F(list.IsEmpty(), "(Failed)%s]]", testName);
			list.Append(&v1);
			MG_COMMON_ASSERT_F(list.PopFirst() == &v1, "(Failed)%s]]", testName);

			v1.myNext = &v1;
			list.Prepend(&v1, &v1);
			MG_COMMON_ASSERT_F(!list.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.PopFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.IsEmpty(), "(Failed)%s]]", testName);

			v1.myNext = &v2;
			list.Prepend(&v1, &v2);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.PopFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.PopFirst() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetFirst() == nullptr, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == nullptr, "(Failed)%s]]", testName);
		}
		// Prepend a list object.
		{
			UTFLList list1;
			UTFLList list2;

			list1.Prepend(std::move(list2));
			MG_COMMON_ASSERT_F(list1.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);

			UTFLValue v1(1);
			list2.Append(&v1);
			list1.Prepend(std::move(list2));
			MG_COMMON_ASSERT_F(list1.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetLast() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);

			UTFLValue v2(2);
			UTFLValue v3(3);
			list2.Append(&v2);
			list2.Append(&v3);
			list1.Prepend(std::move(list2));
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetFirst() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetLast() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v3.myNext == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == nullptr, "(Failed)%s]]", testName);
		}
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestForwardListInsert()
	{
		const char* testName = "ForwardListInsert";
		// Insert into empty.
		{
			UTFLList list;
			UTFLValue v1(1);

			list.Insert(nullptr, &v1);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == nullptr, "(Failed)%s]]", testName);
		}
		// Insert before first.
		{
			UTFLList list;
			UTFLValue v1(1);
			UTFLValue v2(2);
			list.Insert(nullptr, &v2);
			list.Insert(nullptr, &v1);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == nullptr, "(Failed)%s]]", testName);
		}
		// Insert in the middle first.
		{
			UTFLList list;
			UTFLValue v1(1);
			UTFLValue v2(2);
			UTFLValue v3(3);
			list.Append(&v1);
			list.Append(&v3);
			list.Insert(&v1, &v2);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v3.myNext == nullptr, "(Failed)%s]]", testName);
		}
		// Insert in the end.
		{
			UTFLList list;
			UTFLValue v1(1);
			UTFLValue v2(2);
			UTFLValue v3(3);
			list.Append(&v1);
			list.Append(&v2);
			list.Insert(&v2, &v3);
			MG_COMMON_ASSERT_F(list.GetFirst() == &v1, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list.GetLast() == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v1.myNext == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v3.myNext == nullptr, "(Failed)%s]]", testName);
		}
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestForwardListAssign()
	{
		const char* testName = "ForwardListAssign";
		// Assign from a list object.
		{
			UTFLList list1;
			UTFLList list2;
			UTFLValue v1(1);

			list1 = std::move(list2);
			MG_COMMON_ASSERT_F(list1.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);

			list1.Append(&v1);
			list1 = std::move(list2);
			MG_COMMON_ASSERT_F(list1.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);

			UTFLValue v2(2);
			list1.Append(&v1);
			list2.Append(&v2);
			list1 = std::move(list2);
			MG_COMMON_ASSERT_F(list1.GetFirst() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetLast() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);

			UTFLValue v3(3);
			list1.Clear();
			list1.Append(&v1);
			list2.Append(&v2);
			list2.Append(&v3);
			list1 = std::move(list2);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetFirst() == &v2, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetLast() == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v2.myNext == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v3.myNext == nullptr, "(Failed)%s]]", testName);

			list1.Clear();
			list1.Append(&v1);
			list1.Append(&v2);
			list2.Append(&v3);
			list1 = std::move(list2);
			MG_COMMON_ASSERT_F(list2.IsEmpty(), "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetFirst() == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(list1.GetLast() == &v3, "(Failed)%s]]", testName);
			MG_COMMON_ASSERT_F(v3.myNext == nullptr, "(Failed)%s]]", testName);
		}
				Report("(Passed)%s]]", testName);

	}

	struct UTFLValue2
	{
		UTFLValue2()
			: myValue(-1)
			, myNext2((UTFLValue2*)0x123)
		{
		}

		UTFLValue2(
			int aValue)
			: myValue(aValue)
			, myNext2((UTFLValue2*)0x123)
		{
		}

		int myValue;
		UTFLValue2* myNext2;
	};

	using UTFLList2 = mg::common::ForwardList<UTFLValue2, &UTFLValue2::myNext2>;

	static void
	UnitTestForwardListDifferentLink()
	{
		const char* testName = "ForwardListDifferentLink";
		UTFLList2 list2;
		UTFLValue2 v1(1);
		UTFLValue2 v2(2);
		list2.Append(&v1);
		list2.Append(&v2);
		MG_COMMON_ASSERT_F(list2.GetFirst() == &v1, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(list2.GetLast() == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v1.myNext2 == &v2, "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(v2.myNext2 == nullptr, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestForwardListIterator()
	{
		const char* testName = "ForwardListIterator";
		UTFLList list;
		for (UTFLValue* val : list)
		{
			MG_UNUSED(val);
			MG_COMMON_ASSERT_F(false, "(Failed)%s]]", testName);
		}

		UTFLValue v1(1);
		list.Append(&v1);
		int count = 0;
		for (UTFLValue* val : list)
		{
			++count;
			MG_COMMON_ASSERT_F(val->myValue == count, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(count == 1, "(Failed)%s]]", testName);

		count = 0;
		for (UTFLValue* val : list)
		{
			MG_COMMON_ASSERT_F(val == list.PopFirst(), "(Failed)%s]]", testName);
			++count;
			MG_COMMON_ASSERT_F(val->myValue == count, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(count == 1, "(Failed)%s]]", testName);

		UTFLValue v2(2);
		list.Append(&v1);
		list.Append(&v2);
		count = 0;
		for (UTFLValue* val : list)
		{
			++count;
			MG_COMMON_ASSERT_F(val->myValue == count, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(count == 2, "(Failed)%s]]", testName);

		count = 0;
		for (UTFLValue* val : list)
		{
			MG_COMMON_ASSERT_F(val == list.PopFirst(), "(Failed)%s]]", testName);
			++count;
			MG_COMMON_ASSERT_F(val->myValue == count, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(count == 2, "(Failed)%s]]", testName);

		UTFLValue v3(3);
		list.Append(&v1);
		list.Append(&v2);
		list.Append(&v3);
		count = 0;
		for (UTFLValue* val : list)
		{
			++count;
			MG_COMMON_ASSERT_F(val->myValue == count, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(count == 3, "(Failed)%s]]", testName);

		count = 0;
		for (UTFLValue* val : list)
		{
			MG_COMMON_ASSERT_F(val == list.PopFirst(), "(Failed)%s]]", testName);
			++count;
			MG_COMMON_ASSERT_F(val->myValue == count, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(count == 3, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	static void
	UnitTestForwardListConstIterator()
	{
		const char* testName = "ForwardListConstIterator";
		UTFLList list;
		const UTFLList& clist = list;
		for (const UTFLValue* val : clist)
		{
			MG_UNUSED(val);
			MG_COMMON_ASSERT_F(false, "(Failed)%s]]", testName);
		}

		UTFLValue v1(1);
		list.Append(&v1);
		int count = 0;
		for (const UTFLValue* val : clist)
		{
			++count;
			MG_COMMON_ASSERT_F(val->myValue == count, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(count == 1, "(Failed)%s]]", testName);

		UTFLValue v2(2);
		list.Append(&v1);
		list.Append(&v2);
		count = 0;
		for (const UTFLValue* val : clist)
		{
			++count;
			MG_COMMON_ASSERT_F(val->myValue == count, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(count == 2, "(Failed)%s]]", testName);

		UTFLValue v3(3);
		list.Append(&v1);
		list.Append(&v2);
		list.Append(&v3);
		count = 0;
		for (const UTFLValue* val : clist)
		{
			++count;
			MG_COMMON_ASSERT_F(val->myValue == count, "(Failed)%s]]", testName);
		}
		MG_COMMON_ASSERT_F(count == 3, "(Failed)%s]]", testName);
				Report("(Passed)%s]]", testName);

	}

	void
	UnitTestForwardList()
	{
		TestSuiteGuard suite("ForwardList");

		UnitTestForwardListBasic();
		UnitTestForwardListAppend();
		UnitTestForwardListPrepend();
		UnitTestForwardListInsert();
		UnitTestForwardListDifferentLink();
		UnitTestForwardListAssign();
		UnitTestForwardListIterator();
		UnitTestForwardListConstIterator();
	}

}
}
