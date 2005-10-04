/*
 * Copyright (c) 2005 William Pitcock, et al.
 * Rights to this code are as documented in doc/LICENSE.
 *
 * This file contains code for the CService INFO functions.
 *
 * $Id: info.c 2551 2005-10-04 06:14:07Z nenolod $
 */

#include "atheme.h"

DECLARE_MODULE_V1
(
	"chanserv/info", FALSE, _modinit, _moddeinit,
	"$Id: info.c 2551 2005-10-04 06:14:07Z nenolod $",
	"Atheme Development Group <http://www.atheme.org>"
);

static void cs_cmd_info(char *origin);

command_t cs_info = { "INFO", "Displays information on registrations.",
                        AC_NONE, cs_cmd_info };

list_t *cs_cmdtree;
list_t *cs_helptree;

void _modinit(module_t *m)
{
	cs_cmdtree = module_locate_symbol("chanserv/main", "cs_cmdtree");
	cs_helptree = module_locate_symbol("chanserv/main", "cs_helptree");

        command_add(&cs_info, cs_cmdtree);
	help_addentry(cs_helptree, "INFO", "help/cservice/info", NULL);
}

void _moddeinit()
{
	command_delete(&cs_info, cs_cmdtree);
	help_delentry(cs_helptree, "INFO");
}

static void cs_cmd_info(char *origin)
{
	user_t *u = user_find(origin);
	myuser_t *mu;
	mychan_t *mc;
	char *name = strtok(NULL, " ");
	char buf[BUFSIZE], strfbuf[32];
	struct tm tm;
	metadata_t *md;

	if (!name)
	{
		notice(chansvs.nick, origin, "Insufficient parameters specified for \2INFO\2.");
		notice(chansvs.nick, origin, "Syntax: INFO <#channel>");
		return;
	}

	if (*name != '#')
	{
		notice(chansvs.nick, origin, "Invalid parameters specified for \2INFO\2.");
		notice(chansvs.nick, origin, "Syntax: INFO <#channel>");
		return;
	}

	if (!(mc = mychan_find(name)))
	{
		notice(chansvs.nick, origin, "\2%s\2 is not registered.", name);
		return;
	}

	tm = *localtime(&mc->registered);
	strftime(strfbuf, sizeof(strfbuf) - 1, "%b %d %H:%M:%S %Y", &tm);

	notice(chansvs.nick, origin, "Information on \2%s\2", mc->name);

	if (LIST_LENGTH(&mc->founder->logins))
		notice(chansvs.nick, origin, "Founder    : %s (logged in)", mc->founder->name);
	else
		notice(chansvs.nick, origin, "Founder    : %s (not logged in)", mc->founder->name);

	if (mc->successor)
	{
		if (LIST_LENGTH(&mc->successor->logins))
			notice(chansvs.nick, origin, "Successor  : %s (logged in)", mc->successor->name);
		else
			notice(chansvs.nick, origin, "Successor  : %s (not logged in)", mc->successor->name);
	}

	notice(chansvs.nick, origin, "Registered : %s (%s ago)", strfbuf, time_ago(mc->registered));

	if (mc->mlock_on || mc->mlock_off)
	{
		char params[BUFSIZE];

		*buf = 0;
		*params = 0;

		if (mc->mlock_on)
		{
			strcat(buf, "+");
			strcat(buf, flags_to_string(mc->mlock_on));

			/* add these in manually */
			if (mc->mlock_limit)
			{
				strcat(buf, "l");
				strcat(params, " ");
				strcat(params, itoa(mc->mlock_limit));
			}

			if (mc->mlock_key)
				strcat(buf, "k");
		}

		if (mc->mlock_off)
		{
			strcat(buf, "-");
			strcat(buf, flags_to_string(mc->mlock_off));
		}

		if (*buf)
			notice(chansvs.nick, origin, "Mode lock  : %s %s", buf, (params) ? params : "");
	}


	if ((md = metadata_find(mc, METADATA_CHANNEL, "url")))
		notice(chansvs.nick, origin, "URL        : %s", md->value);

	*buf = '\0';

	if (MC_HOLD & mc->flags)
		strcat(buf, "HOLD");

	if (MC_SECURE & mc->flags)
	{
		if (*buf)
			strcat(buf, " ");

		strcat(buf, "SECURE");
	}

	if (MC_VERBOSE & mc->flags)
	{
		if (*buf)
			strcat(buf, " ");

		strcat(buf, "VERBOSE");
	}

	if (MC_STAFFONLY & mc->flags)
	{
		if (*buf)
			strcat(buf, " ");

		strcat(buf, "STAFFONLY");
	}

	if ((is_ircop(u) || is_sra(u->myuser)) && (md = metadata_find(mc, METADATA_CHANNEL, "private:mark:setter")))
	{
		char *setter = md->value;
		char *reason;
		time_t ts;

		md = metadata_find(mc, METADATA_CHANNEL, "private:mark:reason");
		reason = md->value;

		md = metadata_find(mc, METADATA_CHANNEL, "private:mark:timestamp");
		ts = atoi(md->value);

		tm = *localtime(&ts);
		strftime(strfbuf, sizeof(strfbuf) - 1, "%b %d %H:%M:%S %Y", &tm);

		notice(chansvs.nick, origin, "%s was \2MARKED\2 by %s on %s (%s)", mc->name, setter, strfbuf, reason);
	}

	if ((is_ircop(u) || is_sra(u->myuser)) && (md = metadata_find(mc, METADATA_CHANNEL, "private:close:closer")))
	{
		char *setter = md->value;
		char *reason;
		time_t ts;

		md = metadata_find(mc, METADATA_CHANNEL, "private:close:reason");
		reason = md->value;

		md = metadata_find(mc, METADATA_CHANNEL, "private:close:timestamp");
		ts = atoi(md->value);

		tm = *localtime(&ts);
		strftime(strfbuf, sizeof(strfbuf) - 1, "%b %d %H:%M:%S %Y", &tm);

		notice(chansvs.nick, origin, "%s was \2CLOSED\2 by %s on %s (%s)", mc->name, setter, strfbuf, reason);
	}

	if (*buf)
		notice(chansvs.nick, origin, "Flags      : %s", buf);
}
