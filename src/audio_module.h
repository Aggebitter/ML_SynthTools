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
* @file audio_module.ino
* @author Marcel Licence
* @date 16.12.2021
*
* @brief  this file provides a general audio interface to make it easier working with different platforms
*/


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef ML_SYNTH_INLINE_DECLARATION


#include <ml_types.h>


void Audio_Setup(void);
void Audio_Output(const float *left, const float *right);
void Audio_OutputMono(const int32_t *samples);
void Audio_Output(const int16_t *left, const int16_t *right);
void Audio_Output(const Q1_14 *left, const Q1_14 *right);
void Audio_Input(float *left, float *right);
void DaisySeed_Setup(void);
void Teensy_Setup();
void SAMD21_Synth_Init();


#endif // #ifdef ML_SYNTH_INLINE_DECLARATION



#ifdef ML_SYNTH_INLINE_DEFINITION


#include <ml_types.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <I2S.h>
#include <i2s_reg.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#endif

#ifdef TEENSYDUINO
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#endif

#ifdef ARDUINO_DAISY_SEED
#include "DaisyDuino.h" /* requires the DaisyDuino library: https://github.com/electro-smith/DaisyDuino */
#endif

#if (defined ARDUINO_RASPBERRY_PI_PICO) || (defined ARDUINO_GENERIC_RP2040)
#include <I2S.h>

#ifdef RP2040_AUDIO_PWM
#include "RP2040_AudioPwm.h"

uint32_t WavPwmDataBuff[SAMPLE_BUFFER_SIZE];
uint32_t WavPwmDataBuff2[SAMPLE_BUFFER_SIZE];
#endif

#endif

void Audio_Setup(void)
{
#if (defined ESP8266) || (defined ESP32)
    WiFi.mode(WIFI_OFF);
#endif

#if 0 //ndef ESP8266
    btStop();
    esp_wifi_deinit();
#endif

#ifdef ESP32_AUDIO_KIT
#ifdef ES8388_ENABLED
    ES8388_Setup();
    ES8388_SetIn2OoutVOL(0, 0);
#else
    ac101_setup();
#endif
#endif


#ifdef WM8978_ENABLED
	WM8978_Setup();
#endif


#ifdef ESP32
    setup_i2s();
#endif

#ifdef ESP8266
#if 0
    system_update_cpu_freq(160);
    i2s_begin();
    i2s_set_rate(SAMPLE_RATE);
#else
    I2S_init();
#endif
    pinMode(2, INPUT); //restore GPIOs taken by i2s
    pinMode(15, INPUT);
#endif


#ifdef TEENSYDUINO
    AudioMemory(4);
#endif

#ifdef ARDUINO_SEEED_XIAO_M0
    SAMD21_Synth_Init();
    pinMode(DAC0, OUTPUT);
#endif

#if (defined ARDUINO_RASPBERRY_PI_PICO) || (defined ARDUINO_GENERIC_RP2040)
#ifdef RP2040_AUDIO_PWM
    uint8_t pwmPinNumber = 0;
    Serial.printf("Initialize pwm audio used without DAC pin %d + pin %d:\n", pwmPinNumber, pwmPinNumber + 1);
    Serial.printf("    sample rate: %d\n", SAMPLE_RATE);
    Serial.printf("    buffer size: %d\n", SAMPLE_BUFFER_SIZE);
    RP2040_Audio_Pwm_Init(pwmPinNumber, SAMPLE_RATE, WavPwmDataBuff, WavPwmDataBuff2, SAMPLE_BUFFER_SIZE);
#else
    if (!I2S.begin(SAMPLE_RATE))
    {
        Serial.println("Failed to initialize I2S!");
        while (1); // do nothing
    }
#endif
#endif

#if (defined ARDUINO_GENERIC_F407VGTX) || (defined ARDUINO_DISCO_F407VG)

    STM32_AudioInit();

#endif /* (defined ARDUINO_GENERIC_F407VGTX) || (defined ARDUINO_DISCO_F407VG) */
}

#ifdef TEENSYDUINO

const int ledPin = LED_PIN; /* pin configured in config.h */


AudioPlayQueue           queue1;
AudioPlayQueue           queue2;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(queue1, 0, i2s1, 0); /* left channel */
AudioConnection          patchCord2(queue2, 0, i2s1, 1); /* right channel */


static int16_t   sampleBuffer[AUDIO_BLOCK_SAMPLES];
static int16_t   sampleBuffer2[AUDIO_BLOCK_SAMPLES];
static int16_t   *queueTransmitBuffer;
static int16_t   *queueTransmitBuffer2;

void Teensy_Setup()
{
    pinMode(ledPin, OUTPUT);
    Midi_Setup();
}

