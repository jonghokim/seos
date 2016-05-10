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
    addr_t sp = stack_base + stack_size;

    addr_t s_eip = (addr_t)entry;
    addr_t s_ebp = sp;
    addr_t s_null = (addr_t)NULL;

    /* push a new context */
    sp -= 4;
    *(addr_t *)sp = s_eip;
    sp -= 4;
    *(addr_t *)sp = s_null;
    sp -= 4;
    *(addr_t *)sp = s_null;
    sp -= 4;
    *(addr_t *)sp = s_null;
    sp -= 4;
    *(addr_t *)sp = s_null;
    sp -= 4;
    *(addr_t *)sp = s_null;
    sp -= 4;
    *(addr_t *)sp = s_ebp;
    sp -= 4;
    *(addr_t *)sp = s_null;
    sp -= 4;
    *(addr_t *)sp = s_null;

    return sp;
}

void _os_restore_context(addr_t sp) {
    __asm__ __volatile__ ("movl %0, %%esp"::"m"(sp));

    /* pop GPRs and eflags */
    __asm__ __volatile__ ("\
        popl %%edi;\
        popl %%esi;\
        popl %%ebp;\
        popl %%ebx;\
        popl %%edx;\
        popl %%ecx;\
        popl %%eax;\
        popl _eflags;"
        ::
    );

    /* transfer control to $resume_eip
       which is declared at _os_save_context()
       or a function if the context hasn't been stored before */
    __asm__ __volatile__("ret;"::);
}

addr_t _os_save_context() {
    /* set %eax to zero  */
    __asm__ __volatile__("movl %0, %%eax;"::"n"(0));

    /* push resume point where the task whoose this context
       would be resumed when this context is restored */
    __asm__ __volatile__("push $resume_eip;"::);

    /* push GPRs and eflags */
    __asm__ __volatile__ ("\
        push _eflags;\
        push %%eax;\
        push %%ecx;\
        push %%edx;\
        push %%ebx;\
        push %%ebp;\
        push %%esi;\
        push %%edi;"
        ::
    );

    /* set return value as %esp */
    __asm__ __volatile__("movl %%esp, %%eax;"::);

    /* push old %ebp and %eip then set %ebp to %esp
       so that original old %ebp and %eip could be used
       when this context are being restored */
    __asm__ __volatile__("\
        push 4(%%ebp);\
        push 0(%%ebp);\
        movl %%esp, %%ebp;"
        ::
    );

    /* set resume point where the saved task would execute
       when it restore */
    __asm__ __volatile__("\
        resume_eip:\
            leave;\
            ret;"
        ::
    );
}
