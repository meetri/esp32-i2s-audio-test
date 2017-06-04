/*
 * app_main.c
 *
 *  Created on: 30.03.2017
 *      Author: michaelboeckling
 */

#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <stdbool.h>
#include <sys/time.h>
#include <float.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2s.h"

#include "SSD1306.h"

#include "Minmax.h"

#define SDA_PIN 18
#define SCL_PIN 19
SSD1306 display(0x3c,SDA_PIN,SCL_PIN);

#define NUM_CHANNELS 2
#define SAMPLE_RATE 44100
#define BIT_SIZE I2S_BITS_PER_SAMPLE_32BIT
#define BIT_SIZE_CONFIG I2S_BITS_PER_SAMPLE_32BIT

#define TAG "main"

static void init_i2s()
{
   const int sample_rate = 44100;

    /* RX: I2S_NUM_1 */
    i2s_config_t i2s_config;
    i2s_config.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX);
    i2s_config.sample_rate = SAMPLE_RATE;
    i2s_config.bits_per_sample = i2s_bits_per_sample_t(BIT_SIZE_CONFIG);
    i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    //i2s_config.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S_MSB);
    i2s_config.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S);
    i2s_config.dma_buf_count = 92;
    i2s_config.dma_buf_len = 92 * 2;
    i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;

    i2s_pin_config_t pin_config;
    pin_config.bck_io_num = GPIO_NUM_26; //GPIO_NUM_17;
    pin_config.ws_io_num = GPIO_NUM_25; //GPIO_NUM_18;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num = GPIO_NUM_23;

   esp_err_t err = i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
   if ( err == ESP_FAIL ){
       printf("i2s install driver failed");

   }else {
       
       printf("successfully installed driver\n");

   }

   i2s_set_pin(I2S_NUM_1, &pin_config);

}

void init_display(){
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    //display.drawString(0,0,"Hello world");
    //    display.display();
}

unsigned int reverseWord(register unsigned int x) {
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));

}

unsigned char reverseByte(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void task_testbits(void *pvParams)
{
   uint16_t buf_len = 2048;
   char *buf = (char*) calloc(buf_len, sizeof(char));

   Minmax mm0,mm1,mm2,mm3;
   Minmax val0;
   std::ostringstream strs;

   init_i2s();
   init_display();
   //init_wifi();

   unsigned long cnt = 0;
   while(1)
   {
      char *rbuf = buf;
      // read whole block of samples
      int bytes_read = 0;
      while(bytes_read == 0) {
          bytes_read = i2s_read_bytes(I2S_NUM_1, buf, buf_len, 0);
      }

      uint32_t samples_read = bytes_read / NUM_CHANNELS / (BIT_SIZE / 8);

      //  convert 2x 32 bit stereo -> 1 x 16 bit mono PCM
      for(int i = 0; i < samples_read; i++) {
		uint8_t b0 = rbuf[0];
		uint8_t b1 = rbuf[1];
		uint8_t b2 = rbuf[2];
		uint8_t b3 = rbuf[3];
        rbuf += NUM_CHANNELS * (BIT_SIZE / 8);

        uint16_t val = (b3  << 8 ) + b2;
        val0.set(val); 
        mm0.set(b0);
        mm1.set(b1);
        mm2.set(b2);
        mm3.set(b3);

        cnt ++;
        if( cnt > 5000 ) {

            //printf("%d samples in %" PRIu64 " usecs : %d:%d:%d:%d:P2P=%d:Volts=%f\n", cnt, delta,buf[0],buf[1],buf[2],buf[3],cbuf.getPeakToPeak(),cbuf.getVolts());

            strs.clear();
            strs.str("");

            strs << "v: " << val0.getPeakToPeak() << " : " << val0.getMin() << " : " << val0.getMax() << "\n";
            //strs << "samples: " << lcnt << "\n";
            strs << "0: " << mm0.getPeakToPeak() << " : " << mm0.getMin() << " : " << mm0.getMax() << "\n";
            strs << "1: " << mm1.getPeakToPeak() << " : " << mm1.getMin() << " : " << mm1.getMax() << "\n";
            strs << "2: " << mm2.getPeakToPeak() << " : " << mm2.getMin() << " : " << mm2.getMax() << "\n";
            strs << "3: " << mm3.getPeakToPeak() << " : " << mm3.getMin() << " : " << mm3.getMax() << "\n";

            display.clear();
            display.drawString(0,0,strs.str().c_str() );
            //ESP_LOGI(TAG,"%s",strs.str().c_str() );
            display.display();

            cnt = 0; 
            val0.reset(); mm0.reset(); mm1.reset(); mm2.reset(); mm3.reset();
        }

      }


   }

}

/**
 * entry point
 */
extern "C" void app_main()
{
    printf("starting app_main()\n");
    xTaskCreatePinnedToCore(&task_testbits, "task_testbits", 16384, NULL, 20, NULL, 0);
}