#endif /* TEENSYDUINO */

#ifdef ARDUINO_DAISY_SEED

static DaisyHardware hw;
static size_t num_channels;

volatile static  bool dataReady = false;
static float out_temp[2][48];
static float *outCh[2] = {out_temp[0], out_temp[1]};

void MyCallback(float **in, float **out, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {

        out[0][i] = out_temp[0][i];
        out[1][i] = out_temp[1][i];
        out_temp[0][i] = in[0][i];
        out_temp[1][i] = in[1][i];
    }
    dataReady = true;
}

void DaisySeed_Setup(void)
{
    float sample_rate;
    // Initialize for Daisy pod at 48kHz
    hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
    num_channels = hw.num_channels;
    sample_rate = DAISY.get_samplerate();

    DAISY.begin(MyCallback);
}
#endif /* ARDUINO_DAISY_SEED */

#ifdef ARDUINO_SEEED_XIAO_M0

static int32_t u32buf[SAMPLE_BUFFER_SIZE];

inline
void ProcessAudio(uint16_t *buff, size_t len)
{
    /* convert from u16 to u10 */
    for (size_t i = 0; i < len; i++)
    {
        const int32_t preDiv = 4194304; // 2 ^ (16 + 6)
        buff[i] = (uint16_t)(0x200 + (u32buf[i] / (preDiv)));
    }
}

#endif

void Audio_OutputMono(const int32_t *samples)
{
#ifdef ESP8266
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        int32_t sig = samples[i];
        static uint16_t sig16 = 0;
        sig *= 4;
        sig16 = sig;
        while (!I2S_isNotFull())
        {
            /* wait for buffer is not full */
        }
        writeDAC(0x8000 + sig16);
    }
#endif

#ifdef ESP32
    float mono[SAMPLE_BUFFER_SIZE];
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        float sigf = samples[i];
        sigf /= INT16_MAX;
        mono[i] = sigf;
    }

    i2s_write_stereo_samples_buff(mono, mono, SAMPLE_BUFFER_SIZE);
#endif /* ESP32 */

#ifdef TEENSYDUINO
    {
#ifdef CYCLE_MODULE_ENABLED
        calcCycleCountPre();
#endif
        queueTransmitBuffer = queue1.getBuffer(); /* blocking? */
        queueTransmitBuffer2 = queue2.getBuffer();
#ifdef CYCLE_MODULE_ENABLED
        calcCycleCount();
#endif
        if (queueTransmitBuffer)
        {
            {
                for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
                {
                    sampleBuffer[i] = (int16_t)((samples[i]));
                }

                memcpy(queueTransmitBuffer, sampleBuffer, AUDIO_BLOCK_SAMPLES * 2);
                memcpy(queueTransmitBuffer2, sampleBuffer, AUDIO_BLOCK_SAMPLES * 2);
            }
            queue1.playBuffer();
            queue2.playBuffer();
        }
    }
#endif /* TEENSYDUINO */

#ifdef ARDUINO_DAISY_SEED

    float sig_f[SAMPLE_BUFFER_SIZE];

#ifdef CYCLE_MODULE_ENABLED
    calcCycleCountPre();
#endif
    while (!dataReady)
    {
        /* just do nothing */
    }
#ifdef CYCLE_MODULE_ENABLED
    calcCycleCount();
#endif

    for (size_t i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        sig_f[i] = ((float)samples[i]) * (1.0f / ((float)INT16_MAX));
    }

    memcpy(out_temp[0], sig_f, sizeof(out_temp[0]));
    memcpy(out_temp[1], sig_f, sizeof(out_temp[1]));

    dataReady = false;
#endif /* ARDUINO_DAISY_SEED */

#ifdef ARDUINO_SEEED_XIAO_M0
#ifdef CYCLE_MODULE_ENABLED
    calcCycleCountPre();
#endif
    while (!SAMD21_Synth_Process(ProcessAudio))
    {
        /* just do nothing */
    }
#ifdef CYCLE_MODULE_ENABLED
    calcCycleCount();
#endif
    memcpy(u32buf, samples, sizeof(int32_t)*SAMPLE_BUFFER_SIZE);
#endif /* ARDUINO_SEEED_XIAO_M0 */

#if (defined ARDUINO_RASPBERRY_PI_PICO) || (defined ARDUINO_GENERIC_RP2040)
#ifdef RP2040_AUDIO_PWM
    union sample
    {
        uint32_t buff;
        struct
        {
            uint16_t left;
            uint16_t right;
        };
    };

    while (!RP2040_Audio_Pwm_BufferReady())
    {
        /* block! */
    }

    union sample *audioBuff = (union sample *) RP2040_Audio_Pwm_getFreeBuff();

    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        uint16_t val = (samples[i] + 0x8000) >> 5; /* 21 with 32 bit input */
        val += 361;

        audioBuff[i].left = val;
        audioBuff[i].right = val;
    }
