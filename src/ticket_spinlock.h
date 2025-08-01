#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "cpu_relax/cpu_relax.h"
#include "threading/threading.h"

#define MAX_PAUSE_ITERATIONS 40

typedef struct {
    atomic_uint_fast16_t ticket;
    atomic_uint_fast16_t now_serving;
} ticket_spinlock_t;

static inline void ticket_spinlock_init(ticket_spinlock_t *lock) {
    atomic_init(&lock->ticket, 0);
    atomic_init(&lock->now_serving, 0);
}

static void ticket_spinlock_lock(ticket_spinlock_t *lock) {
    uint16_t ticket = atomic_fetch_add_explicit(&lock->ticket, 1, memory_order_acq_rel);

    for (size_t i = 0; i < MAX_PAUSE_ITERATIONS; i++) {
        if (atomic_load_explicit(&lock->now_serving, memory_order_acquire) == ticket) return;
        cpu_relax();
    }
    while (atomic_load_explicit(&lock->now_serving, memory_order_acquire) != ticket) {
        thrd_yield();
    }
}

static void ticket_spinlock_unlock(ticket_spinlock_t *lock) {
    atomic_fetch_add_explicit(&lock->now_serving, 1, memory_order_release);
}

static bool ticket_spinlock_trylock(ticket_spinlock_t *lock) {
    uint16_t ticket = atomic_load_explicit(&lock->ticket, memory_order_relaxed);
    uint16_t now_serving = atomic_load_explicit(&lock->now_serving, memory_order_relaxed);
    if (ticket != now_serving) return false;

    uint16_t next_ticket = ticket + 1;

    return atomic_compare_exchange_strong(&lock->ticket, &ticket, next_ticket);
}

static bool ticket_spinlock_is_locked(ticket_spinlock_t *lock) {
    uint16_t ticket = atomic_load(&lock->ticket);
    uint16_t now_serving = atomic_load(&lock->now_serving);
    return now_serving != ticket;
}

#endif /* SPINLOCK_H */
