#include <netdb.h>
#include <stdio.h>

int	main(int argc, char **argv)
{
	(void)argc;
	struct protoent	*protocol;

	protocol = getprotobyname(argv[1]);
	if (!protocol)
	{
		printf("%s\n", "There is error");
		return (1);
	}
	printf("%s\n%s\n", "Name of protocol", protocol->p_name);
	printf("%s\n", "Aliases");
	while (*protocol->p_aliases)
	{
		printf("%s\n", *protocol->p_aliases);
		protocol->p_aliases++;
	}
	printf("%s\n%d\n", "Number of protocol", protocol->p_proto);
	return (0);
}