#else /* RP2040_AUDIO_PWM */
    /*
     * @see https://arduino-pico.readthedocs.io/en/latest/i2s.html
     * @see https://www.waveshare.com/pico-audio.htm for connections
     */
    int16_t u16int[2 * SAMPLE_BUFFER_SIZE];

    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        u16int[2 * i] = samples[i];
        u16int[(2 * i) + 1] = samples[i];
    }
#if 1
    for (int i = 0; i < SAMPLE_BUFFER_SIZE * 2; i++)
    {
        I2S.write(u16int[i]);
    }
#else
    /* this does not work, I do not know why :-/ */
    static int16_t u16int_buf[2 * SAMPLE_BUFFER_SIZE];
    memcpy(u16int_buf, u16int, sizeof(u16int));
    I2S.write(u16int_buf, sizeof(u16int));
#endif
#endif /* RP2040_AUDIO_PWM */
#endif /* ARDUINO_RASPBERRY_PI_PICO, ARDUINO_GENERIC_RP2040 */

#ifdef ARDUINO_GENERIC_F407VGTX
    /*
     * Todo Implementation for the STM32F407VGT6
     * Can be found on the ST Discovery Board
     */
#endif /* ARDUINO_GENERIC_F407VGTX */

#ifdef ARDUINO_DISCO_F407VG
    STM32_AudioWriteS16(samples);
#endif
}

void Audio_Output(const Q1_14 *left, const Q1_14 *right)
{
    Audio_Output((const int16_t *)left, (const int16_t *)right);
}

void Audio_Output(const int16_t *left, const int16_t *right)
{
#ifdef ESP32
    i2s_write_stereo_samples_i16(left, right, SAMPLE_BUFFER_SIZE);
#endif /* ESP32 */

#ifdef ARDUINO_DAISY_SEED

    float sig_l[SAMPLE_BUFFER_SIZE];
    float sig_r[SAMPLE_BUFFER_SIZE];

#ifdef CYCLE_MODULE_ENABLED
    calcCycleCountPre();
#endif
    while (!dataReady)
    {
        /* just do nothing */
    }
#ifdef CYCLE_MODULE_ENABLED
    calcCycleCount();
#endif

    for (size_t i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        sig_l[i] = ((float)left[i]) * (1.0f / ((float)INT16_MAX));
        sig_r[i] = ((float)right[i]) * (1.0f / ((float)INT16_MAX));
    }

    memcpy(out_temp[0], sig_l, sizeof(out_temp[0]));
    memcpy(out_temp[1], sig_r, sizeof(out_temp[1]));

    dataReady = false;
#endif /* ARDUINO_DAISY_SEED */

#if (defined ARDUINO_RASPBERRY_PI_PICO) || (defined ARDUINO_GENERIC_RP2040)
#ifdef RP2040_AUDIO_PWM
    union sample
    {
        uint32_t buff;
        struct
        {
            uint16_t left;
            uint16_t right;
        };
    };

    while (!RP2040_Audio_Pwm_BufferReady())
    {
        /* block! */
    }

    union sample *audioBuff = (union sample *) RP2040_Audio_Pwm_getFreeBuff();

    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        uint16_t val;
        val = (left[i] + 0x8000) >> 5; /* 21 with 32 bit input */
        val += 361;

        audioBuff[i].left = val;

        val = (right[i] + 0x8000) >> 5; /* 21 with 32 bit input */
        val += 361;
        audioBuff[i].right = val;
    }
#else /* RP2040_AUDIO_PWM */
    /*
     * @see https://arduino-pico.readthedocs.io/en/latest/i2s.html
     * @see https://www.waveshare.com/pico-audio.htm for connections
     */
    int16_t u16int[2 * SAMPLE_BUFFER_SIZE];

    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        u16int[2 * i] = samples[i];
        u16int[(2 * i) + 1] = samples[i];
    }
#if 1
    for (int i = 0; i < SAMPLE_BUFFER_SIZE * 2; i++)
    {
        I2S.write(u16int[i]);
    }
#else
    /* this does not work, I do not know why :-/ */
    static int16_t u16int_buf[2 * SAMPLE_BUFFER_SIZE];
    memcpy(u16int_buf, u16int, sizeof(u16int));
    I2S.write(u16int_buf, sizeof(u16int));
#endif
#endif /* RP2040_AUDIO_PWM */
#endif /* ARDUINO_RASPBERRY_PI_PICO, ARDUINO_GENERIC_RP2040 */
}

