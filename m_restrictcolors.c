/*
 *   IRC - Internet Relay Chat, m_restrictcolors.c
 *   (C) 2012 CrazyCat
 *
 * This module allow the +W mode on a channel
 * When activated, only halfops and ops can use
 * colors on the chan.
 * Others are stripped.
 */

#include "config.h"
#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "channel.h"
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include "h.h"
#include "proto.h"
#ifdef STRIPBADWORDS
#include "badwords.h"
#endif
#ifdef _WIN32
#include "version.h"
#endif

Cmode_t RESTRICT_COLORS = 0L;
Cmode *ModeRcolors = NULL;

ModuleHeader MOD_HEADER(restrictcolors)
 = {
   "m_restrictcolors",
   "1.0",
   "color stripping for non-(half)op",
   "3.2-b8-1",
   NULL
};

static ModuleInfo RestrictColorsModInfo;
static Hook *CheckMsg;

DLLFUNC char *restrictcolors_checkmsg(aClient *, aClient *, aChannel *, char *, int);

DLLFUNC int MOD_INIT(m_restrictcolors)(ModuleInfo *modinfo)
{
    ModuleSetOptions(modinfo->handle, MOD_OPT_PERM);
    CmodeInfo req;
    ircd_log(LOG_ERROR, "debug: mod_init called from m_restrictcolors");
    sendto_realops("loading m_restrictcolors");
    memset(&req, 0, sizeof(req));
    req.paracount = 0;
    req.is_ok = extcmode_default_requirehalfop;
    req.flag = 'W';
    ModeRcolors = CmodeAdd(modinfo->handle, req, &RESTRICT_COLORS);

    bcopy(modinfo,&RestrictColorsModInfo,modinfo->size);
    CheckMsg = HookAddPCharEx(RestrictColorsModInfo.handle, HOOKTYPE_CHANMSG, restrictcolors_checkmsg);
    return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_restrictcolors)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_restrictcolors)(int module_unload)
{
	HookDel(CheckMsg);
	CmodeDel(ModeRcolors);
   ircd_log(LOG_ERROR, "debug: mod_unload called from m_restrictcolors");
	sendto_realops("unloading m_restrictcolors");
	return MOD_SUCCESS;
}


DLLFUNC char *restrictcolors_checkmsg(aClient *cptr, aClient *sptr, aChannel *chptr, char *text, int notice)
{
    if (IsULine(sptr) || IsServer(sptr))
       return text;

    if (chptr->mode.extmode && RESTRICT_COLORS && !is_chan_op(sptr, chptr) && !is_halfop(sptr, chptr))
       return StripColors(text);

    return text;
}
