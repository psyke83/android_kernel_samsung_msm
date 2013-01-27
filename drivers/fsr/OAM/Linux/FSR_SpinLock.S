/* =============================================================================
 *
 *  FUNCTION smem_flash_spin_lock
 *
 *  DESCRIPTION
 *    Acquire a spin lock as indicated by input integer.
 *
 * DEPENDENCIES
 *   Depends on smem_spin_lock_init being called to initialize the pointers
 *   used in this routine.
 *   Depends on MMU being configured as non-cached, non-buffered for the
 *   shared memory.
 *
 * PROTOTYPE
 *   void smem_flash_spin_lock(int)
 *
 * RETURN VALUE
 *   None
 *
 * SIDE EFFECTS
 *   The BUS (is it EBI1 or EBI2?) is locked while SWP instruction executes.
 *
 * =============================================================================
 */    
 .global  smem_flash_spin_lock
smem_flash_spin_lock:

    stmfd   sp!, {r0-r4, lr}
    mov     r4, r0

    add     r1, r1, r4, lsl #2

    #ifdef IMAGE_APPS_PROC
    mov     r2, #0x01                  /* PID or "owner" */
    #else
    mov     r2, #0x02                  /* PID or "owner" */
    #endif

tryit2:
    swp     r3, r2, [r1]               /* Check how the land is laying */
    cmp     r3, #0                     /* Is the lock free? */
    bne     tryit2

    ldmfd   sp!, {r0-r4, pc}     /* restore trashed registers including r14 */

/*
 * =============================================================================
 *
 *  FUNCTION smem_flash_spin_unlock
 *
 *  DESCRIPTION
 *    Free a spin lock as indicated by input integer.
 *
 *  DEPENDENCIES
 *    Depends on sec_smem_spin_lock_init being called to initialize the pointers
 *    used in this routine.
 *    Depends on MMU being configured as non-cached, non-buffered for the
 *    shared memory.
 *
 *  PROTOTYPE
 *    void smem_flash_spin_unlock(int)
 *
 *  RETURN VALUE
 *    None
 *
 *  SIDE EFFECTS
 *    The BUS (is it EBI1 or EBI2?) is locked while SWP instruction executes.
 *
 * =============================================================================
 */

    .global  smem_flash_spin_unlock
smem_flash_spin_unlock:

	stmfd   sp!, {r0-r3, lr}
    mov     r3, r0

    add     r1,  r1,  r3, lsl #2


    /* release spin lock */
    mov     r2, #0
    str     r2, [r1]

    ldmfd   sp!, {r0-r3, pc}

