/*
 * Copyright PolySat, California Polytechnic State University, San Luis Obispo. cubesat@calpoly.edu
 * This file is part of libproc, a PolySat library.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Test code for process command handling.
 *
 * Don't input any lines over BUF_LEN or a buffer overrun will occur.
 *
 * @author Greg Eddington
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <polysat/polysat.h>
#include <polysat/cmd-pkt.h>
#include "test_schema.h"

struct ProcessData *gProc = NULL;

static void status_cmd_handler(struct ProcessData *proc,
      struct IPC_Command *cmd,
      struct sockaddr_in *fromAddr, void *arg, int fd);
static void params_cmd_handler(struct ProcessData *proc,
      struct IPC_Command *cmd,
      struct sockaddr_in *fromAddr, void *arg, int fd);

//maps generated command numbers to c functions
struct XDR_CommandHandlers handlers[] = {
   { IPC_CMDS_STATUS, &status_cmd_handler, NULL},
   { IPC_TEST_COMMANDS_PTEST, &params_cmd_handler, NULL},
   { 0, NULL, NULL }
};

static void status_cmd_handler(struct ProcessData *proc, struct IPC_Command *cmd,
      struct sockaddr_in *fromAddr, void *arg, int fd)
{
    //status struct to send back
   struct IPC_TEST_Status status;
   struct IPC_TEST_DictTest val1, val2, val3;
   int ival1 = 987, ival2 = 654, ival3 = 321, ival4 = 12345;

   memset(&status, 0, sizeof(status));
   status.foo = 123;
   status.bar = 464;

   val1.field1 = 234;
   val1.field2 = 345;
   val1.str = "hello world";

   val2.field1 = 456;
   val2.field2 = 567;
   val2.str = "goodbye world";

   val3.field1 = 678;
   val3.field2 = 789;
   val3.str = "polysat";

   XDR_dict_add(&status.values, "zxcvbnm", &val1);
   XDR_dict_add(&status.values, "asdfghjk", &val2);
   XDR_dict_add(&status.values, "qwertyui", &val3);

   XDR_dict_add(&status.intdict, "aaaabbbbccccdddd", &ival1);
   XDR_dict_add(&status.intdict, "eeeeffffgggghhhh", &ival2);
   XDR_dict_add(&status.intdict, "iiiijjjjkkkkllll", &ival3);
   XDR_dict_add(&status.intdict, "mmmmnnnnoooopppp", &ival4);

   status.test.field1 = 3333;
   status.test.field2 = 999999;
   status.test.str = "go mustangs";
    
    //pass enum to associate with struct being sent
   IPC_response(proc, cmd, IPC_TEST_DATA_TYPES_STATUS, &status, fromAddr);

   XDR_dict_remove_all(&status.values, NULL, NULL);
   printf("Status command!!\n");
}

static void params_cmd_handler(struct ProcessData *proc, struct IPC_Command *cmd,
      struct sockaddr_in *fromAddr, void *arg, int fd)
{
   struct IPC_TEST_PTest *params = (struct IPC_TEST_PTest*)cmd->parameters.data;

   if (params)
      printf("v1=%d, v2=%d, v3=%d, v4=%d\n", params->val1, params->val2, params->val3, params->val4);

   IPC_response(proc, cmd, IPC_TYPES_VOID, NULL, fromAddr);
   printf("Params command!!\n");
}

// Simple SIGINT handler example
int sigint_handler(int signum, void *arg)
{
   DBG("SIGINT handler!\n");
   EVT_exit_loop(PROC_evt(arg));
   return EVENT_KEEP;
}

int main(int argc, char *argv[])
{
   gProc = PROC_init_xdr("test1", WD_DISABLED, handlers);

   // Add a signal handler call back for SIGINT signal
   DBG_setLevel(DBG_LEVEL_ALL);
   PROC_signal(gProc, SIGINT, &sigint_handler, gProc);

   EVT_start_loop(PROC_evt(gProc));

   return 0;
}
