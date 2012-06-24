/*
 * This module allow the +d mode on a channel
 * When activated, only halfops and ops can use
 * colors on the chan.
 * Others are stripped.
 */

#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "proto.h"
#include "channel.h"
#include "h.h"

#ifdef _WIN32
#include <io.h>
#include "version.h"
#endif

#ifdef STRIPBADWORDS
#include "badwords.h"
#endif

Cmode_t NOCOLOR_BLOCK = 0L;
Cmode *ModeBlock = NULL;

ModuleHeader MOD_HEADER(m_nocaps)
  = {
	"m_nocolor",
	"1.1",
	"Block color",
	"3.2-b8-1",
	NULL 
    };

static ModuleInfo NoColorModInfo;
static Hook *CheckMsg;

DLLFUNC char *nocolor_checkmsg(aClient *, aClient *, aChannel *, char *, int);

DLLFUNC int MOD_INIT(m_nocolor)(ModuleInfo *modinfo)
{
    ModuleSetOptions(modinfo->handle, MOD_OPT_PERM);
    CmodeInfo req;
    ircd_log(LOG_ERROR, "debug: mod_init called from m_nocolor");
    sendto_realops("loading m_nocolor");
    memset(&req, 0, sizeof(req));
    req.paracount = 0;
    req.is_ok = extcmode_default_requirechop;
    req.flag = 'd';
    ModeBlock = CmodeAdd(modinfo->handle, req, &NOCOLOR_BLOCK);

    bcopy(modinfo,&NoColorModInfo,modinfo->size);
    CheckMsg = HookAddPCharEx(NoColorModInfo.handle, HOOKTYPE_CHANMSG, nocolor_checkmsg);
    return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_nocolor)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_nocolor)(int module_unload)
{
	HookDel(CheckMsg);
	CmodeDel(ModeBlock);
        ircd_log(LOG_ERROR, "debug: mod_unload called from m_nocolor");
	sendto_realops("unloading m_nocolor");
	return MOD_SUCCESS;
}

DLLFUNC char *nocolor_checkmsg(aClient *cptr, aClient *sptr, aChannel *chptr, char *text, int notice)
{
	if (IsULine(sptr) || IsServer(sptr)) {
		return text;
	}

	if (chptr->mode.extmode & NOCOLOR_BLOCK && !is_chan_op(sptr, chptr) && !is_halfop(sptr, chptr))
	{
		return StripColors(text);		    
	}
	else {
		return text;
	}    
}
