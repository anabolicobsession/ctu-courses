#include "ec.h"
#include "ptab.h"
#include "string.h"

const mword BREAK_ERROR = 0;
const mword BREAK_MAX = 0xbffff000;
const char FILL_BYTE = 0;

typedef enum {
    sys_print      = 1,
    sys_sum        = 2,
    sys_break      = 3,
    sys_thr_create = 4,
    sys_thr_yield  = 5,
} Syscall_numbers;

mword page_floor(mword page)
{
    return page - page % PAGE_SIZE;
}

mword page_ceil(mword page)
{
    return page_floor(page) + PAGE_SIZE * (page % PAGE_SIZE > 0);
}

mword decrease_break(mword break_old, mword break_new)
{
    for (mword virt = page_ceil(break_old) - PAGE_SIZE; virt >= page_floor(break_new); virt -= PAGE_SIZE)
    {
        mword phys = Ptab::get_mapping(virt);
        phys = page_floor(phys); // get rid of attributes
        void *kernel_virt = Kalloc::phys2virt(phys);

        if (virt < page_ceil(break_new)) { // then we need to set to zero some part of page
            // void* type restricts arithmetic, but char* doesn't
            kernel_virt = static_cast<void*>(static_cast<char*>(kernel_virt) + break_new % PAGE_SIZE);
            memset(kernel_virt, FILL_BYTE, PAGE_SIZE - break_new % PAGE_SIZE);
            break;
        }

        mword attr = 0;
        if (!Ptab::insert_mapping(virt, phys, attr)) {
            printf("insert_mapping() failure while memory deallocation\n");
            return virt;
        }

        Kalloc::allocator.free_page(kernel_virt);
    }

    return break_new;
}

mword increase_break(mword break_old, mword break_new)
{
    mword attr = Ptab::PRESENT | Ptab::USER | Ptab::RW;
    bool mem_failure = false;

    for (mword virt = page_ceil(break_old); virt < page_ceil(break_new); virt += PAGE_SIZE)
    {
        void *kernel_virt = Kalloc::allocator.alloc_page(1, Kalloc::FILL_0);

        if (kernel_virt) {
            mword phys = Kalloc::virt2phys(kernel_virt);
            if (!Ptab::insert_mapping(virt, phys, attr)) {
                Kalloc::allocator.free_page(kernel_virt);
                mem_failure = true;
            }
        }
        else {
            mem_failure = true;
        }

        if (mem_failure) {
            printf("alloc_page() or insert_mapping() failure while memory allocation\n");
            return decrease_break(virt, break_old);
        }
    }

    return break_new;
}

void Ec::syscall_handler (uint8 a)
{
    // Get access to registers stored during entering the system - see
    // entry_sysenter in entry.S
    Sys_regs * r = current->sys_regs();
    Syscall_numbers number = static_cast<Syscall_numbers> (a);

    switch (number) {
        case sys_print: {
            char *data = reinterpret_cast<char*>(r->esi);
            unsigned len = r->edi;
            for (unsigned i = 0; i < len; i++)
                printf("%c", data[i]);
            break;
        }
        case sys_sum: {
            // Naprosto nepotřebné systémové volání na sečtení dvou čísel
            int first_number = r->esi;
            int second_number = r->edi;
            r->eax = first_number + second_number;
            break;
        }
        case sys_break: {
            mword addr = r->esi;

            if (addr == 0) {
                r->eax = break_current;
            }
            else if (addr < break_min || addr > BREAK_MAX) {
                r->eax = BREAK_ERROR;
            }
            else if (addr == break_current) {
                r->eax = break_current;
            }
            else {
                r->eax = break_current;
                break_current = addr > break_current ? increase_break(break_current, addr) : decrease_break(break_current, addr);
                if (break_current != addr) r->eax = BREAK_ERROR;
            }

            break;
        }
        default: {
            printf ("unknown syscall %d\n", number);
            break;
        }
    };

    ret_user_sysexit();
}
