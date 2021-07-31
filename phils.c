#include "header.h"

/* makes node and puts name and mutexses in node */

static t_node	*make_node(t_cmd *cmd, int name)
{
	t_node	*node;

	node = malloc(sizeof(t_node));
	if (!node)
		return (NULL);
	node->name = name;
	node->next = NULL;
	node->queue = NULL;
	node->cmd = cmd;
	node->ticket = cmd->tickets;
	node->l_mtx = false;
	node->r_mtx = false;
	node->r_fork = cmd->forks[name - 1];
	node->l_fork = cmd->forks[name % cmd->guests];
	return (node);
}

/* prepares mutexses for nodes */

static bool	serve_forks(t_cmd *cmd)
{
	int	len;
	int	ct;

	len = cmd->guests;
	cmd->forks = malloc(sizeof(pthread_mutex_t *) * len);
	if (!cmd->forks)
		return (set_error(cmd, bad_malloc));
	ct = -1;
	while (++ct < len)
	{
		cmd->forks[ct] = malloc(sizeof(pthread_mutex_t));
		if (!cmd->forks[ct])
			return (set_error(cmd, bad_malloc));
		if (pthread_mutex_init(cmd->forks[ct], NULL))
			return (set_error(cmd, bad_init));
	}
	return (true);
}

/* preperes and sets mutexses for queue */

static bool	init_queue(t_cmd *cmd)
{
	t_node	*temp;

	temp = cmd->philos.head;
	while (temp)
	{
		temp->queue = malloc(sizeof(pthread_mutex_t));
		if (!temp->queue)
			return (set_error(cmd, bad_malloc));
		if (pthread_mutex_init(temp->queue, NULL))
			return (set_error(cmd, bad_init));
		if (pthread_mutex_lock(temp->queue))
			return (set_error(cmd, bad_lock));
		temp = temp->next;
	}
	return (true);
}

/* puts node in right place */

bool	spawn_philo(t_cmd *cmd)
{
	t_node	*newnode;
	int		ct;

	if (!(serve_forks(cmd)))
		return (false);
	ct = -1;
	while (++ct < cmd->guests)
	{
		newnode = make_node(cmd, ct + 1);
		if (!newnode)
			return (set_error(cmd, bad_malloc));
		if (cmd->philos.head == NULL)
		{
			cmd->philos.head = newnode;
			cmd->philos.tail = newnode;
		}
		else
		{
			cmd->philos.tail->next = newnode;
			cmd->philos.tail = newnode;
		}
	}
	if (!(init_queue(cmd)))
		return (false);
	return (true);
}
