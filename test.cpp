#include <string>
#include <iostream>
#include <time.h>

int main ()
{
	time_t now = time(0);
	std::cout << now << std::endl;

	return 0;
}
