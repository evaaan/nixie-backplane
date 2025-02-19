/*
* @author Evan Reeves
* @ date 2019-05-14
*/

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "math.h"

uint16_t idx;
uint16_t delay, delay_start, delay_inc, delay_end; 
uint16_t delay_inc;
uint16_t delay_end;
uint32_t total_delay;
uint32_t g_ADCValue = 0;
uint32_t g_MeasurementNumber;
uint32_t t_on = 0;
uint32_t t_off = 0;
uint32_t tick = 0;
uint32_t ms_counter = 0;
uint8_t my_val;
#define __PERIOD 7

extern uint32_t tick;
float freq;
float delay_f;
volatile time clock;

GPIO_InitTypeDef GPIO_InitStruct;
ADC_HandleTypeDef g_AdcHandle;

int main(void) {
  /* Initialize HAL Library. */
  HAL_Init();
  
  /* Initialize GPIOs */
  GPIO_Init();
  
  /* Initialize system clock and ADC */
  SystemClock_Config();
  ConfigureADC();
  HAL_ADC_Start(&g_AdcHandle);
  
  clock.hours = 0;
  clock.minutes = 0;
  clock.seconds = 0;
  freq = 3;
  delay_inc = 0;
  delay_start = 1000;
  delay_end = 1000;
  
  delay = delay_start;
  update_display();
  inc_seconds();
  
  
	while(1) {
    // brightness_control();
    // main_loop();
    // dev_loop();
    listen();
    
	}
}

void main_loop(void) {
  // inc_seconds();
  // update_display();
  listen();
}

void listen(void) {
  if (sec_pressed()) {
    clock.seconds += 1;
    update_display();
    HAL_Delay(200);
  }
  if (min_pressed()) {
    clock.minutes += 1;
    update_display();
    HAL_Delay(200);
  }
  if (hour_pressed()) {
    clock.hours += 1;
    update_display();
    HAL_Delay(200);
  }
}

/* Set all digits to the same value 0-9 */
void set_all(uint8_t val) {
  uint8_t digits = val * 10 + val;
  set_seconds(digits);
  set_minutes(digits);
  set_hours(digits);
}

void dev_loop(void) {
  uint8_t i;
  for (i=0; i < 10; i++) {
    set_all(i);
    HAL_Delay(200);
  }
}

void dev_loop2(void) {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, gpio_state(0x00));
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, gpio_state(0x00));
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, gpio_state(0x00));
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, gpio_state(0x00));
  HAL_Delay(delay_start);
}

/* Update Nixie Tubes */
void update_display(void) {
  set_seconds(clock.seconds);
  set_minutes(clock.minutes);
  set_hours(clock.hours);
}

/* Convert integer to binary-coded decimal (ex: 54 to 0x54)*/
uint8_t int2bcd(uint8_t val) {
  return ((val / 10) << 4) | (val % 10);
}

void SysTick_Handler(void)
{
  HAL_IncTick();
  ms_counter += 1;
  if (ms_counter >= 1000) {
    tick = 1;
    ms_counter = 0;
    clock.seconds += 1;
    update_display();
  }
}

/* Time update methods */
void inc_seconds(void) {
  if (clock.seconds >= 59) {
    clock.seconds = 0;
    inc_minutes();
  } else {
    clock.seconds += 1;
  }
}

void inc_minutes(void) {
  if (clock.minutes >= 59) {
    clock.minutes = 0;
    inc_hours();
  } else {
    clock.minutes += 1;
  }
}

void inc_hours(void) {
  if (clock.hours >= 12) {
    clock.hours = 1;
  } else {
    clock.hours += 1;
  }
}

uint8_t sec_pressed(void) {
  return (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET);
}

uint8_t min_pressed(void) {
  return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET);
}

uint8_t hour_pressed(void) {
  return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET);
}

GPIO_PinState gpio_state(uint8_t val) {
  if (val == 0x00) {
    return GPIO_PIN_RESET;
  } else {
    return GPIO_PIN_SET;
  }
}

void set_seconds(uint8_t val) {
  uint8_t bcd, ones, tens;
  bcd = int2bcd(val);
  ones = bcd & 0x0F;
  tens = (bcd & 0xF0) >> 4;
  my_val = ones;
    
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, gpio_state(ones & 0x01));
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, gpio_state(ones & 0x02));
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, gpio_state(ones & 0x04));
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, gpio_state(ones & 0x08));
  
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, gpio_state(tens & 0x01));
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, gpio_state(tens & 0x02));
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, gpio_state(tens & 0x04));
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, gpio_state(tens & 0x08));
}

