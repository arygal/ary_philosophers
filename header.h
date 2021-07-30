#ifndef HEADER_H
# define HEADER_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <stdbool.h>
# include <limits.h>
# include <sys/time.h>
# include <pthread.h>

# define LAG 20
typedef enum e_psa
{
	take,
	eat,
	rest,
	think,
	died
}t_psa;

typedef enum e_err
{
	bad_arg,		// bad arguments
	bad_malloc,		// malloc ret NULL
	bad_time,		// bad gettimeofday
	thread_err,		// errors in thread
	bad_create,		// bad pthread create
	bad_join,		// bad pthread join
	bad_init,		// bad mutex init
	bad_sleep,		// bad usleep
	bad_detach,		// bad detach
	bad_lock,		// bad mutex lock
	bad_unlock		// bad mutex unlock
}t_err;


typedef struct	s_node
{
	int					name;
	int					ticket;
	pthread_mutex_t		*queue;
	size_t				life;
	size_t				start;
	pthread_t			thread;
	pthread_mutex_t		*l_fork;
	pthread_mutex_t		*r_fork;
	bool				l_mtx;
	bool				r_mtx;
	struct s_cmd		*cmd;
	struct s_node		*next;
}t_node;

typedef struct	s_list
{
	t_node	*head;
	t_node	*tail;
}t_list;

typedef struct	s_cmd
{
	t_list			philos;
	pthread_mutex_t	**forks;
	pthread_t		master;
	pthread_mutex_t	sync;
	pthread_mutex_t flag;
	bool			queue;
	int				guests;
	int				death;
	int				feast;
	int				rest;
	int				tickets;
	unsigned long	time;
	short			error;
	bool			party;
}t_cmd;

#endif