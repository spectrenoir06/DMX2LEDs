/* 
 * This file is part of the ESP32-DMX distribution (https://github.com/luksal/ESP32-DMX).
 * Copyright (c) 2021 Lukas Salomon.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <dmx_lib.h>

#include "driver/gpio.h"
#include "driver/uart.h"

#define UART_TXD_INV  (BIT(22))
#define UART_TXD_INV_M  (BIT(22))
#define UART_INVERSE_TXD   (UART_TXD_INV_M)    /*!< UART TXD output inverse*/

#define DMX_UART_NUM            UART_NUM_2  // dmx uart

#define HEALTHY_TIME            500         // timeout in ms 

#define BUF_SIZE                1024        //  buffer size for rx events

#define DMX_CORE                1           // select the core the rx/tx thread should run on

#define DMX_IGNORE_THREADSAFETY 0           // set to 1 to disable all threadsafe mechanisms

QueueHandle_t DMXLibrary::dmx_rx_queue;

SemaphoreHandle_t DMXLibrary::sync_dmx;

DMXState DMXLibrary::dmx_state = DMX_IDLE;

uint16_t DMXLibrary::current_rx_addr = 0;

long DMXLibrary::last_dmx_packet = 0;

uint8_t DMXLibrary::dmx_data[513];

// Frames are received into rx_buf and only copied to dmx_data once complete,
// so a glitch on the line (seen as a break in the middle of a frame) can't
// shift the rest of a frame onto the first channels.
static uint8_t rx_buf[513];
static uint16_t rx_len = 0;
static unsigned long last_full_frame = 0;  // ticks (ms), last frame with all 512 channels
static uint16_t last_short_len = 0;        // length of the previous short frame
static uint32_t dropped_frames = 0;
static portMUX_TYPE dmx_mux = portMUX_INITIALIZER_UNLOCKED;


DMXLibrary::DMXLibrary()
{

}

