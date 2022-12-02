#include "mbed.h"
#include "USBCDC.h"
#include "I2C.h"

#include <string>
#include <cmath>
#include <stddef.h>
#include <functional>
#include <abmt/os.h>
#include <abmt/blob.h>
#include <abmt/io/protocols/s2p.h>
#include "io_buffer.h"
#include "stepper.h"
#include "adc.h"

I2C i2c(PB_9, PB_8); // Data-Pin, Clock-Pin

void abmt::os::die(std::string msg){

}

bool next_adc = true;

#define DEVICE_NUM '0'
#define BPIO_VERSION '2'

class USBCDC_BPIO: public USBCDC{
public:
	USBCDC_BPIO():USBCDC(false){
		// when waiting in USBCDC constructor the overloaded
		// virtual functions are not called.
		connect();
		wait_ready();
	}
protected:
	virtual const uint8_t* string_iproduct_desc(){
		static const uint8_t string_iproduct_descriptor[] = {
			0x0c,                                                       /*bLength*/
			STRING_DESCRIPTOR,                                          /*bDescriptorType 0x03*/
			'B', 0, 'P', 0, 'I', 0, 'O', 0, DEVICE_NUM, 0 /*bString iProduct - USB DEVICE*/
		};
		return string_iproduct_descriptor;
	};

	virtual const uint8_t* string_imanufacturer_desc(){
	    static const uint8_t string_imanufacturer_descriptor[] = {
	        0x10,                                            /*bLength*/
	        STRING_DESCRIPTOR,                               /*bDescriptorType 0x03*/
	        'b', 0, 'p', 0, 'i', 0, 'o', 0, ' ', 0, 'v', 0, BPIO_VERSION, 0 /*bString iManufacturer - mbed.org*/
	    };
	    return string_imanufacturer_descriptor;
	}


};



PwmOut pwm_1(PA_6);
PwmOut pwm_2(PA_7);
PwmOut pwm_3(PB_0);
PwmOut pwm_4(PB_1);
PwmOut pwm_5(PB_10);
PwmOut pwm_6(PB_11);

volatile uint32_t* reg_pwm_1 = &(TIM3->CCR1);
volatile uint32_t* reg_pwm_2 = &(TIM3->CCR2);
volatile uint32_t* reg_pwm_3 = &(TIM3->CCR3);
volatile uint32_t* reg_pwm_4 = &(TIM3->CCR4);
volatile uint32_t* reg_pwm_5 = &(TIM2->CCR3);
volatile uint32_t* reg_pwm_6 = &(TIM2->CCR4);

void setup_pwm(){
	TIM2->PSC  = 0;  		// makes 72000000/1024 = 70khz
	TIM2->ARR  = 1023; 		// max-value -1 => 1024 == max == 3.3v
	TIM3->PSC  = TIM2->PSC; // prescaler
	TIM3->ARR  = TIM2->ARR; // max-value
	TIM2->CNT  = 0;
	TIM3->CNT  = 0;
}

class pt2_int{
public:
	int32_t out;
	int64_t mem1;
	int64_t mem2;
	int32_t in = 0;
	int32_t d_1k;
	int64_t T_inv;
	int64_t TT_inv;
	const int32_t ticks_per_sec = 8192;
	float param_t = 0.1;
    float param_d = 1.0;

	void init(){
		d_1k = param_d*1024;
	    float T  = 1.0/2.0/M_PI*param_t;
	    T_inv    = 1.0/T + 0.5;
	    TT_inv   = 1.0/(T*T) + 0.5;
	    mem2 = 0;
	    mem1 = in*16*ticks_per_sec;

	}

	void tick(){
		mem2 += (in*16*TT_inv - mem1*TT_inv/ticks_per_sec - mem2*2*d_1k*T_inv/1024)/ticks_per_sec;
		mem1 += mem2;
		out = mem1/16/ticks_per_sec;
	}
};

struct io_stepper{
	stepper s;
	pt2_int filter;
	DigitalOut dir;
	DigitalOut step;

	io_stepper(PinName d, PinName s): dir(d), step(s){
		filter.init();
	}

	void set_val(int64_t v){
		filter.in = v;
	}

	void tick(){
		filter.tick();
		s.in_pos = filter.out;
		s.tick();
		dir  = s.out_dir;
		step = s.out_step;
	}

	void reset(){
		s.in_pos = filter.in;
		s.pos = s.in_pos;
		filter.init();
	}
};

#pragma pack(push ,1)
struct io_in_data{
    uint16_t pwm[6];
    int64_t  stepper_pos[4];
    bool stepper_reset[4];
    bool led;
};

struct io_out_data{
	uint16_t version = 1;
	uint16_t analog[6] = {};
};

struct bpio_parameters{
	float stepper_pt2_t = 0.1;
	float stepper_pt2_d = 1;
	uint32_t pwm_prescaler = 1;
	uint32_t i2c_baudrate = 400000;
	bool steppers_enabled = true;
};

#pragma pack(pop)

bpio_parameters current_config;

io_stepper s1(PB_12, PB_13);
io_stepper s2(PB_14, PB_15);
io_stepper s3(PA_8,  PA_9);
io_stepper s4(PC_14, PC_15);

