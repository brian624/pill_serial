/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file implements the platform specific functions for the ST-Link
 * implementation.
 */

#include "general.h"
#include "cdcacm.h"
#include "usbuart.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/adc.h>

uint8_t running_status = true; 

uint16_t led_idle_run;
static uint32_t rev;

int platform_hwversion(void)
{
	return rev;
}

/* return 0  no built in st-link on bluepill */
uint32_t detect_rev(void)
{
	uint32_t rev = 0;

	while (RCC_CFGR & 0xf) /* Switch back to HSI. */
		RCC_CFGR &= ~3;
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_USB);
	rcc_periph_reset_pulse(RST_USB);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_CRC);


	return rev;
}

void platform_request_boot(void)
{
	uint32_t crl = GPIOA_CRL;
	/* Assert bootloader marker.
	 * Enable Pull on GPIOA1. We don't rely on the external pin
	 * really pulled, but only on the value of the CNF register
	 * changed from the reset value
	 */
	crl &= 0xffffff0f;
	crl |= 0x80;
	GPIOA_CRL = crl;
	SCB_VTOR = 0;
}

void platform_init(void)
{
	rev = detect_rev();
	SCS_DEMCR |= SCS_DEMCR_VC_MON_EN;
#ifdef ENABLE_DEBUG
	void initialise_monitor_handles(void);
	initialise_monitor_handles();
#endif
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
	
	led_idle_run = GPIO13;
	

	gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, led_idle_run);

	/* Relocate interrupt vector table here */
	extern int vector_table;
	SCB_VTOR = (uint32_t)&vector_table;

	platform_timing_init();
	cdcacm_init();
	/* Don't enable UART if we're being debugged. */
	if (!(SCS_DEMCR & SCS_DEMCR_TRCENA))
		usbuart_init();
}

static volatile uint32_t time_ms;

void platform_timeout_set(platform_timeout *t, uint32_t ms)
{
	t->time = platform_time_ms() + ms;
}

bool platform_timeout_is_expired(platform_timeout *t)
{
	return platform_time_ms() > t->time;
}

void platform_timing_init(void)
{
	/* Setup heartbeat timer */
	systick_set_frequency(10,72000000);
	//systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	//systick_set_reload(900000);	/* Interrupt us at 10 Hz */
	SCB_SHPR(11) &= ~((15 << 4) & 0xff);
	SCB_SHPR(11) |= ((14 << 4) & 0xff);
	systick_interrupt_enable();
	systick_counter_enable();
}

void platform_delay(uint32_t ms)
{
	platform_timeout timeout;
	platform_timeout_set(&timeout, ms);
	while (!platform_timeout_is_expired(&timeout));
}

void sys_tick_handler(void)
{
	if(running_status)
		gpio_toggle(LED_PORT, LED_IDLE_RUN);

	time_ms += 100;
}

uint32_t platform_time_ms(void)
{
	return time_ms;
}