void set_minutes(uint8_t val) {
  uint8_t bcd, ones, tens;
  bcd = int2bcd(val);
  ones = bcd & 0x0F;
  tens = (bcd & 0xF0) >> 4;
    
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, ones & 0x01);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, ones & 0x02);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, ones & 0x04);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, ones & 0x08);
  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, tens & 0x01);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, tens & 0x02);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, tens & 0x04);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, tens & 0x08);
}

void set_hours(uint8_t val) {
  uint8_t bcd, ones, tens;
  bcd = int2bcd(val);
  ones = bcd & 0x0F;
  tens = (bcd & 0xF0) >> 4;
    
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, ones & 0x01);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, ones & 0x02);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, ones & 0x04);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, ones & 0x08);
  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, tens & 0x01);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, tens & 0x02);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, tens & 0x04);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, tens & 0x08);
}


void GPIO_Init(void) {
  
  /* General Config */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  
  
  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  
  /* S1A - PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* S1B - PE8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
  /* S1C - PE10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
  /* S1D - PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* S10A - PE11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  /* S10B - PE7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  /* S10C - PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
  /* S10D - PE9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  /* M1A - PE12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  /* M1B - PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
  /* M1C - PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
  /* M1D - PE14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* M10A - PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* M10B - PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
  /* M10C - PD0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
  /* M10D - PD6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /* H1A - PD8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /* H1B - PD3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
  /* H1C - PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
  /* H1D - PD1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /* H10A - PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* H10B - PE6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
  /* H10C - PE5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
  /* H10D - PE4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  /* PWM OUTPUT - PA1 
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); */
  
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  
  /* SET_S - PE15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  /* SET_M - PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
  /* SET_H - PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
  
  
}

/* PWM bit-bang for brightness circuit testing */
void brightness_control(void) {
    if (HAL_ADC_PollForConversion(&g_AdcHandle, 1000000) == HAL_OK) {
      g_ADCValue = HAL_ADC_GetValue(&g_AdcHandle);
      g_MeasurementNumber++;
    }
    t_on = g_ADCValue >> 9; // Convert 0-4095 to 0-7
    t_off = __PERIOD - t_on;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 1);
    HAL_Delay(t_on);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);
    HAL_Delay(t_off);
}

static void SystemClock_Config(void)
{
  /* Based on example from https://visualgdb.com/tutorials/arm/stm32/adc/ */
  
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 288;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 6;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  SystemCoreClockUpdate();
  
  if (HAL_GetREVID() == 0x1001)
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
}
 
void ConfigureADC(void)
{
  /* Based on example from https://visualgdb.com/tutorials/arm/stm32/adc/ */
  
  GPIO_InitTypeDef gpioInit;
  
  __GPIOC_CLK_ENABLE();
  __ADC1_CLK_ENABLE();
  
  gpioInit.Pin = GPIO_PIN_1;
  gpioInit.Mode = GPIO_MODE_ANALOG;
  gpioInit.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &gpioInit);
  
  HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ADC_IRQn);
  
  ADC_ChannelConfTypeDef adcChannel;
  
  g_AdcHandle.Instance = ADC1;
  
  g_AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
  g_AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
  g_AdcHandle.Init.ScanConvMode = DISABLE;
  g_AdcHandle.Init.ContinuousConvMode = ENABLE;
  g_AdcHandle.Init.DiscontinuousConvMode = DISABLE;
  g_AdcHandle.Init.NbrOfDiscConversion = 0;
  g_AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  g_AdcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
  g_AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  g_AdcHandle.Init.NbrOfConversion = 1;
  g_AdcHandle.Init.DMAContinuousRequests = ENABLE;
  g_AdcHandle.Init.EOCSelection = DISABLE;
  
  HAL_ADC_Init(&g_AdcHandle);
  
  adcChannel.Channel = ADC_CHANNEL_11;
  adcChannel.Rank = 1;
  adcChannel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  adcChannel.Offset = 0;
  
  if (HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel) != HAL_OK)
  {
    asm("bkpt 255");
  }
}
