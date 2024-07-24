#include "CppUTest/CommandLineTestRunner.h"

int main(int ac, char **av)
{
	int return_value;

	return_value = CommandLineTestRunner::RunAllTests(ac, av);

	return return_value;
}
