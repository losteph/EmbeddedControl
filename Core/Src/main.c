/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Progetto Levitating Ball
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PRINT_PRESCALER 10  // Stampa ogni 10 cicli da 20ms = 200ms (5 Hz)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim6;

/* USER CODE BEGIN PV */
/* --- FLAG DI SINCRONIZZAZIONE (volatile perché modificate/lette tra ISR e main) --- */
volatile uint8_t task_50hz = 0;   // Alzata a 1 ogni 20ms dall'interrupt di TIM6
uint8_t print_flag = 0;           // Alzata a 1 ogni 10 cicli (200ms) per la telemetria
uint16_t print_counter = 0;       // Contatore secondario per rallentare le stampe

/* --- VARIABILI DI STATO DEL CONTROLLO --- */
volatile float setpoint_mm = 150.0f; // Altezza desiderata (modificabile da bottoni/potenziometro)
float current_height_mm = 0.0f;      // Altezza reale letta dal sensore ToF
float control_pwm = 0.0f;            // Azione di controllo calcolata dal PID

/*--- VARIABILI USILIARIE PER TEST --- */
uint16_t timer_seconds_count = 0; // Contatore per scandire i 5 secondi
uint32_t elapsed_seconds = 0;     // Tempo totale trascorso in secondi
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
void Read_Sensors(void);
void Compute_PID(void);
void Update_Actuators(void);
void Print_Telemetry(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Reindirizzamento di printf per funzionare sia su SWV/ITM sia su UART
int __io_putchar(int ch) {
    ITM_SendChar(ch);
    return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  /* --- 1. INIZIALIZZAZIONE E AVVIO SERVIZI (Prima del while) --- */

    // Qui inseriremo l'init del sensore VL53L1X (I2C) e l'avvio del PWM ventola

    // Avvio del Timer 6 in modalità Interrupt (scatta ogni 20 ms = 50 Hz)
    HAL_TIM_Base_Start_IT(&htim6);
    printf("--- Firmware Avviato: Timer a 50 Hz attivo ---\r\n"); //per test iniziale, poi cancelliamo.
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* Controllo ad anello chiuso a 50 Hz */
	  if (task_50hz == 1)
	      {
	          task_50hz = 0; // Reset immediato della flag

	          Read_Sensors();
	          Compute_PID();
	          Update_Actuators();

	          /*
	          // Base dei tempi secondaria per le stampe (10 cicli * 20ms = 200ms -> 5 Hz)
	          print_counter++;
	          if (print_counter >= 250) // 10 cicli * 20ms = 200ms
	          {
	              print_counter = 0;
	              print_flag = 1;
	          }
	          */
	          // Calcolo dei 5 secondi: 250 cicli da 20 ms = 5000 ms
	          timer_seconds_count++;
	          if (timer_seconds_count >= 250)
	          {
	        	  timer_seconds_count = 0;
	        	  elapsed_seconds += 5;

	        	  // Stampa di test visiva su console SWV
	        	  printf("[DEBUG 50Hz OK] Tempo trascorso: %lu s | LED commutato!\r\n", elapsed_seconds);
	          }
	      }
    /* USER CODE END WHILE */
  }
    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 8399;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 199;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* ============================================================================
 * 1. IMPLEMENTAZIONE DELLE FUNZIONI DELLO SCHELETRO (Per ora vuote/di test)
 * ============================================================================ */

void Read_Sensors(void)
{
    // TODO: Qui leggeremo la distanza da VL53L1X via I2C (e eventuale potenziometro)
}

void Compute_PID(void)
{
    // TODO: Qui chiameremo PID_Update() per calcolare control_pwm
}

void Update_Actuators(void)
{
    // TODO: Qui aggiorneremo il registro CCR del PWM ventola e i 2 LED (Verde/Rosso)
}

/* ============================================================================
 * 2. ROUTINE DI GESTIONE DEGLI INTERRUPT (ISR Callbacks - Brevissime!)
 * ============================================================================ */

/**
  * @brief  Callback del Timer a 50 Hz (scatta esattamente ogni 20 ms).
  *         Come da best-practice del corso, alza solo la flag ed esce subito!
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        task_50hz = 1;
    }
}

/**
  * @brief  Callback per gli Interrupt Esterni (Pulsanti fisici per il Setpoint).
  *         Isola l'input utente asincrono dalla logica di controllo.
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // TODO: Qui inseriremo l'incremento/decremento di setpoint_mm con debouncing
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
