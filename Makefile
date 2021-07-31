
NAME = philo

CC = gcc

RM = rm -f

CFLAGS = -Wall -Wextra -Werror

OFLAGS = -O3

DFLAGS = -g

LAG = 100

DEF = -D LAG=

SRCS =	main.c\
		arg_proc.c\
		exit.c\
		inits.c\
		phils.c\
		thread_start.c\
		thread_utils.c\
		thread.c\
		
OBJS = $(patsubst %.c,%.o,$(SRCS))

DINF = $(patsubst %.c,%.d,$(SRCS))

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OFLAGS) $(OBJS) $(DEF)$(LAG) -pthread -o $(NAME)

all:  $(NAME)

debug: 
	$(CC) $(CFLAGS) $(DFLAGS) $(SRCS) -o $(NAME)

%.o : %.c
	$(CC) $(CFLAGS) $(OFLAGS) -c $< -o $@ -MD

include $(wildcard $(DINF))

clean:
	$(RM) $(OBJS) $(DINF)

fclean: clean
	$(RM) $(NAME) $(DINF)

re: fclean all

.PHONY:			all clean fclean re