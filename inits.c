#include "header.h"

/* inits just inits */

void inits(t_cmd *cmd)
{
	cmd->forks = NULL;
	cmd->philos.head = NULL;
	cmd->tickets = -1;
	cmd->error = -1;
	cmd->party = false;
	cmd->queue = false;
}
