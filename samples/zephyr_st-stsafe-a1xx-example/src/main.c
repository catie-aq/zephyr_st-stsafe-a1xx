/*
 * Copyright (c) 2026, CATIE
 * SPDX-License-Identifier: Apache-2.0
 *
 * Multi-threaded STSAFE-A1xx example.
 *
 * Two threads hammer the same STSAFE instance concurrently via the
 * locked API (stsafe_acquire / stsafe_release). Each thread runs an
 * echo round-trip in a loop and checks that the reply matches what it
 * sent. The per-instance mutex inside the driver serialises the actual
 * I2C traffic so the two threads can't step on each other's APDUs.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include <drivers/stsafe.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define STACK_SIZE      2048
#define PRIO            5
#define ECHO_LEN        8
#define ITERATIONS      10
#define ACQUIRE_TIMEOUT K_MSEC(1000)

static const struct device *const se = DEVICE_DT_GET(DT_NODELABEL(stsafe_1_20));

struct echo_worker {
	const char *name;
	uint8_t pattern;
	k_tid_t tid;
	int errors;
};

static void echo_thread(void *p1, void *p2, void *p3)
{
	struct echo_worker *w = p1;

	uint8_t tx[ECHO_LEN];
	uint8_t rx[ECHO_LEN];

	memset(tx, w->pattern, sizeof(tx));

	for (int i = 0; i < ITERATIONS; i++) {
		stse_Handle_t *stse_handle = stsafe_acquire(se, ACQUIRE_TIMEOUT);
		if (!stse_handle) {
			LOG_ERR("[%s] acquire timed out on iter %d", w->name, i);
			w->errors++;
			continue;
		}

		memset(rx, 0, sizeof(rx));
		int ret = stse_device_echo(stse_handle, tx, rx, sizeof(rx));

		stsafe_release(se);

		if (ret != STSE_OK) {
			LOG_ERR("[%s] iter %d: echo failed (0x%x)", w->name, i, ret);
			w->errors++;
			continue;
		}

		if (memcmp(tx, rx, sizeof(tx)) != 0) {
			LOG_ERR("[%s] iter %d: echo mismatch", w->name, i);
			w->errors++;
			continue;
		}

		LOG_INF("[%s] iter %d OK", w->name, i);

		k_sleep(K_MSEC(50));
	}

	LOG_INF("[%s] done, %d error(s)", w->name, w->errors);
}

static K_THREAD_STACK_DEFINE(stack_a, STACK_SIZE);
static K_THREAD_STACK_DEFINE(stack_b, STACK_SIZE);
static struct k_thread thread_a;
static struct k_thread thread_b;

static struct echo_worker worker_a = {.name = "A", .pattern = 0xAA};
static struct echo_worker worker_b = {.name = "B", .pattern = 0xBB};

int main(void)
{
	LOG_INF("************************************************************");
	LOG_INF("   STSAFE-A1xx multi-threaded example (acquire/release)");
	LOG_INF("************************************************************");

	if (!device_is_ready(se)) {
		LOG_ERR("STSAFE device not ready");
		return -ENODEV;
	}
	LOG_INF("STSAFE device ready: %s", se->name);

	worker_a.tid = k_thread_create(&thread_a, stack_a, STACK_SIZE, echo_thread, &worker_a, NULL,
				       NULL, PRIO, 0, K_NO_WAIT);
	k_thread_name_set(&thread_a, "echo-A");

	worker_b.tid = k_thread_create(&thread_b, stack_b, STACK_SIZE, echo_thread, &worker_b, NULL,
				       NULL, PRIO, 0, K_NO_WAIT);
	k_thread_name_set(&thread_b, "echo-B");

	k_thread_join(&thread_a, K_FOREVER);
	k_thread_join(&thread_b, K_FOREVER);

	int total_errors = worker_a.errors + worker_b.errors;
	LOG_INF("All workers finished, total errors: %d", total_errors);

	return total_errors == 0 ? 0 : -1;
}
