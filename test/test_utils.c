#include "../src/utils.c"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_cpy_str_to_arr(void)
{
	char *test_arr[] = {NULL};
	char *test_str   = "TEST STRING\0";
	cpy_str_to_arr(&test_arr[0], test_str);
	TEST_ASSERT_EQUAL_STRING("TEST STRING", test_arr[0]);

	free(test_arr[0]);
	test_str = "\0";
	cpy_str_to_arr(&test_arr[0], test_str);
	TEST_ASSERT_EQUAL_STRING("", test_arr[0]);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_cpy_str_to_arr);
	return UNITY_END();
}
