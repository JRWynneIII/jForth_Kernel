#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>

size_t strlen(const char* str)
{
	size_t ret = 0;
	while ( str[ret] != 0 )
		ret++;
	return ret;
}
