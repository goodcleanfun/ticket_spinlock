#include <stdio.h>
#include <stdlib.h>

#include "greatest/greatest.h"
#include "threading/threading.h"

#include "ticket_spinlock.h"

typedef struct {
    int counter;
    ticket_spinlock_t lock;
} ticket_lock_test_t;

#ifndef NUM_THREADS
#define NUM_THREADS 16
#endif
#ifndef NUM_PUSHES
#define NUM_PUSHES 100000
#endif


int increment_counter(void *arg) {
    ticket_lock_test_t *test = (ticket_lock_test_t *)arg;
    for (int i = 0; i < NUM_PUSHES; i++) {
        ticket_spinlock_lock(&test->lock);
        test->counter++;
        ticket_spinlock_unlock(&test->lock);
    }
    return 0;
}

TEST ticket_lock_multithread_test(void) {
    thrd_t threads[NUM_THREADS];

    ticket_lock_test_t *test = (ticket_lock_test_t *)malloc(sizeof(ticket_lock_test_t));

    test->counter = 0;
    ticket_spinlock_t lock;
    ticket_spinlock_init(&lock);
    test->lock = lock;

    for (int i = 0; i < NUM_THREADS; i++) {
        ASSERT_EQ(thrd_success, thrd_create(&threads[i], increment_counter, test));
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        thrd_join(threads[i], NULL);
    }

    ASSERT_EQ(NUM_THREADS * NUM_PUSHES, test->counter);

    free(test);

    PASS();
}


// Main test suite
SUITE(ticket_lock_tests) {
    RUN_TEST(ticket_lock_multithread_test);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(ticket_lock_tests);

    GREATEST_MAIN_END();
}