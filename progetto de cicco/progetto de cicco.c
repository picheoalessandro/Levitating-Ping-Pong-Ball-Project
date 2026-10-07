/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Test collegamento VL53L0X tramite I2C
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "vl53l0x_api.h"
#include <string.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/*
 * Indirizzo I2C del VL53L0X.
 *
 * L'indirizzo a 7 bit è 0x29.
 * La HAL STM32 richiede l'indirizzo spostato di un bit:
 *
 * 0x29 << 1 = 0x52
 */
#define VL53L0X_ADDRESS_7_BIT    0x29
#define VL53L0X_ADDRESS_HAL      (VL53L0X_ADDRESS_7_BIT << 1)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/*
 * Queste variabili possono essere osservate durante il debug.
 */
volatile uint8_t sensore_trovato = 0;
volatile uint32_t errore_i2c = 0;

/* Messaggio inviato tramite UART. */
char messaggio_uart[100];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

void UART_InviaStringa(const char *testo);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief Invia una stringa tramite USART2.
  * @param testo Stringa da trasmettere.
  */
void UART_InviaStringa(const char *testo)
{
    HAL_UART_Transmit(
        &huart2,
        (uint8_t *)testo,
        strlen(testo),
        HAL_MAX_DELAY
    );
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* Reset delle periferiche e inizializzazione HAL. */
    HAL_Init();

    /* Configurazione del clock di sistema. */
    SystemClock_Config();

    /* Inizializzazione delle periferiche configurate in CubeMX. */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    /* USER CODE BEGIN 2 */

    UART_InviaStringa("\r\n");
    UART_InviaStringa("Avvio test VL53L0X\r\n");
    UART_InviaStringa("------------------\r\n");

    /*
     * Attesa iniziale per permettere al sensore
     * di completare l'accensione.
     */
    HAL_Delay(100);

    /*
     * Verifica se il sensore risponde sul bus I2C.
     *
     * Parametri:
     * - &hi2c1: periferica I2C utilizzata;
     * - VL53L0X_ADDRESS_HAL: indirizzo 0x52;
     * - 3: numero di tentativi;
     * - 100: timeout in millisecondi.
     */
    if (HAL_I2C_IsDeviceReady(
            &hi2c1,
            VL53L0X_ADDRESS_HAL,
            3,
            100
        ) == HAL_OK)
    {
        sensore_trovato = 1;
        errore_i2c = 0;

        UART_InviaStringa(
            "VL53L0X trovato all'indirizzo 0x29\r\n"
        );
    }
    else
    {
        sensore_trovato = 0;
        errore_i2c = HAL_I2C_GetError(&hi2c1);

        snprintf(
            messaggio_uart,
            sizeof(messaggio_uart),
            "VL53L0X non trovato. Errore I2C: %lu\r\n",
            (unsigned long)errore_i2c
        );

        UART_InviaStringa(messaggio_uart);
    }

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    while (1)
    {
        /*
         * Ripetiamo il controllo ogni secondo.
         * In questo modo possiamo anche verificare se il sensore
         * viene scollegato o ricollegato.
         */

        if (HAL_I2C_IsDeviceReady(
                &hi2c1,
                VL53L0X_ADDRESS_HAL,
                3,
                100
            ) == HAL_OK)
        {
            sensore_trovato = 1;
            errore_i2c = 0;

            UART_InviaStringa("Sensore collegato\r\n");
        }
        else
        {
            sensore_trovato = 0;
            errore_i2c = HAL_I2C_GetError(&hi2c1);

            snprintf(
                messaggio_uart,
                sizeof(messaggio_uart),
                "Sensore non raggiungibile. Errore: %lu\r\n",
                (unsigned long)errore_i2c
            );

            UART_InviaStringa(messaggio_uart);
        }

        HAL_Delay(1000);
    }

    /* USER CODE END WHILE */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /*
     * Configurazione del regolatore interno.
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    /*
     * Configurazione dell'oscillatore HSI e del PLL.
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

    /*
     * Configurazione dei clock di CPU e bus.
     */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_2
        ) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief Funzione eseguita in caso di errore.
  * @retval None
  */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief Reports the name of the source file and the source line number.
  */
void assert_failed(uint8_t *file, uint32_t line)
{
}

#endif
