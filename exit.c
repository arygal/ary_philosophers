#include "header.h"

/* prints error */

static void errors(int n)
{
	if (n == bad_arg)
		write(2, "bad arguments\n", 14);
	else if (n == bad_malloc)
		write(2, "malloc error\n", 13);
	else if (n == bad_time)
		write(2, "gettimeofday error\n", 19);
	else if (n == bad_create)
		write(2, "pthread_create error\n", 21);
	else if (n == bad_join)
		write(2, "pthread_join error\n", 19);
	else if (n == bad_init)
		write(2, "pthread_mutex_init error\n", 25);
	else if (n == bad_sleep)
		write(2, "usleep error\n", 13);
	else if (n == bad_detach)
		write(2, "pthread_detach error\n", 21);
	else if (n == bad_lock)
		write(2, "pthread_mutex_lock error\n", 25);
	else
		write(2, "pthread_mutex_unlock error\n", 27);
}

/* frees all that was alloceted befor exit */

void free_exit(t_cmd *cmd)
{
	t_node *temp;
	int ct;
	int i;

	ct = 0;
	while (cmd->philos.head)
	{
		temp = cmd->philos.head;
		cmd->philos.head = cmd->philos.head->next;
		free(temp->queue);
		free(temp);
		ct++;
	}
	if (cmd->forks)
	{
		i = -1;
		while (++i < ct)
			free(cmd->forks[i]);
		free(cmd->forks);
	}
}

/* detaches threads and desroys mutex before exit */

void thread_exit(t_cmd *cmd)
{
	t_node *temp;
	int 	ct;

	pthread_mutex_destroy(&cmd->sync);
	pthread_mutex_destroy(&cmd->flag);
	if (cmd->error != bad_create)
		pthread_detach(cmd->master);
	ct = 0;
	temp = cmd->philos.head;
	while (temp)
	{
		pthread_mutex_destroy(cmd->forks[ct]);
		if (cmd->error != bad_create)
			pthread_detach(temp->thread);
		++ct;
		temp = temp->next;
	}
}

/* calls another err func when error occured */

int err_exit(t_cmd *cmd, int n)
{

	cmd->error = n;
	errors(cmd->error);
	if (n == bad_malloc || n == bad_arg)
	{
		free_exit(cmd);
		return(-1);
	}
	thread_exit(cmd);
	free_exit(cmd);
	return (-1);
}

/* sets error to valid flag and unlocks all mutexses */

bool set_error(t_cmd *cmd, int n)
{
	t_node *node;

	cmd->error = n;
	node = cmd->philos.head;
	while (node)
	{
		if (node->l_mtx)
			pthread_mutex_unlock(node->l_fork);
		if (node->r_mtx)
			pthread_mutex_unlock(node->r_fork);
		node = node->next;
	}
	return (false);
}
