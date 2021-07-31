#include "header.h"

/* unlocks mutexses whten thread is closing */

static bool	unlock_forks(t_node *phil)
{
	if (phil->r_mtx)
	{
		if (pthread_mutex_unlock(phil->r_fork))
			return (set_error(phil->cmd, bad_unlock));
		phil->r_mtx = false;
	}
	if (phil->l_mtx)
	{
		if (pthread_mutex_unlock(phil->l_fork))
			return (set_error(phil->cmd, bad_unlock));
		phil->l_mtx = false;
	}
	if (pthread_mutex_lock(&phil->cmd->sync))
		return ((void *)set_error(phil->cmd, bad_lock));
	--phil->cmd->guests;
	if (pthread_mutex_unlock(&phil->cmd->sync))
		return ((void *)set_error(phil->cmd, bad_unlock));
	return (true);
}

/* basic thread with philo "SIMULATION" */

static void	*thread(void *ptr)
{
	t_node	*phil;
	size_t	time_a;

	phil = (t_node *)ptr;
	if (pthread_mutex_lock(phil->queue))
		return ((void *)set_error(phil->cmd, bad_lock));
	if (pthread_mutex_unlock(phil->queue))
		return ((void *)set_error(phil->cmd, bad_unlock));
	if (!phil->next)
		if (pthread_mutex_unlock(&phil->cmd->flag))
			return ((void *)set_error(phil->cmd, bad_unlock));
	if (!(get_time(&phil->start)))
		return ((void *)set_error(phil->cmd, bad_time));
	phil->life = phil->start + phil->cmd->death;
	while (phil->cmd->party)
		if (!(thread_head(phil->cmd, phil, &time_a)))
			break ;
	if (phil->ticket)
		phil->cmd->party = false;
	unlock_forks(phil);
	return (NULL);
}

/* master thread - it terminate threads if they live longer then they should */

static void	*death(void *ptr)
{
	t_cmd	*cmd;
	t_node	*temp;
	size_t	time;

	cmd = (t_cmd *)ptr;
	if (pthread_mutex_lock(&cmd->flag))
		return ((void *)set_error(cmd, bad_lock));
	if (pthread_mutex_unlock(&cmd->flag))
		return ((void *)set_error(cmd, bad_unlock));
	while (cmd->party)
	{
		temp = cmd->philos.head;
		while (temp)
		{
			if (!cmd->guests)
				return (NULL);
			if (!(get_time(&time)))
				return ((void *)set_error(cmd, bad_time));
			if (temp->life < time)
				if (!(announcer(temp, died)))
					return (false);
			temp = temp->next;
		}
	}
	return (NULL);
}

/* releses threads in queue order then spawn master thread */

static bool	death_start(t_cmd *cmd)
{
	void	*ptr;
	t_node	*temp;

	temp = cmd->philos.head;
	while (temp)
	{
		if (pthread_mutex_unlock(temp->queue))
			return (set_error(cmd, bad_unlock));
		if (usleep(LAG))
			return (set_error(cmd, bad_sleep));
		temp = temp->next;
	}
	ptr = (void *)cmd;
	if (pthread_create(&cmd->master, NULL, death, ptr))
		return (set_error(cmd, bad_create));
	if (pthread_join(cmd->master, NULL))
		return (set_error(cmd, bad_join));
	temp = cmd->philos.head;
	while (temp)
	{
		if (pthread_join(temp->thread, NULL))
			return (set_error(cmd, bad_join));
		temp = temp->next;
	}
	return (true);
}

/* inits  lst 2 mutexses for sync and queue than start spwawn of threads */

bool	thread_start(t_cmd *cmd)
{
	void	*ptr;
	t_node	*temp;

	if (pthread_mutex_init(&cmd->sync, NULL))
		return (set_error(cmd, bad_init));
	if (pthread_mutex_init(&cmd->flag, NULL))
		return (set_error(cmd, bad_init));
	temp = cmd->philos.head;
	cmd->party = true;
	if (pthread_mutex_lock(&cmd->flag))
		return (set_error(cmd, bad_lock));
	while (temp)
	{
		ptr = (void *)temp;
		if (pthread_create(&temp->thread, NULL, thread, ptr))
			return (set_error(cmd, bad_create));
		temp = temp->next;
	}
	return (death_start(cmd));
}
