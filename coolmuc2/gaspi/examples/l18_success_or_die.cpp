#include <l17_success_or_die.h>
#include <cstdlib>
#include <iostream>
#include <GASPI.h>

void success_or_die(const char* file, const int line, const int ec)
{
	if (ec != GASPI_SUCCESS)
	{
		gaspi_string_t str;

		gaspi_error_message(ec, &str);

		fprintf(stderr, "error in %s[%i]: %s\n", file, line, str);

		exit(EXIT_FAILURE);
	}
}
