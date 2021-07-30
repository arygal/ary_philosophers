
#include "header.h"
void test(t_cmd *cmd);
bool get_time(size_t *t);
bool set_error(t_cmd *cmd, int n);
bool unlock_forcks(t_node *phil);

/* ASCII TO POSITIVE INTEGER ALLSO IT CHECKS IF CHAR IS NOT NUMBER*/

bool atoi_p(const char *str, int *i)
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

void inits(t_cmd *cmd)
{
	cmd->forks = NULL;
	cmd->philos.head = NULL;
	cmd->tickets = -1;
	cmd->error = -1;
	cmd->party = false;
	cmd->queue = false;
}

void thread_errors(t_cmd *cmd)
{
	printf("ErrN:%d\n", cmd->error);
	return;
}

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

void thread_exit(t_cmd *cmd)
{
	t_node *temp;
	int 	ct;

	pthread_mutex_destroy(&cmd->sync);
	pthread_mutex_destroy(&cmd->flag);
	pthread_detach(cmd->master);
	ct = 0;
	temp = cmd->philos.head;
	while (temp)
	{
		pthread_mutex_destroy(cmd->forks[ct]);
		pthread_detach(temp->thread);
		++ct;
		temp = temp->next;
	}
}

int err_exit(t_cmd *cmd, int n)
{

	if (n == bad_arg)
		write(2, "Bad arguments\n", 14);
	else if (n == bad_malloc)
		write(2, "Malloc error\n", 13);
	else if (n == bad_time)
		write(2, "Time error\n", 11);
	else
		thread_errors(cmd);
	thread_exit(cmd);
	free_exit(cmd);
	return (-1);
}

t_node *make_node(t_cmd *cmd, int name)
{
	t_node *node;

	node = malloc(sizeof(t_node));
	if (!node)
		return (NULL);
	node->name = name;
	node->next = NULL;
	node->cmd = cmd;
	node->ticket = cmd->tickets;
	node->l_mtx = false;
	node->r_mtx = false;
	node->r_fork = cmd->forks[name - 1];
	node->l_fork = cmd->forks[name % cmd->guests];
	return (node);
}

bool serve_forks(t_cmd *cmd)
{
	int len;
	int ct;

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

bool init_queue(t_cmd *cmd)
{
	t_node *temp;

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

bool spawn_philo(t_cmd *cmd)
{
	t_node *newnode;
	int ct;
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

bool get_time(size_t *t)
{
	struct timeval time;

	if (gettimeofday(&time, NULL))
		return (false);
	*t = time.tv_sec * 1000 + time.tv_usec / 1000;
	return (true);
}

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

bool set_error(t_cmd *cmd, int n)
{
	t_node *node;

	cmd->error = n;
	node = cmd->philos.head;
	while (node)
	{
		pthread_mutex_unlock(node->l_fork);
		pthread_mutex_unlock(node->r_fork);
		node = node->next;
	}
	return (false);
}

bool thread_tail(t_cmd *cmd, t_node *phil, size_t *time_a, size_t *time_b)
{
	if (!(announcer(phil, rest)))
		return (false);
	if (!(alarm_clock(cmd, *time_a + cmd->rest)))
		return (false);
	if (!(announcer(phil, think)))
		return (false);
	return (true);
}

bool thread_body(t_cmd *cmd, t_node *phil, size_t *time_a, size_t *time_b)
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
	return (thread_tail(cmd, phil, time_a, time_b));
}

bool thread_head(t_cmd *cmd, t_node *phil, size_t *time_a, size_t *time_b)
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
	return (thread_body(cmd, phil, time_a, time_b));
}

bool unlock_forks(t_node *phil)
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
	return (true);
}

void *thread(void *ptr)
{
	t_node *phil;
	size_t time_a;
	size_t time_b;

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
		if (!(thread_head(phil->cmd, phil, &time_a, &time_b)))
			break;
	if (phil->ticket)
		phil->cmd->party = false;
	unlock_forks(phil);
	if (pthread_mutex_lock(&phil->cmd->sync))
		return ((void *)set_error(phil->cmd, bad_lock));
	--phil->cmd->guests;
	if (pthread_mutex_unlock(&phil->cmd->sync))
		return ((void *)set_error(phil->cmd, bad_unlock));
	return (NULL);
}

void *death(void *ptr)
{
	t_cmd *cmd;
	t_node *temp;
	size_t time;

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

bool thread_start(t_cmd *cmd)
{
	void *ptr;
	t_node *temp;

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
	if (cmd->error != -1)
		return (false);
	return (true);
}

int main(int argc, char **argv)
{
	t_cmd cmd;

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
