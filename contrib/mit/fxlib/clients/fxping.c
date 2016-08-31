/**********************************************************************
 * File Exchange fxping client
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mitcopyright.h>.
 **********************************************************************/

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <mitcopyright.h>

 

#include <stdio.h>
#include <strings.h>
#include <fxcl.h>		/* overrides #include <time.h> */

main(argc, argv)
  int argc;
  char *argv[];
{
  FX fx;
  FX *fxp;
  long code;
  stringlist node;
  server_stats *stats;
  struct tm *tim;

  /* Initialization needed for com_err routines */
  initialize_fxcl_error_table();
  initialize_rpc_error_table();
  initialize_fxsv_error_table();
  initialize_krb_error_table();
/*  fxp = fx_open("", &code);
  if (code) com_err(argv[0], code, "while connecting.");

  if (!fxp) exit(1);
 */
  fxp = &fx;
  bzero(fx, sizeof(fx));
  for (node = fx_host_list(); node != NULL; node = node->next) {
    strcpy(fx.host, node->s);
    code = fx_connect(&fx);
    if (!code) fx_stat(fxp, &stats);
    fx_close(fxp);
    if (code) {
      printf("%16s down: %s\n", fx.host, error_message(code));
      continue;
    }

    tim = (struct tm *)gmtime(&stats->uptime);
    printf("%16s up %d days, %02d:%02d:%02d, DB %ld/%ld\n", fxp->host,
	   tim->tm_yday, tim->tm_hour, tim->tm_min, tim->tm_sec,
	   stats->vers.synctime, stats->vers.commit);
  }
  exit(0);
}
