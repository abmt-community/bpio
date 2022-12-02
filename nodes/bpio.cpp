#include "bpio.h"
#include <iostream>
#include <termios.h>
#include <sys/ioctl.h>
#include <abmt/time.h>
#include <unistd.h>

using namespace std;
using namespace bpio;


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

void bpio_usb::init(){
    out_i2c_bus.reset(new bpio_i2c(*this));
    con.reset(new abmt::io::serial(lst, param_device));
	s2p.set_sink(con);
	s2p.set_source(con);
	s2p.on_new_pack = [this](size_t id, void* data, size_t size){
	    out_connected = true;
		if(id == 1){
		    io_out_data* d = (io_out_data*) data;
		    out_analog_1 = d->analog[0] / 4096.0;
		    out_analog_2 = d->analog[1] / 4096.0;
		    out_analog_3 = d->analog[2] / 4096.0;
		    out_analog_4 = d->analog[3] / 4096.0;
		    out_analog_5 = d->analog[4] / 4096.0;
		    out_analog_6 = d->analog[5] / 4096.0;
		}else if(id == 3){
		    if(out_i2c_bus){
		        out_i2c_bus->i2c_response(data,size);
		    }
		}
		
	};
	
	bpio_parameters p;
	p.stepper_pt2_t = param_stepper_pt2_t;
	p.stepper_pt2_d = param_stepper_pt2_d;
	p.pwm_prescaler = param_pwm_prescaler;
	p.i2c_baudrate = param_i2c_baudrate;
	p.steppers_enabled = param_enable_steppers;
	
	s2p.send(2,&p,sizeof(p));
}

void bpio_usb::tick(){
    lst.wait(0); // calls on_new_data and other events
    io_in_data d;
    
    auto to_pwm = [](float i)->uint16_t{
        if(i < 0){
            i = 0;
        }else if(i > 1){
            i = 1;
        }
        return i*1024;
    };
    
    d.pwm[0] = to_pwm(in_pwm_1);
    d.pwm[1] = to_pwm(in_pwm_2);
    d.pwm[2] = to_pwm(in_pwm_3);
    d.pwm[3] = to_pwm(in_pwm_4);
    d.pwm[4] = to_pwm(in_pwm_5);
    d.pwm[5] = to_pwm(in_pwm_6);
    
    d.stepper_pos[0] = in_stepper_pos_1;
    d.stepper_pos[1] = in_stepper_pos_2;
    d.stepper_pos[2] = in_stepper_pos_3;
    d.stepper_pos[3] = in_stepper_pos_4;
    
    d.stepper_reset[0] = in_stepper_reset_1;
    d.stepper_reset[1] = in_stepper_reset_2;
    d.stepper_reset[2] = in_stepper_reset_3;
    d.stepper_reset[3] = in_stepper_reset_4;
    
    d.led = in_led;
    
    s2p.send(1,&d,sizeof(d));
}

bpio_i2c::bpio_i2c(bpio_usb& b):bpio(b){
    
}

bool bpio_i2c::write_read(uint16_t addr, void* write, uint16_t w_len, void* read, uint16_t r_len){
    new_data = false;
    read_dst = read;
    read_len = r_len;
    i2c_result = false;
    
    // write
    auto snd_pack = new uint8_t[2+w_len];
    snd_pack[0] = addr;
    snd_pack[1] = r_len;
    memcpy(snd_pack + 2, write, w_len);
    bpio.s2p.send(3, snd_pack, 2+w_len);
    delete[] snd_pack;
    
    // read
    for(int i = 0; i < 10; ++i){
        bpio.lst.wait(10); // calls i2c_response
        if(new_data){
            break;
        }
    }
    read_dst = 0;
    read_len = 0;
    return i2c_result;
}

void bpio_i2c::i2c_response(void* data, size_t size){
    if(read_len != size -1){
        i2c_result = false;
        return;
    }
    uint8_t* d = (uint8_t*) data;
    i2c_result = d[0];
    memcpy(read_dst, d+1,size -1);
    new_data = true;
}
