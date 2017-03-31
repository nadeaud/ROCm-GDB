/*
   ROCm GDB commands for GNU debugger GDB.

   Copyright (c) 2015-2016 ADVANCED MICRO DEVICES, INC.  All rights reserved.
   This file includes code originally published under

   Copyright (C) 1986-2014 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#if !defined (HSAIL_CMD_H)
#define HSAIL_CMD_H 1

/* The header files shared with the agent*/
#include "CommunicationControl.h"

void hsail_command_clear_argument_buff(void);

/* Command for printing hsail source within gdb's terminal
 * */
void hsail_cmd_list_command(char* arg, int from_tty);

/* Clear the focus at the end of the dispatch */
void hsail_cmd_clear_focus(void);

/* Function to set the focus wave sent from the Agent */
void hsail_cmd_set_focus(HsailWaveDim3 focusWg, HsailWaveDim3 focusWi);

bool hsail_command_get_argument(const char* cmdInfo, const char* argName, unsigned int* itemDim3, int* numItems);

bool hsail_cmd_get_show_isa_option(void);

void mi_hsail_thread_info (char *command, char **argv, int argc);

void mi_hsail_wave_group (char *command, char **argv, int argc);

void mi_hsail_focus_selection (char *command, char **argv, int argc);

void mi_hsail_work_item_list (char *command, char **argv, int argc);

bool check_wave_in_focus (HsailAgentWaveInfo *wave);

/*bool check_wg_focus_x (uint32_t xId);

bool check_wg_focus_y (uint32_t xId);

bool check_wg_focus_z (uint32_t xId);
*/
#endif

