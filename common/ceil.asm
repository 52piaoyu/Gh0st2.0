.globl _ceil

.intel_syntax noprefix

/* FUNCTIONS ***************************************************************/

_ceil:
        push    ebp
        mov     ebp,esp
        sub     esp,4                   // Allocate temporary space
        fld     qword ptr [ebp+8]       // Load real from stack
        fstcw   [ebp-2]                 // Save control word
        fclex                           // Clear exceptions
        mov     word ptr [ebp-4],0xb63  // Rounding control word
        fldcw   [ebp-4]                 // Set new rounding control
        frndint                         // Round to integer
        fclex                           // Clear exceptions
        fldcw   [ebp-2]                 // Restore control word
        mov     esp,ebp                 // Deallocate temporary space
        pop     ebp
        ret
