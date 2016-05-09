#include <core/eos.h>
#include <core/eos_internal.h>

typedef struct _os_context {
	/* low address */
    addr_t s_edi;
    addr_t s_esi;
    addr_t s_ebp;
    addr_t s_ebx;
    addr_t s_edx;
    addr_t s_ecx;
    addr_t s_eax;
    addr_t s_eflags;
    addr_t s_eip;
	/* high address */
} _os_context_t;

void print_context(addr_t context) {
	if(context == NULL) return;
	_os_context_t *ctx = (_os_context_t *)context;
    PRINT("0x%x\n", ctx->s_edi);
    PRINT("0x%x\n", ctx->s_esi);
    PRINT("0x%x\n", ctx->s_ebp);
    PRINT("0x%x\n", ctx->s_ebx);
    PRINT("0x%x\n", ctx->s_edx);
    PRINT("0x%x\n", ctx->s_ecx);
    PRINT("0x%x\n", ctx->s_eax);
    PRINT("0x%x\n", ctx->s_eflags);
    PRINT("0x%x\n", ctx->s_eip);
}

addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg) {
    addr_t *sp = (addr_t *) (stack_base + stack_size);

    (*(--sp)) = (addr_t) arg;
    (*(--sp)) = (addr_t) NULL;
    (*(--sp)) = (addr_t) entry;
    (*(--sp)) = (addr_t) NULL;
    (*(--sp)) = (addr_t) NULL;
    (*(--sp)) = (addr_t) NULL;
    (*(--sp)) = (addr_t) NULL;
    (*(--sp)) = (addr_t) NULL;
    (*(--sp)) = (addr_t) NULL;
    (*(--sp)) = (addr_t) NULL;
    (*(--sp)) = (addr_t) NULL;

    return (addr_t) sp;
}

void _os_restore_context(addr_t sp) {
    __asm__ __volatile__ ("movl %0, %%esp;"::"m"(sp));
    __asm__ __volatile__ ("popl %%edi;"::);
    __asm__ __volatile__ ("popl %%esi;"::);
    __asm__ __volatile__ ("popl %%ebp;"::);
    __asm__ __volatile__ ("popl %%ebx;"::);
    __asm__ __volatile__ ("popl %%edx;"::);
    __asm__ __volatile__ ("popl %%ecx;"::);
    __asm__ __volatile__ ("popl %%eax;"::);
    __asm__ __volatile__ ("popfl;"::);
    __asm__ __volatile__ ("ret;"::);
}

addr_t _os_save_context() {
    __asm__ __volatile__("movl %0, %%eax;"::"n"(0));
    __asm__ __volatile__("pushl $resume_eip;"::);

    // save context
    __asm__ __volatile__ ("pushfl;"::);
    __asm__ __volatile__ ("pushl %%eax;"::);
    __asm__ __volatile__ ("pushl %%ecx;"::);
    __asm__ __volatile__ ("pushl %%edx;"::);
    __asm__ __volatile__ ("pushl %%ebx;"::);
    __asm__ __volatile__ ("pushl %%ebp;"::);
    __asm__ __volatile__ ("pushl %%esi;"::);
    __asm__ __volatile__ ("pushl %%edi;"::);

    __asm__ __volatile__("movl %%esp, %%eax;"::);
    __asm__ __volatile__("push 4(%%ebp);"::);
    __asm__ __volatile__("push 0(%%ebp);"::);
    __asm__ __volatile__("movl %%esp, %%ebp;"::);
    /* set resume point where the saved task would execute
       when it restore */
    __asm__ __volatile__("\
        resume_eip:\
            leave;\
            ret;"
        ::
    );
}