#if (defined ESP32) || (defined TEENSYDUINO) || (defined ARDUINO_DAISY_SEED) || (defined ARDUINO_GENERIC_F407VGTX) || (defined ARDUINO_DISCO_F407VG) || (defined ARDUINO_BLACK_F407VE) || (((defined ARDUINO_RASPBERRY_PI_PICO) || (defined ARDUINO_GENERIC_RP2040)) && (defined RP2040_AUDIO_PWM))
void Audio_Input(float *left, float *right)
{
#ifdef ESP32
    i2s_read_stereo_samples_buff(left, right, SAMPLE_BUFFER_SIZE);
#endif /* ESP32 */
}

#ifdef OUTPUT_SAW_TEST
void Audio_Output(float *left, float *right)
#else
void Audio_Output(const float *left, const float *right)
#endif
{
#ifdef OUTPUT_SAW_TEST
    /*
     * base frequency: SAMPLE_FREQ / SAMPLE_BUFFER_SIZE
     * for example: Fs : 44100Hz, Lsb = 48 -> Freq: 918.75 Hz
     */
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        left[i] = ((float)i * 2.0f) / ((float)SAMPLE_BUFFER_SIZE);
        right[i] = ((float)i * 2.0f) / ((float)SAMPLE_BUFFER_SIZE);
        left[i] -= 1.0f;
        right[i] -= 1.0f;
    }
#endif

#ifdef ESP32
    i2s_write_stereo_samples_buff(left, right, SAMPLE_BUFFER_SIZE);
#endif /* ESP32 */

#ifdef TEENSYDUINO
    {
#ifdef CYCLE_MODULE_ENABLED
        calcCycleCountPre();
#endif
        queueTransmitBuffer = queue1.getBuffer(); /* blocking? */
        queueTransmitBuffer2 = queue2.getBuffer();
#ifdef CYCLE_MODULE_ENABLED
        calcCycleCount();
#endif
        if (queueTransmitBuffer)
        {
            {
                for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
                {
                    sampleBuffer[i] = (int16_t)(left[i] * INT16_MAX);
                    sampleBuffer2[i] = (int16_t)(right[i] * INT16_MAX);
                }

                memcpy(queueTransmitBuffer, sampleBuffer, AUDIO_BLOCK_SAMPLES * 2);
                memcpy(queueTransmitBuffer2, sampleBuffer2, AUDIO_BLOCK_SAMPLES * 2);
            }

            queue1.playBuffer();
            queue2.playBuffer();
        }
    }
#endif /* TEENSYDUINO */

#ifdef ARDUINO_DAISY_SEED

#ifdef CYCLE_MODULE_ENABLED
    calcCycleCountPre();
#endif
    while (!dataReady)
    {
        /* just do nothing */
    }
#ifdef CYCLE_MODULE_ENABLED
    calcCycleCount();
#endif

#if 0
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        left[i] = i * (1.0f / ((float)SAMPLE_BUFFER_SIZE));
        right[i] = i * (1.0f / ((float)SAMPLE_BUFFER_SIZE));
    }
#endif
    memcpy(out_temp[0], left, sizeof(out_temp[0]));
    memcpy(out_temp[1], right, sizeof(out_temp[1]));

    dataReady = false;

#endif /* ARDUINO_DAISY_SEED */

#ifdef ARDUINO_GENERIC_F407VGTX

    STM32_AudioWrite(left, right);

#endif /* ARDUINO_GENERIC_F407VGTX */

#ifdef ARDUINO_DISCO_F407VG

    STM32_AudioWrite(left, right);

#endif /* ARDUINO_GENERIC_F407VGTX */

#ifdef RP2040_AUDIO_PWM
    union sample
    {
        uint32_t buff;
        struct
        {
            uint16_t left;
            uint16_t right;
        };
    };

    while (!RP2040_Audio_Pwm_BufferReady())
    {
        /* block! */
    }

    union sample *audioBuff = (union sample *)RP2040_Audio_Pwm_getFreeBuff();

    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        float val;

        val = left[i];
        val += 0.5f;
        val *= (0x8000) >> 5;
        audioBuff[i].left = val;

        val = right[i];
        val += 0.5f;
        val *= (0x8000) >> 5;
        audioBuff[i].right = val;
    }
#endif
}
#endif /* (defined ESP32) || (defined TEENSYDUINO) || (defined ARDUINO_DAISY_SEED) || (defined ARDUINO_GENERIC_F407VGTX) || (defined ARDUINO_DISCO_F407VG) */

#endif // ML_SYNTH_INLINE_DEFINITION

