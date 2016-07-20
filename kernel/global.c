/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "func.h"
#include "global.h"
#include "process.h"


PUBLIC  PROCESS         proc_table[NR_TASKS];

PUBLIC  char            task_stack[STACK_SIZE_TOTAL];

PUBLIC  TASK task_table[NR_TASKS] = {
    {TestA, STACK_SIZE_TESTA, "TestA"},
    {TestB, STACK_SIZE_TESTB, "TestB"}
};

