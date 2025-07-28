#include <Arduino.h>
#include <Serial.h>
#include <I2C.h>

#define MUX_ADDR 0x70
#define ACC_ADDR 0x53


#define ACC_PWR 0x2D
#define ACC_ON 0x8
#define ACC_OFF 0x00

#define ACC_DAT_FORMAT 0x31
#define ACC_2G 0x8
#define ACC_4G 0x9

#define ACC_BW_RATE 0x2C
#define ACC_RATE_100HZ 0x0A

#define ACC_TEST 0x1D
#define ACC_DAT_START 0x32

#define DEBUG_MODE 0 // Set to 1 for more verbose output (Breaks labview)

uint8_t act_channels[8] = {0,0,0,0,0,0,0,0};
bool min = 0;

uint8_t i2c_err(uint8_t err){
	if (err != 0) {
		#if DEBUG_MODE
        Serial_print_s("I2C failed with code: ");
        Serial_print_i(err);
        Serial_println_s("");
		#endif
    }
	return err;
}

uint8_t select_i2c_channel(uint8_t channel){
	uint8_t err = I2C_write(MUX_ADDR, 1<<channel);
	if(i2c_err(err)!=0){
		#if DEBUG_MODE
		Serial_print_s("Mux error on channel ");	
		Serial_print_i(channel);
		Serial_print_c('\n');
		#endif
	}
	return 0;
}

uint8_t setup_acc(uint8_t channel){
	int err;
	if(select_i2c_channel(channel) !=0){
		return 0;
	}
	delay(5);

	err = I2C_write_reg(ACC_ADDR, ACC_PWR, ACC_ON); // Turn device on
	if(i2c_err(err)!=0){
		return 0;
	}
	
	err = I2C_write_reg(ACC_ADDR, ACC_DAT_FORMAT, ACC_2G); // Turn device on
	if(i2c_err(err)!=0){
		return 0;
	}
	err = I2C_write_reg(ACC_ADDR, ACC_BW_RATE, ACC_RATE_100HZ);
	if (i2c_err(err) != 0) {
		return 0;
	}

	return 1;
}

void setup(){
	int err;

	Serial_begin(9600);
	I2C_begin();

	for(int i=0; i<8; i++){
		if(setup_acc(i)){
			act_channels[i] = 1;
			#if DEBUG_MODE
			Serial_print_s("Device on at channel: ");
			Serial_print_i(i);
			Serial_print_c('\n');
			#endif
			min++;
		}
	}
	if(!min){
		#if DEBUG_MODE
		Serial_println_s("No sensors connected!");
		#endif
		return;
	}
}

void loop() {
	int16_t data[3];
	uint8_t raw_data[6];
	int err;
	float scale = 1.0 / 256.0;

	for (int ch = 0; ch < 8; ch++) {
		if (!act_channels[ch]) continue;

		if (select_i2c_channel(ch) != 0) {
			#if DEBUG_MODE
			Serial_print_s("Failed to select channel ");
			Serial_print_i(ch);
			Serial_println_s("");
			#endif
			continue;
		}
		delay(5);

		err = I2C_readbuf_reg(ACC_ADDR, ACC_DAT_START, 6, raw_data);
		if (i2c_err(err) != 0) {
			#if DEBUG_MODE
			Serial_print_s("Error reading data from channel ");
			Serial_print_i(ch);
			Serial_println_s("");
			#endif
			continue;
		}

		for (int i = 0; i < 3; i++) {
			data[i] = (int16_t)(raw_data[i * 2] | (raw_data[i * 2 + 1] << 8));
		}


		Serial_print_i(ch);
		Serial_print_c(',');
		Serial_print_f(data[0] * scale);
		 Serial_print_c(',');
		Serial_print_f(data[1] * scale);
		 Serial_print_c(',');
		Serial_print_f(data[2] * scale);
		 Serial_println_s("");
	}

	delay(11); // ~100 Hz sample rate
}