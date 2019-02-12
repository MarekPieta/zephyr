/*
 * Copyright (c) 2019, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr.h>
#include <gpio.h>

#define CALC_THREAD_PRIORITY K_PRIO_PREEMPT(2)
#define CALC_THREAD_STACK_SIZE 800

#define SHOW_RES_THREAD_PRIORITY K_PRIO_COOP(1)
#define SHOW_RES_THREAD_STACK_SIZE 400

static K_THREAD_STACK_DEFINE(calc_thread_stack, CALC_THREAD_STACK_SIZE);
static K_THREAD_STACK_DEFINE(show_res_thread_stack, SHOW_RES_THREAD_STACK_SIZE);

static struct k_thread calc_thread;
static struct k_thread show_res_thread;
static const char calc_thread_name[] = "Calc_thread";
static const char show_res_thread_name[] = "Show_res_thread";

static struct device       *gpio_dev;

static struct gpio_callback gpio_cb;
static const  u32_t         pin_id = 13;

static void busy_wait(u32_t time_ms)
{
	u32_t start_time = k_uptime_get_32();
	u32_t cur_time = k_uptime_get_32();
	while (cur_time - start_time < time_ms) { 
		cur_time = k_uptime_get_32();
	}
}

static void button_pressed(struct device *gpio_dev, struct gpio_callback *cb,
		    	   u32_t pins)
{
	printk("Pressed button \n");
	u32_t readout = 0;
	while (!readout) {
		gpio_pin_read(gpio_dev, pin_id, &readout);
	}
	printk("Released button \n");
}

static void calc_thread_fn(void)
{
	char descr[300] = "Calc_thread started";
	
	printk("%s", descr);	
	while(1) {
		busy_wait(40);
		k_sleep(160);
	}
}

static void show_res_thread_fn(void)
{
	while(1) {
		busy_wait(10);
		k_sleep(190);
	}
}

void main(void)
{
	k_tid_t calc_tid = k_thread_create(&calc_thread, 
					   calc_thread_stack,
					   CALC_THREAD_STACK_SIZE,
				 	   (k_thread_entry_t)calc_thread_fn,
					   NULL, NULL, NULL,
					   CALC_THREAD_PRIORITY,
					   K_USER,
					   K_NO_WAIT);
	k_thread_name_set(calc_tid, calc_thread_name);
	k_tid_t show_res_tid = k_thread_create(&show_res_thread,
					       show_res_thread_stack,
					       SHOW_RES_THREAD_STACK_SIZE,
					       (k_thread_entry_t)show_res_thread_fn,
					       NULL, NULL, NULL,
					       SHOW_RES_THREAD_PRIORITY,
					       K_USER,
					       K_NO_WAIT);
	k_thread_name_set(show_res_tid, show_res_thread_name);

	gpio_dev = device_get_binding(SW0_GPIO_CONTROLLER);
	gpio_pin_configure(gpio_dev, pin_id, GPIO_PUD_PULL_UP | GPIO_DIR_IN |
			   GPIO_INT | GPIO_INT_EDGE | GPIO_INT_ACTIVE_LOW);
	gpio_init_callback(&gpio_cb, button_pressed, BIT(pin_id));
	
	gpio_add_callback(gpio_dev, &gpio_cb);
	gpio_pin_enable_callback(gpio_dev, pin_id);

	while(1) {
		k_sleep(50);
	}
}
