#include "header.h"

/* last part -> phil sleeps until next cycle */

bool thread_tail(t_cmd *cmd, t_node *phil, size_t *time_a)
{
	if (!(announcer(phil, rest)))
		return (false);
	if (!(alarm_clock(cmd, *time_a + cmd->rest)))
		return (false);
	if (!(announcer(phil, think)))
		return (false);
	return (true);
}

/* second part -> phil eats and drops forcks(mutexses) */

bool thread_body(t_cmd *cmd, t_node *phil, size_t *time_a)
{
	if (phil->ticket != -1)
		--phil->ticket;
	if (!(announcer(phil, eat)))
		return (false);
	if (!(get_time(time_a)))
		return (set_error(cmd, bad_time));
	phil->life = *time_a + cmd->death;
	if (!(alarm_clock(cmd, *time_a + cmd->feast)))
		return (false);
	if (!(get_time(time_a)))
		return (set_error(cmd, bad_time));
	if (pthread_mutex_unlock(phil->r_fork))
		return (set_error(cmd, bad_unlock));
	phil->r_mtx = false;
	if (pthread_mutex_unlock(phil->l_fork))
		return (set_error(cmd, bad_unlock));
	phil->l_mtx = false;
	if (!phil->ticket)
		return (false);
	return (thread_tail(cmd, phil, time_a));
}

/* first part -> phil tries to pick up forcks(mutexses) */

bool thread_head(t_cmd *cmd, t_node *phil, size_t *time_a)
{
	if (!cmd->party)
		return (false);
	if (pthread_mutex_lock(phil->r_fork))
		return (set_error(cmd, bad_lock));
	phil->r_mtx = true;
	if (!(announcer(phil, take)))
		return (false);
	if (!cmd->philos.head->next)
	{
		if (usleep((cmd->death) * 1000))
			return (set_error(cmd, bad_sleep));
		if (!(announcer(phil, died)))
			return (false);
		if (pthread_mutex_unlock(phil->l_fork))
			return (set_error(cmd, bad_unlock));
		return (false);
	}
	if (pthread_mutex_lock(phil->l_fork))
		return (set_error(cmd, bad_lock));
	phil->l_mtx = true;
	if (!(announcer(phil, take)))
		return (false);
	return (thread_body(cmd, phil, time_a));
}
