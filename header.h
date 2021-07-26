#ifndef HEADER_H
# define HEADER_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <stdbool.h>
# include <limits.h>
# include <sys/time.h>
# include <pthread.h>


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
	arg,
	bad_malloc,
	bad_time,
	thread
}t_err;


typedef struct	s_node
{
	pthread_t			thread;
	bool				alive;
	int					name;
	int					ticket;
	pthread_mutex_t		*l_fork;
	pthread_mutex_t		*r_fork;
	unsigned	long	life;
	unsigned	long	start;
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
	int				guests;
	int				death;
	int				feast;
	int				rest;
	int				tickets;
	unsigned long	time;
	short			error;
	bool			demise;
}t_cmd;

#endif