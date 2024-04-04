#include "UnitTest.h"

#include "mg/common/Assert.h"
#include "mg/common/Util.h"

namespace mg {
namespace unittests {

	static void
	UnitTestUtilFormat()
	{
		const char* testName = "UtilFormat";
		MG_COMMON_ASSERT_F(mg::common::StringFormat("") == "", "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(mg::common::StringFormat("abc") == "abc", "(Failed)%s]]", testName);
		MG_COMMON_ASSERT_F(mg::common::StringFormat(
			"a %d %s b c %u", 1, "str", 2U) == "a 1 str b c 2", "(Failed)%s]]", testName);
					Report("(Passed)%s]]", testName);

	}

	void
	UnitTestUtil()
	{
		TestSuiteGuard suite("Util");

		UnitTestUtilFormat();
	}

}
}
