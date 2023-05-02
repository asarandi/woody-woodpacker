/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asarandi <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/14 23:42:52 by asarandi          #+#    #+#             */
/*   Updated: 2019/06/25 00:17:30 by sgardner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

int			fatal_error(void *p, char *s)
{
	free(p);
	fprintf(stderr, "%s\n", s);
	return (1);
}

static int	set_key(t_woody *woody, const char *arg)
{
	static uint64_t	key;
	char			*found;
	int				i;

	if (*arg && *arg == '0' && *(arg + 1) && tolower(*(arg + 1)) == 'x')
		arg += 2;
	i = -1;
	while (arg[++i])
	{
		if (i > 15 || !(found = strchr(BASE16, tolower(arg[i]))))
			return (0);
		key = (key * 16) + (found - BASE16);
	}
	if (i == 16)
	{
		woody->key = &key;
		return (1);
	}
	return (0);
}

static int	parse_flags(t_woody *woody, int ac, char *const av[])
{
	int opt;

	while ((opt = getopt(ac, av, "dk:")) != -1)
	{
		if (opt == 'k' && !set_key(woody, optarg))
		{
			fprintf(stderr, E_BADKEY);
			return (0);
		}
		else if (opt == 'd')
			woody->decrypt = 1;
		else if (opt == '?')
			return (0);
	}
	return (1);
}

static int	write_output(t_woody *woody, int decrypt)
{
	char	*fname;
	int		fd;
	ssize_t	bytes;

	fname = (decrypt) ? F_DEC : F_ENC;
	if ((fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0755)) == -1)
		return (0);
	bytes = write(fd, woody->data, woody->filesize);
	close(fd);
	return (bytes == woody->filesize);
}

int			main(int ac, char **av)
{
	t_woody		woody;

	memset(&woody, 0, sizeof(woody));
	if (ac < 2)
		return (fatal_error(0, E_USAGE));
	if (!parse_flags(&woody, ac, av))
		return (1);
	if (!load_file(&woody, av[optind]))
		return (fatal_error(0, E_READ));
	if (!is_valid_elf64(&woody))
		return (fatal_error(woody.data, E_INVALID));
	if (woody.decrypt)
	{
		if (decrypt_binary(&woody))
			return (1);
	}
	else if (create_encrypted_binary(&woody))
		return (1);
	if (!write_output(&woody, woody.decrypt))
		return (fatal_error(woody.data, E_WRITE));
	free(woody.data);
	return (0);
}
