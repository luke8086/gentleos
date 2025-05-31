// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: interrupt.c - Interrupt handling
// --------------------------------------------------------------------------------------

#include <kernel.h>

static isr_handler_fn krn_interrupt_handlers[64] = { NULL };

void
krn_interrupt_handle(isr_stack_st *isr_stack)
{
    if (krn_interrupt_handlers[isr_stack->int_no]) {
        krn_interrupt_handlers[isr_stack->int_no](isr_stack);
    }
}

void
krn_interrupt_set_handler(uint8_t int_no, isr_handler_fn handler)
{
    krn_interrupt_handlers[int_no] = handler;
}
