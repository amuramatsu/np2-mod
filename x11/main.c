/*
 * Copyright (c) 2003 NONAKA Kimihiro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "compiler.h"

#include <getopt.h>
#include <signal.h>

#include "np2.h"
#include "diskdrv.h"
#include "dosio.h"
#include "ini.h"
#include "parts.h"
#include "pccore.h"
#include "s98.h"
#include "scrndraw.h"
#include "serial.h"
#include "timing.h"
#include "toolkit.h"

#include "keydisp.h"
#include "toolwin.h"

#include "commng.h"
#include "joymng.h"
#include "kbdmng.h"
#include "mousemng.h"
#include "scrnmng.h"
#include "soundmng.h"
#include "sysmng.h"


/*
 * resume
 */
static const char np2resumeext[] = "sav";


/*
 * failure signale handler
 */
typedef void sigfunc(int);

static sigfunc *
setup_signal(int signo, sigfunc *func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(signo, &act, &oact) < 0)
		return SIG_ERR;
	return oact.sa_handler;
}

static void
sighandler(int signo)
{

	UNUSED(signo);

	toolkit_widget_quit();
}


/*
 * option
 */
static struct option longopts[] = {
	{ "config",		required_argument,	0,	'c' },
	{ "timidity-config",	required_argument,	0,	'C' },
	{ "help",		no_argument,		0,	'h' },
	{ 0,			0,			0,	0   },
};

static char* progname;

static void
usage(void)
{

	printf("Usage: %s [options] [[FD0 image] [[FD1 image] [[FD2 image] [FD3 image]]]]\n", progname);
	exit(1);
}


/*
 * main
 */
int
main(int argc, char *argv[])
{
	struct stat sb;
	BOOL result;
	int rv = 1;
	int ch;
	int i, drvmax;

	progname = argv[0];

	toolkit_initialize();
	toolkit_arginit(&argc, &argv);

	while ((ch = getopt_long(argc, argv, "c:C:h", longopts, NULL)) != -1) {
		switch (ch) {
		case 'c':
			if (stat(optarg, &sb) < 0 || !S_ISREG(sb.st_mode)) {
				fprintf(stderr, "Can't access %s.\n", optarg);
				exit(1);
			}
			milstr_ncpy(modulefile, optarg, sizeof(modulefile));

			/* resume/statsave dir */
			file_cpyname(statpath, modulefile, sizeof(statpath));
			file_cutname(statpath);
			break;

		case 'C':
			if (stat(optarg, &sb) < 0 || !S_ISREG(sb.st_mode)) {
				fprintf(stderr, "Can't access %s.\n", optarg);
				exit(1);
			}
			milstr_ncpy(timidity_cfgfile_path, optarg,
			    sizeof(timidity_cfgfile_path));
			break;

		case 'h':
		case '?':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (modulefile[0] == '\0') {
		char *env = getenv("HOME");
		if (env) {
			/* base dir */
			snprintf(modulefile, sizeof(modulefile),
			    "%s/.np2", env);
			if (stat(modulefile, &sb) < 0) {
				if (mkdir(modulefile, 0700) < 0) {
					perror(modulefile);
					exit(1);
				}
			} else if (!S_ISDIR(sb.st_mode)) {
				fprintf(stderr, "%s isn't directory.\n",
				    modulefile);
				exit(1);
			}

			/* font file */
			snprintf(np2cfg.fontfile, sizeof(np2cfg.fontfile),
			    "%s/font.bmp", modulefile);

			/* resume/statsave dir */
			file_cpyname(statpath, modulefile, sizeof(statpath));
			file_catname(statpath, "/sav/", sizeof(statpath));
			if (stat(statpath, &sb) < 0) {
				if (mkdir(statpath, 0700) < 0) {
					perror(statpath);
					exit(1);
				}
			} else if (!S_ISDIR(sb.st_mode)) {
				fprintf(stderr, "%s isn't directory.\n",
				    statpath);
				exit(1);
			}

			/* config file */
			milstr_ncat(modulefile, "/np2rc", sizeof(modulefile));
			if ((stat(modulefile, &sb) >= 0)
			 && !S_ISREG(sb.st_mode)) {
				fprintf(stderr, "%s isn't regular file.\n",
				    modulefile);
			}
		}
	}

	dosio_init();
	file_setcd(modulefile);
	initload();
	toolwin_readini();
	keydisp_readini();

	rand_setseed((SINT32)time(NULL));

#if defined(__GNUC__) && (defined(i386) || defined(__i386__))
	mmxflag = havemmx() ? 0 : MMXFLAG_NOTSUPPORT;
	mmxflag += np2oscfg.disablemmx ? MMXFLAG_DISABLE : 0;
#endif

	TRACEINIT();

	keystat_reset();

	toolkit_widget_create();
	scrnmng_initialize();
	kbdmng_init();

	scrnmode = 0;
	if (np2cfg.RASTER) {
		scrnmode |= SCRNMODE_HIGHCOLOR;
	}
	if (scrnmng_create(scrnmode) != SUCCESS)
		goto resource_cleanup;

	if (soundmng_initialize() == SUCCESS) {
		result = soundmng_pcmload(SOUND_PCMSEEK, file_getcd("fddseek.wav"));
		if (!result) {
			result = soundmng_pcmload(SOUND_PCMSEEK, SYSRESPATH "/fddseek.wav");
		}
		if (result) {
			soundmng_pcmvolume(SOUND_PCMSEEK, np2cfg.MOTORVOL);
		}

		result = soundmng_pcmload(SOUND_PCMSEEK1, file_getcd("fddseek1.wav"));
		if (!result) {
			soundmng_pcmload(SOUND_PCMSEEK1, SYSRESPATH "/fddseek1.wav");
		}
		if (result) {
			soundmng_pcmvolume(SOUND_PCMSEEK1, np2cfg.MOTORVOL);
		}
	}

	mousemng_initialize();
	if (np2oscfg.MOUSE_SW) {
		mouse_running(MOUSE_ON);
	}

	commng_initialize();
	sysmng_initialize();

	joy_init();
	pccore_init();
	S98_init();

	toolkit_widget_show();
	scrndraw_redraw();

	pccore_reset();

	if (!(scrnmode & SCRNMODE_FULLSCREEN)) {
		if (np2oscfg.toolwin) {
			toolwin_create();
		}
		if (np2oscfg.keydisp) {
			keydisp_create();
		}
	}

	if (np2oscfg.resume) {
		flagload(np2resumeext, "Resume", FALSE);
	}
	sysmng_workclockreset();

	drvmax = (argc < 4) ? argc : 4;
	for (i = 0; i < drvmax; i++) {
		milstr_ncpy(diskdrv_fname[i],argv[i],sizeof(diskdrv_fname[0]));
		diskdrv_delay[i] = 1;
	}

	setup_signal(SIGINT, sighandler);
	setup_signal(SIGTERM, sighandler);

	np2running = TRUE;
	toolkit_widget_mainloop();
	np2running = FALSE;
	rv = 0;

	keydisp_destroy();
	toolwin_destroy();

	pccore_cfgupdate();

	mouse_running(MOUSE_OFF);
	S98_trash();

	if (np2oscfg.resume) {
		flagsave(np2resumeext);
	} else {
		flagdelete(np2resumeext);
	}

	pccore_term();

	soundmng_deinitialize();
	scrnmng_destroy();

resource_cleanup:
	if (sys_updates & (SYS_UPDATECFG|SYS_UPDATEOSCFG)) {
		initsave();
		toolwin_writeini();
		keydisp_writeini();
	}

	TRACETERM();
	dosio_term();

	return rv;
}