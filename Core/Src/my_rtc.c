/*
 * my_rtc.c
 *
 *  Created on: Jan 29, 2026
 *      Author: User
 */
#include <stdio.h>

#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "my_rtc.h"

extern RTC_HandleTypeDef hrtc;

uint32_t Get_RTC_Current_Time(void){
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	uint32_t current_time = 0;

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	current_time |=(sTime.Hours   << 16);
	current_time |=(sTime.Minutes << 8);
	current_time |=(sTime.Seconds << 0);

	return current_time;
}

