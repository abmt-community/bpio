/*
 * i2c.h
 *
 *  Created on: 29.09.2021
 *      Author: hva
 */

#ifndef SHARED_ABMT_IO_I2C_H_
#define SHARED_ABMT_IO_I2C_H_

#include <cstdint>
#include <memory>

namespace abmt{
namespace io{

struct i2c_bus{

    virtual bool write_read(uint16_t addr, void* write, uint16_t w_len, void* read, uint16_t r_len) = 0;

    template<typename T1, typename T2>
    bool write_read(uint16_t adr, T1& w, T2& r){
        return write_read(adr,&w, sizeof(w), &r, sizeof(r));
    }

    template<typename T1>
    bool write_read(uint16_t adr, std::initializer_list<uint8_t> w, T1& r){
        return write_read(adr,(void*)w.begin(), w.size(), &r, sizeof(r));
    }

    template<typename T>
    bool write(uint16_t adr, T& w){
        return write_read(adr,&w, sizeof(w), 0, 0);
    }

    bool write(uint16_t adr, std::initializer_list<uint8_t> d){
        return write_read(adr,(void*)d.begin(), d.size(), 0, 0);
    }

    template<typename T>
    bool read(uint16_t adr, T& w){
        return write_read(adr,0, 0, &w, sizeof(w));
    }

    virtual ~i2c_bus() {};
};
using i2c_bus_ptr = std::shared_ptr<i2c_bus>;

} // namespace io
} // namespace abmt


#endif /* SHARED_ABMT_IO_I2C_H_ */
