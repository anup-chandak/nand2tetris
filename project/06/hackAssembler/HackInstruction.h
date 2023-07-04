#ifndef HACKINSTRUCTION_H_
#define HACKINSTRUCTION_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "HackTools.h"

typedef struct HackInstruction HackInstruction;
struct HackInstruction {
    // line Number or Address of instruction in ROM
    uint32_t lineNumber;
    HackInstruction* next;

    // Concat these from top to bottom to get
    // instruction from LSB[15] to MSB[0]
    bool instruct[16];
    /* 1 wide : A or C instruction [0]
     * 2 wide : 1s of C instruction [1-2]
     * 1 wide : A or M register in ALU [3]
     * 6 wide : ALU control bits [4-9]
     * 3 wide : destination bits [10:A ; 11:D ; 12:M]
     * 3 wide : jump bits [13:<0 ; 14:0 ; 15:>0]
     */
};

// Sets the instruction instr to be an A instruction pointing to address
// Note, address is a 15-bit uint, so an in16_t is perfect to check for
// errors and there's no need to use a 16-bit unsigned int
void set_AInstruction(HackInstruction* instr, int16_t address);

// Sets the instruction instr to be a C instruction using dest(ination),
// comp(utation) and jump (condition).
// Any of the strings may be empty, but the caller has to make sure at least
// one string is not.
// Also the string in dest, comp and jump must be uppercase without any space
void set_CInstruction(HackInstruction* instr, char* dest, char* comp,
                      char* jump);

void printInstruction(HackInstruction* instr);

#endif  // HACKINSTRUCTION_H_
