/* Copyright 2017 Gerry Agbobada
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _VMTWRITER_H_
#define _VMTWRITER_H_
#ifndef __STDC_WANT_LIB_EXT2__
#define __STDC_WANT_LIB_EXT2__ 1
#endif  // __STDC_WANT_LIB_EXT2__
#define ASM_LINE_BUFFER_SIZE 33
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dict/vmTDictFiles.h"
#include "vmTTools.h"

// Struct that keeps the count of label-powered translations to
// produce unique labels during one file translation.
typedef struct LabelCounter {
    int nb_all;
    int nb_return;
} LabelCounter;

void LC_reset_return_counter(LabelCounter* p_lc);
void LC_init(LabelCounter* p_lc);

/* Choose the correct asm file for the translation
 * this function is neither responsible for malloc nor freeing memory
 * for command[i] arrays.
 * Giving the number of words in the command helps the choosing
 * The returned string should NOT be modified, and is ine of the strings in
 * vmTDictFiles.h
 */
const char* choose_asm_dict_file(VMCommand* p_cmd, int command_len);

/* Main writer function :
 * filestream : output filestream
 * command : originally parsed command, used to get command[2]
 * p_labelCounter : pointer to LabelCounter struct to make unique labels
 * asm_stub : COPY (caller responsiblity) of the XXX_asm string in vmTDictFiles
 * asm_stub_number : int version of command[2] when applicable, negative
 * otherwise
 * basename : basename of the vm file to produce unique static labels
 *
 * I = replaced by command[2]
 * J = replaced by p_labelCounter->nb_all
 * K = replaced by THIS or THAT if command[2] is 0 or 1
 * L = replaced by p_labelCounter->nb_return
 *
 * Also resets the nb_return counter if command[0] is return
 */
void write_to_file(FILE* filestream, const VMCommand* p_cmd,
                   LabelCounter* p_labelCounter, const char* asm_stub,
                   char* basename, char* staticName);

#endif  // _VMTWRITER_H_
