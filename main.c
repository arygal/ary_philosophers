#include "header.h"

void test(t_cmd *cmd);
bool	get_time(unsigned long *t);

/* ASCII TO POSITIVE INTEGER ALLSO IT CHECKS IF CHAR IS NOT NUMBER*/

bool	atoi_p(const char *str, int *i)
{
	int		ct;
	long	ret;

	ct = -1;
	ret = 0;
	while (str[++ct])
	{
		if (str[ct] < '0' || str[ct] > '9' || ct > 9)
			return (false);
		ret = ret * 10 + str[ct] - '0';
	}
	if (ret > INT_MAX)
		return(false);
	*i = (int)ret;
	return (true);
}

bool	arg_proc(t_cmd *cmd, int argc, char **argv)
{
	int val;

	if (argc != 6 && argc != 5)
		return(false);
	if (!(atoi_p(argv[1], &val)))
		return(false);
	cmd->guests = val;
	if (!(atoi_p(argv[2], &val)))
		return(false);
	cmd->death = val;
	if (!(atoi_p(argv[3], &val)))
		return(false);
	cmd->feast = val;
	if(!(atoi_p(argv[4], &val)))
		return(false);
	cmd->rest = val;
	if (argc == 6)
	{
		if (!(atoi_p(argv[5], &val)))
			return(false);
		cmd->tickets = val;
	}
	return(true);
}

void	inits(t_cmd *cmd)
{
	cmd->forks = NULL;
	cmd->philos.head = NULL;
	cmd->tickets = -1;
	cmd->demise = false;
	cmd->error = -1;
}

int err_exit(t_cmd	*cmd, int n)
{
	t_node *tmp;

	if (n == arg)
		printf("Bad arguments\n");
	else if (n == bad_malloc)
		printf("Malloc error\n");
	else if (n == bad_time)
		printf("Time error\n");
	else
		printf("Thread error\n");
	while (cmd->philos.head)
	{
		tmp = cmd->philos.head;
		cmd->philos.head = cmd->philos.head->next;
		free(tmp);
	}
	return(-1);
}

t_node	*make_node(t_cmd *cmd, int name)
{
	t_node *node;

	node = malloc(sizeof(t_node));
	if (!node)
		return(NULL);
	node->name = name;
	node->next = NULL;
	node->cmd = cmd;
	node->alive = true;
	node->ticket = cmd->tickets;
	if (name == cmd->guests && name != 1)
	{
		node->r_fork = cmd->forks[name - 1];
		node->l_fork = cmd->forks[0];
	}
	else if ((name) % 2)
		{
			node->l_fork = cmd->forks[name - 1];
			node->r_fork = cmd->forks[name];
		}
	else
		{
			node->l_fork = cmd->forks[name];
			node->r_fork = cmd->forks[name - 1];
		}
	return(node);
}

bool	serve_forks(t_cmd *cmd)
{
	int len;
	int ct;

	if (cmd->guests == 1)
		len = 2;
	else
		len = cmd->guests;
	cmd->forks = malloc(sizeof(pthread_mutex_t *) * len);
	if (!cmd->forks)
		return(false);
	ct = -1;
	while (++ct < len)
	{
		cmd->forks[ct] = malloc(sizeof(pthread_mutex_t));
		if (!cmd->forks[ct])
			return(false);
		if (pthread_mutex_init(cmd->forks[ct] , NULL))
			return(false);
	}
	return(true);
}