Ticker step_ticker;

void step(){
	static int cnt_adc = 0;
	if(current_config.steppers_enabled){
		s1.tick();
		s2.tick();
		s3.tick();
		s4.tick();
	}

	cnt_adc++;
	if(cnt_adc > 8){
		next_adc = true;
		cnt_adc  = 0;
	}
}


void setup_parameters(){
	i2c.frequency(current_config.i2c_baudrate);

	s1.filter.param_t = current_config.stepper_pt2_t;
	s1.filter.param_d = current_config.stepper_pt2_d;
	s1.reset();
	s2.filter.param_t = current_config.stepper_pt2_t;
	s2.filter.param_d = current_config.stepper_pt2_d;
	s2.reset();
	s3.filter.param_t = current_config.stepper_pt2_t;
	s3.filter.param_d = current_config.stepper_pt2_d;
	s3.reset();
	s4.filter.param_t = current_config.stepper_pt2_t;
	s4.filter.param_d = current_config.stepper_pt2_d;
	s4.reset();

	TIM2->PSC = current_config.pwm_prescaler;
	TIM3->PSC = current_config.pwm_prescaler;
}

DigitalOut  led(PC_13);

uint8_t i2c_response[129];



int main() {
	led = true; // default off;
	ThisThread::sleep_for(5ms);
	USBCDC_BPIO cdc;

	setup_pwm();
	setup_parameters();

	adc_init();
	adc_measure();
	step_ticker.attach(step, 122us); // 1/0.000122 == 8196.7 HZ (around 8192 for pt2 division)
	uint32_t bytes_read = 0;
	uint32_t bytes_send = 0;
	buffer in;
	buffer out;
	abmt::io::s2p s2p;
	s2p.set_source(&in);
	s2p.set_sink(&out);
	s2p.on_new_pack = [&](size_t id, void* data, size_t size){
		//led = !led;
		if(id == 1){
			io_in_data* d = (io_in_data*)data;
			if(d->led){
				led = false; // inverted pin!
			}else{
				led = true;
			}
			*reg_pwm_1 = d->pwm[0];
			*reg_pwm_2 = d->pwm[1];
			*reg_pwm_3 = d->pwm[2];
			*reg_pwm_4 = d->pwm[3];
			*reg_pwm_5 = d->pwm[4];
			*reg_pwm_6 = d->pwm[5];
			s1.set_val(d->stepper_pos[0]);
			s2.set_val(d->stepper_pos[1]);
			s3.set_val(d->stepper_pos[2]);
			s4.set_val(d->stepper_pos[3]);
			if(d->stepper_reset[0]){
				s1.reset();
			}
			if(d->stepper_reset[1]){
				s2.reset();
			}
			if(d->stepper_reset[2]){
				s3.reset();
			}
			if(d->stepper_reset[3]){
				s4.reset();
			}

		}else if(id == 2){
			current_config = *((bpio_parameters*)data);
			setup_parameters();
		}else if(id == 3){
			// i2c
			bool ok = false;
			if(size < 2){
				i2c_response[0] = false;
				s2p.send(3,i2c_response, 1);
				return;
			}
			uint8_t bytes_to_send = size -2;
			uint8_t bytes_to_read = *((uint8_t*)data + 1);
			uint8_t adr = *(uint8_t*)(data);
			adr = adr << 1;

			if(bytes_to_read > sizeof(i2c_response)){
				bytes_to_read = sizeof(i2c_response) -1;
			}
			if(bytes_to_send > 0){
				bool read_after_write = false;
				if(bytes_to_read > 0){
					read_after_write = true;
				}
				ok = i2c.write(adr,(char*) data+2,bytes_to_send, read_after_write);
			}
			if(bytes_to_read > 0){
				ok = i2c.read(adr, (char*) i2c_response + 1, bytes_to_read);
			}
			i2c_response[0] = (ok == 0); // true if ok == 0
			s2p.send(3,i2c_response, bytes_to_read + 1);
		}
	};
	uint32_t cnt = 0;
	uint32_t rd_cnt = 0;

	// I don't know why, but when you receive to fast after the
	// connection, the first package contains garbage. You may miss
	// the configuration or the first i2c package (eg. mpu6050 powerup).
	// 3ms -> 7x good 3x bad
	// 4mx -> 10x good 0x bad
	ThisThread::sleep_for(8ms);

    while(1) {
    	if(cdc.ready() == false){
    		continue;
    	}
        cdc.receive_nb(in.buffer + in.bytes_used, sizeof(in.buffer) - in.bytes_used, &bytes_read);
        if(bytes_read > 0){
        	in.bytes_used += bytes_read;
        	in.send();
        	rd_cnt++;
        	if(rd_cnt > 1){
        		asm("nop;");
        	}
        }
        if(out.bytes_used > 0){
        	cdc.send_nb(out.buffer,out.bytes_used,&bytes_send);
        	out.pop_front(bytes_send);
        }

        if(next_adc){
        	next_adc = false;
        	cnt++;
        	io_out_data data;
        	for(int i = 0; i < 6; ++i){
        		data.analog[i] = adc_values[i];
        	}
        	s2p.send(1,&data, sizeof(data));
        	adc_measure();
        }
    }
}
