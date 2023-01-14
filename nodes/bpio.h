#ifndef BPIO_H
#define BPIO_H BPIO_H

#include <abmt/io/eio.h>
#include <abmt/io/protocols/s2p.h>
#include <abmt/io/i2c.h>

#include <iostream>
#include <functional>

namespace bpio{

struct bpio_i2c;

//@node: auto
struct bpio_usb{
    bool in_led;
    float in_pwm_1 = 0;
    float in_pwm_2 = 0;
    float in_pwm_3 = 0;
    float in_pwm_4 = 0;
    float in_pwm_5 = 0;
    float in_pwm_6 = 0;
    
    int64_t in_stepper_pos_1   = 0;
    bool    in_stepper_reset_1 = false;
    int64_t in_stepper_pos_2   = 0;
    bool    in_stepper_reset_2 = false;
    int64_t in_stepper_pos_3   = 0;
    bool    in_stepper_reset_3 = false;
    int64_t in_stepper_pos_4   = 0;
    bool    in_stepper_reset_4 = false;
    
    
    bool out_connected = false;
    std::shared_ptr<bpio_i2c> out_i2c_bus;
    float out_analog_1 = 0;
    float out_analog_2 = 0;
    float out_analog_3 = 0;
    float out_analog_4 = 0;
    float out_analog_5 = 0;
    float out_analog_6 = 0;
    
    
    float param_stepper_pt2_t    = 0.05;
    float param_stepper_pt2_d    = 1;
    uint32_t param_pwm_prescaler = 0;
    uint32_t param_i2c_baudrate = 400000;
    bool param_enable_steppers = true;
    
    std::string param_device = "/dev/ttyACM0";
    
    abmt::io::event_list lst;
    abmt::io::serial_ptr con;
    abmt::io::s2p s2p;
    
    void init();
    void tick();
    void serial_to_raw_mode();
    
};

struct bpio_i2c: public abmt::io::i2c_bus{
    bpio_usb& bpio;
    bool new_data = false;
    void*  read_dst;
    size_t read_len;
    bool i2c_result = false;
    
    bpio_i2c(bpio_usb&);
    virtual bool write_read(uint16_t addr, void* write, uint16_t w_len, void* read, uint16_t r_len);
    void i2c_response(void* data, size_t size);
};


} // namespace bpio

#endif // BPIO_H
