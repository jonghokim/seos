#include <core/eos.h>

#define WORK_STACK_SIZE 8192

static int8u_t stack1[WORK_STACK_SIZE];    // stack size = WORK_STACK_SIZE bytes
static int8u_t stack2[WORK_STACK_SIZE];
static int8u_t stack3[WORK_STACK_SIZE];

static eos_tcb_t tcb1;
static eos_tcb_t tcb2;
static eos_tcb_t tcb3;

static void task1() {
    while (1) {
        PRINT("A\n");
        eos_sleep(0);
    }
}

static void task2() {
    while (1) {
        PRINT("B\n");
        eos_sleep(0);
    }
}

static void task3() {
    while (1) {
        PRINT("C\n");
        eos_sleep(0);
    }
}

void eos_user_main() {
    eos_create_task(&tcb1, (addr_t)stack1, WORK_STACK_SIZE, task1, NULL, 1);
    eos_set_period(&tcb1, 2);

    eos_create_task(&tcb2, (addr_t)stack2, WORK_STACK_SIZE, task2, NULL, 10);
    eos_set_period(&tcb2, 4);

    eos_create_task(&tcb3, (addr_t)stack3, WORK_STACK_SIZE, task3, NULL, 50);
    eos_set_period(&tcb3, 8);
}
