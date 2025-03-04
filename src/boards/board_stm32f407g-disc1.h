/*
 * Copyright (c) 2023 Marcel Licence
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
 *
 * Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
 * veröffentlichten Version, weiter verteilen und/oder modifizieren.
 *
 * Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
 * OHNE JEDE GEWÄHR,; sogar ohne die implizite
 * Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Einzelheiten.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
 */

/**
 * @file board_stm32f407g-disc1.h
 * @author Marcel Licence
 * @date 21.12.2021
 *
 * @brief Board description for STM32F4DISCOVERY STM32F407G-DISC1 ST STM32 STM32F4 STM32F407 MCU Discovery ARM Cortex-M4 Development Board Kit
 *
 * @see https://www.st.com/en/evaluation-tools/stm32f4discovery.html
 * @see https://www.st.com/resource/en/data_brief/stm32f4discovery.pdf
 */


#ifndef BOARDS_BOARD_STM32F407G_DISC1_H_
#define BOARDS_BOARD_STM32F407G_DISC1_H_


/* todo!!!! */
#define LED_USER_ORANGE PD13
#define LED_USER_GREEN  PD12
#define LED_USER_RED    PD14
#define LED_USER_BLUE   PD15

#define B1_USER PA0

#define I2S_I2SN    SPI3 // Using SPI3 for I2S
#define I2S_MCLK    PC7 // I2S3_MCK
#define I2S_SCLK    PC10 // I2S3_CK
#define I2S_SDIN    PC12 // I2S3_SD mcu out -> dac in
#define I2S_LRCK    PA4 // I2S3_WS

#define DAC_RESET   PD4
#define AUDIOCODEC_CS PD4

/* cs44l22 codec */
#define I2C_SCL PB6 // I2C1_SCL
#define I2C_SDA PB9 // I2C1_SDA

/* for gyro but does not work with arduino */
#define I2C_GYRO_SCL    PA9 // SPI1_SCK
#define I2C_GYRO_SDA    PA7 // SPI1_MOSI


#endif /* BOARDS_BOARD_STM32F407G_DISC1_H_ */
