#include <core/eos.h>
#include <core/eos_internal.h>

typedef struct _os_context {
	/* low address */
    addr_t _edi;
    addr_t _esi;
    addr_t _ebp;
    addr_t _ebx;
    addr_t _edx;
    addr_t _ecx;
    addr_t _eax;
    addr_t _eflags;
    addr_t _eip;
	/* high address */
} _os_context_t;

void print_context(addr_t context) {
	if(context == NULL) return;
	_os_context_t *ctx = (_os_context_t *)context;
    PRINT("reg edi:  0x%x\n", ctx->_edi);
    PRINT("reg esi:  0x%x\n", ctx->_esi);
    PRINT("reg ebp:  0x%x\n", ctx->_ebp);
    PRINT("reg ebx:  0x%x\n", ctx->_ebx);
    PRINT("reg edx:  0x%x\n", ctx->_edx);
    PRINT("reg ecx:  0x%x\n", ctx->_ecx);
    PRINT("reg eax:  0x%x\n", ctx->_eax);
    PRINT("reg eflags:  0x%x\n", ctx->_eflags);
    PRINT("reg eip:  0x%x\n", ctx->_eip);
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
    __asm__ __volatile__ ("movl %0, %%esp;"::"m"(sp));
    __asm__ __volatile__ ("popl %%edi;"::);
    __asm__ __volatile__ ("popl %%esi;"::);
    __asm__ __volatile__ ("popl %%ebp;"::);
    __asm__ __volatile__ ("popl %%ebx;"::);
    __asm__ __volatile__ ("popl %%edx;"::);
    __asm__ __volatile__ ("popl %%ecx;"::);
    __asm__ __volatile__ ("popl %%eax;"::);
    __asm__ __volatile__ ("popl _eflags;"::);
    __asm__ __volatile__ ("ret;"::);
}

addr_t _os_save_context() {
    __asm__ __volatile__("movl %0, %%eax;"::"n"(0));
    __asm__ __volatile__("push $resume_point;"::);

    // save context
    __asm__ __volatile__ ("push _eflags;"::);
    __asm__ __volatile__ ("push %%eax;"::);
    __asm__ __volatile__ ("push %%ecx;"::);
    __asm__ __volatile__ ("push %%edx;"::);
    __asm__ __volatile__ ("push %%ebx;"::);
    __asm__ __volatile__ ("push %%ebp;"::);
    __asm__ __volatile__ ("push %%esi;"::);
    __asm__ __volatile__ ("push %%edi;"::);

    __asm__ __volatile__("movl %%esp, %%eax;"::);
    __asm__ __volatile__("push 4(%%ebp);"::);
    __asm__ __volatile__("push 0(%%ebp);"::);
    __asm__ __volatile__("movl %%esp, %%ebp;"::);
    /* set resume point where the saved task would execute
       when it restore */
    __asm__ __volatile__("\
        resume_point:\
            leave;\
            ret;"
        ::
    );
}
