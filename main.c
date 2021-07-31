#include "header.h"

int	main(int argc, char **argv)
{
	t_cmd	cmd;

	inits(&cmd);
	if (!(arg_proc(&cmd, argc, argv)))
		return (err_exit(&cmd, bad_arg));
	if (!(spawn_philo(&cmd)))
		return (err_exit(&cmd, cmd.error));
	if (!(thread_start(&cmd)))
		return (err_exit(&cmd, cmd.error));
	if (cmd.error != -1)
		err_exit(&cmd, cmd.error);
	thread_exit(&cmd);
	free_exit(&cmd);
	return (true);
}
