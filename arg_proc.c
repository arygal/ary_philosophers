#include "header.h"

/* ASCII TO POSITIVE INTEGER ALLSO IT CHECKS IF CHAR IS NOT NUMBER*/

static bool atoi_p(const char *str, int *i)
{
	int ct;
	long ret;

	ct = -1;
	ret = 0;
	while (str[++ct])
	{
		if (str[ct] < '0' || str[ct] > '9' || ct > 9)
			return (false);
		ret = ret * 10 + str[ct] - '0';
	}
	if (ret > INT_MAX)
		return (false);
	*i = (int)ret;
	return (true);
}

/* turn str *argv to int arguments and stores it in cmd */ 

bool arg_proc(t_cmd *cmd, int argc, char **argv)
{
	int val;

	if (argc != 6 && argc != 5)
		return (false);
	if (!(atoi_p(argv[1], &val)))
		return (false);
	cmd->guests = val;
	if (!(atoi_p(argv[2], &val)))
		return (false);
	cmd->death = val;
	if (!(atoi_p(argv[3], &val)))
		return (false);
	cmd->feast = val;
	if (!(atoi_p(argv[4], &val)))
		return (false);
	cmd->rest = val;
	if (argc == 6)
	{
		if (!(atoi_p(argv[5], &val)))
			return (false);
		cmd->tickets = val;
	}
	return (true);
}