void DMXLibrary::Initialize(DMXDirection direction)
{
    // configure UART for DMX
    uart_config_t uart_config =
    {
        .baud_rate = 250000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_2,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(DMX_UART_NUM, &uart_config);

    // Set pins for UART
    uart_set_pin(DMX_UART_NUM, DMX_SERIAL_OUTPUT_PIN, DMX_SERIAL_INPUT_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // install queue
    uart_driver_install(DMX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &dmx_rx_queue, 0);

    // create mutex for syncronisation
    sync_dmx = xSemaphoreCreateMutex();

    // set gpio for direction
    gpio_pad_select_gpio(DMX_SERIAL_IO_PIN);
    gpio_set_direction((gpio_num_t)DMX_SERIAL_IO_PIN, GPIO_MODE_OUTPUT);

    // depending on parameter set gpio for direction change and start rx or tx thread
    if(direction == output)
    {
        gpio_set_level((gpio_num_t)DMX_SERIAL_IO_PIN, 1);
        dmx_state = DMX_OUTPUT;
        
        // create send task
        xTaskCreatePinnedToCore(DMXLibrary::uart_send_task, "uart_send_task", 1024, NULL, 1, NULL, DMX_CORE);
    }
    else
    {    
        gpio_set_level((gpio_num_t)DMX_SERIAL_IO_PIN, 0);
        dmx_state = DMX_IDLE;

        // create receive task
        xTaskCreatePinnedToCore(DMXLibrary::uart_event_task, "uart_event_task", 2048, NULL, 1, NULL, DMX_CORE);
    }
}

uint8_t DMXLibrary::Read(uint16_t channel)
{
    // restrict acces to dmx array to valid values
    if(channel < 1 || channel > 512)
    {
        return 0;
    }

    // take data threadsafe from array and return
#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreTake(sync_dmx, portMAX_DELAY);
#endif
    portENTER_CRITICAL(&dmx_mux);
    uint8_t tmp_dmx = dmx_data[channel];
    portEXIT_CRITICAL(&dmx_mux);
#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreGive(sync_dmx);
#endif
    return tmp_dmx;
}

void DMXLibrary::ReadAll(uint8_t * data, uint16_t start, size_t size)
{
    // restrict acces to dmx array to valid values
    if(start < 1 || start > 512 || start + size > 513)
    {
        return;
    }
#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreTake(sync_dmx, portMAX_DELAY);
#endif
    memcpy(data, (uint8_t *)dmx_data + start, size);
#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreGive(sync_dmx);
#endif
}

void DMXLibrary::Write(uint16_t channel, uint8_t value)
{
    // restrict acces to dmx array to valid values
    if(channel < 1 || channel > 512)
    {
        return;
    }

#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreTake(sync_dmx, portMAX_DELAY);
#endif
    dmx_data[channel] = value;
#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreGive(sync_dmx);
#endif
}

void DMXLibrary::WriteAll(uint8_t * data, uint16_t start, size_t size)
{
    // restrict acces to dmx array to valid values
    if(start < 1 || start > 512 || start + size > 513)
    {
        return;
    }
#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreTake(sync_dmx, portMAX_DELAY);
#endif
    memcpy((uint8_t *)dmx_data + start, data, size);
#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreGive(sync_dmx);
#endif
}

uint8_t DMXLibrary::IsHealthy()
{
    // get timestamp of last received packet
#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreTake(sync_dmx, portMAX_DELAY);
#endif
    portENTER_CRITICAL(&dmx_mux);
    long dmx_timeout = last_dmx_packet;
    portEXIT_CRITICAL(&dmx_mux);
#ifndef DMX_IGNORE_THREADSAFETY
    xSemaphoreGive(sync_dmx);
#endif
    // check if elapsed time < defined timeout
    if(xTaskGetTickCount() - dmx_timeout < HEALTHY_TIME)
    {
        return 1;
    }
    return 0;
}

void DMXLibrary::uart_send_task(void*pvParameters)
{
    uint8_t start_code = 0x00;
    for(;;)
    {
        // wait till uart is ready
        uart_wait_tx_done(DMX_UART_NUM, 1000);
        // set line to inverse, creates break signal
        uart_set_line_inverse(DMX_UART_NUM, UART_INVERSE_TXD);
        // wait break time
        ets_delay_us(184);
        // disable break signal
        uart_set_line_inverse(DMX_UART_NUM,  0);
        // wait mark after break
        ets_delay_us(24);
        // write start code
        uart_write_bytes(DMX_UART_NUM, (const char*) &start_code, 1);
#ifndef DMX_IGNORE_THREADSAFETY
        xSemaphoreTake(sync_dmx, portMAX_DELAY);
#endif
        // transmit the dmx data
        uart_write_bytes(DMX_UART_NUM, (const char*) dmx_data+1, 512);
#ifndef DMX_IGNORE_THREADSAFETY
        xSemaphoreGive(sync_dmx);
#endif
    }
}

uint32_t DMXLibrary::DroppedFrames()
{
    return dropped_frames;
}

// Called when a frame ends (next break, or 512 channels received)
static void end_frame(uint8_t* dmx_data, long* last_dmx_packet)
{
    bool valid;
    if(rx_len == 513)
    {
        valid = true;
        last_full_frame = xTaskGetTickCount();
    }
    else
    {
        // Short frames are legal (sources sending fewer channels), but a
        // false break also gives one. Accept them only from a source that
        // doesn't send full frames, and twice the same length in a row.
        valid = rx_len > 1 && rx_len == last_short_len && xTaskGetTickCount() - last_full_frame > 1000;
        last_short_len = rx_len;
    }

    if(valid)
    {
        portENTER_CRITICAL(&dmx_mux);
        memcpy(dmx_data, rx_buf, rx_len);
        *last_dmx_packet = xTaskGetTickCount();
        portEXIT_CRITICAL(&dmx_mux);
    }
    else if(rx_len > 0)
    {
        dropped_frames++;
    }
    rx_len = 0;
}

void DMXLibrary::uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(BUF_SIZE);
    for(;;)
    {
        // wait for data in the dmx_queue
        if(xQueueReceive(dmx_rx_queue, (void * )&event, (portTickType)portMAX_DELAY))
        {
            switch(event.type)
            {
                case UART_DATA:
                {
                    int len = uart_read_bytes(DMX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    int i = 0;
                    // first byte after a break: start code, 0 for DMX (else RDM or custom protocol)
                    if(dmx_state == DMX_BREAK && len > 0)
                    {
                        dmx_state = (dtmp[0] == 0) ? DMX_DATA : DMX_IDLE;
                        rx_len = 0;
                    }
                    if(dmx_state == DMX_DATA)
                    {
                        for(; i < len && rx_len < 513; i++)
                            rx_buf[rx_len++] = dtmp[i];
                        if(rx_len == 513)
                        {
                            end_frame(dmx_data, &last_dmx_packet);
                            dmx_state = DMX_IDLE; // wait for the next break
                        }
                    }
                    break;
                }
                case UART_BREAK:
                    // break detected: the previous frame is over
                    if(dmx_state == DMX_DATA)
                        end_frame(dmx_data, &last_dmx_packet);
                    uart_flush_input(DMX_UART_NUM);
                    xQueueReset(dmx_rx_queue);
                    dmx_state = DMX_BREAK;
                    break;
                case UART_FRAME_ERR:
                case UART_PARITY_ERR:
                case UART_BUFFER_FULL:
                case UART_FIFO_OVF:
                default:
                    // error received: drop the frame, wait for the next break
                    if(dmx_state == DMX_DATA && rx_len > 0)
                        dropped_frames++;
                    rx_len = 0;
                    uart_flush_input(DMX_UART_NUM);
                    xQueueReset(dmx_rx_queue);
                    dmx_state = DMX_IDLE;
                    break;
            }
        }
    }
}
