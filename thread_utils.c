#include "header.h"

/* gets current time */

bool get_time(size_t *t)
{
	struct timeval time;

	if (gettimeofday(&time, NULL))
		return (false);
	*t = time.tv_sec * 1000 + time.tv_usec / 1000;
	return (true);
}

/* anunce what thread do (protected by logic and mutexses) */

bool announcer(t_node *phil, int act)
{
	size_t time;
	if (phil->cmd->error != -1 || !phil->cmd->party)
		return (false);
	if (pthread_mutex_lock(&phil->cmd->sync))
		return (set_error(phil->cmd, bad_lock));
	if (phil->cmd->error != -1 || !phil->cmd->party)
	{
		if (pthread_mutex_unlock(&phil->cmd->sync))
			return (set_error(phil->cmd, bad_unlock));
		return (false);
	}
	if (!(get_time(&time)))
		return (set_error(phil->cmd, bad_time));
	time = time - phil->start;
	if (act == take)
		printf("%lu %i has taken a fork\n", time, phil->name);
	else if (act == eat)
		printf("%lu %i is eating\n", time, phil->name);
	else if (act == rest)
		printf("%lu %i is sleeping\n", time, phil->name);
	else if (act == think)
		printf("%lu %i is thinking\n", time, phil->name);
	else
	{
		phil->cmd->party = false;
		printf("%lu %i died\n", time, phil->name);
	}
	if (pthread_mutex_unlock(&phil->cmd->sync))
		return (set_error(phil->cmd, bad_unlock));
	return (true);
}

/* wakes up thread when curennt time extends time given */

bool alarm_clock(t_cmd *cmd, size_t time)
{
	size_t cur_time;

	while (true)
	{
		if (!get_time(&cur_time))
			return (set_error(cmd, bad_time));
		if (cur_time > time)
			break;
		if (usleep(1000))
			return (set_error(cmd, bad_sleep));
	}
	return (true);
}
