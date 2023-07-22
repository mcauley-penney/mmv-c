#include "../src/utils.c"
#include "../src/set.c"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_set_alloc(void)
{
	struct Set *test_set = set_alloc(10);

	TEST_ASSERT_EQUAL_UINT(10, test_set->map_capacity);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_set_alloc);

	return UNITY_END();
}