bool	spawn_philo(t_cmd *cmd)
{
	t_node	*newnode;
	int		ct;

	if (!(serve_forks(cmd)))
		return(false);
	ct = -1;
	while(++ct < cmd->guests)
	{
		newnode = make_node(cmd ,ct + 1);
		if (!newnode)
			return(false);
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
	return(true);
}

void	announcer(t_node *phil, int act, unsigned long time)
{	
	time = time - phil->start;
	if (act == take)
		printf("%0.4lu %i has taken a fork\n", time, phil->name);
	else if (act == eat)
		printf("%0.4lu %i is eating\n", time, phil->name);
	else if (act == rest)
		printf("%0.4lu %i is sleeping\n", time, phil->name);
	else if (act == think)
		printf("%0.4lu %i is thinking\n", time, phil->name);
	else
		printf("%0.4lu %i died\n", time, phil->name);
}

bool	status_add(t_cmd *cmd, t_node *phil, unsigned long time_a)
{
	unsigned long time_b;

	if (!(get_time(&time_a)))
		return(false);
	announcer(phil, rest , time_a);
	if (!(get_time(&time_b)))
		return(false);
	if (usleep((cmd->rest - (int)(time_b - time_a)) * 1000))
		return(false);
	if (!(get_time(&time_a)))
		return(false);
	announcer(phil, think, time_a);
	return(true);
}

bool	status(t_cmd *cmd, t_node *phil)
{
	unsigned long time_a;
	unsigned long time_b;

	if (pthread_mutex_lock(phil->l_fork))
		return (false);  // TODO: ERROR HANDLING AND WINRARING;
	if (!(get_time(&time_a)))
		return(false);
	announcer(phil, take, time_a);
	if (pthread_mutex_lock(phil->r_fork))
		return (false);
	if (!(get_time(&time_a)))
		return(false);
	announcer(phil, take, time_a);
	if (!(get_time(&time_a)))
		return(false);
	announcer(phil, eat, time_a);
	phil->life = time_a + phil->cmd->death;
	if (phil->ticket != -1)
		--phil->ticket;
	if (!(get_time(&time_b)))
		return(false);
	if (usleep((cmd->feast - (int)(time_b - time_a)) * 1000))
		return(false);
	if (!(get_time(&time_a)))
		return(false);
	if (pthread_mutex_unlock(phil->l_fork))
		return(false);
	if (pthread_mutex_unlock(phil->r_fork))
		return(false);
	if (!(status_add(cmd, phil, time_a)))
		return(false);
	return(true);
}

bool	get_time(unsigned long *t)
{
	struct timeval time;

	if(gettimeofday(&time, NULL))
		return(false);
	*t =  time.tv_sec * 1000 + time.tv_usec / 1000;
	return(true);
}


void *table(void *menu)
{
	t_node	*phil;
	unsigned long time;

	phil = (t_node *)menu;
	if (!(get_time(&time)))
	{
		phil->cmd->error = bad_time;
		return(false);
	}
	phil->life = time + phil->cmd->death;
	phil->start = time;
	while (phil->alive)
	{
		if (phil->cmd->tickets != -1 && !phil->ticket)
			break ;
		if (!(get_time(&time)))
		{
			phil->cmd->error = bad_time;
			break ;
		}
		if (time > phil->life)
		{
			phil->alive = false;
			if (!(get_time(&time)))
			{
				phil->cmd->error = bad_time;
				break ;
			}
			announcer(phil, died, time);
			break ;
		}
		if (!(status(phil->cmd, phil)))
			break ;
	}
	return(NULL);
}

bool banquet(t_cmd *cmd)
{
	void	*menu;
	t_node	*guest;

	guest = cmd->philos.head;
	while (guest)
	{
		menu = (void *)guest;
		if (pthread_create(&guest->thread, NULL, table, menu))
			return(false);
		guest = guest->next;
	}
	guest = cmd->philos.head;
	while (guest)
	{
		if (pthread_join(guest->thread, NULL))
			return(false);
		guest = guest->next;
	}
	return(true);
}

int		main(int argc ,char **argv)
{
	t_cmd	cmd;
	
	inits(&cmd);
	if (!(arg_proc(&cmd, argc, argv)))
		return(err_exit(&cmd, 0));	
	if (!(spawn_philo(&cmd)))
		return(err_exit(&cmd, 1));
	if (!(banquet(&cmd)))
		return(err_exit(&cmd, 3));
	// test(&cmd);
	return(1);
}

void test(t_cmd *cmd)
{
	t_node *temp;

	temp = cmd->philos.head;
	printf("PHILOS: %d\n", cmd->guests);
	printf("DEATH: %d\n", cmd->death);
	printf("FEAST: %d\n", cmd->feast);
	printf("REST: %d\n", cmd->rest);
	printf("TICKETS: %d\n", cmd->tickets);
	printf("TIME: %lu\n", cmd->time);
	while (temp)
	{
		printf("Spawned N: %i\n" , temp->name);
		temp = temp->next;
	}
}