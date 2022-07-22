/*
 * stepper.h
 *
 *  Created on: 23.11.2021
 *      Author: hva
 */

#ifndef STEPPER_H_
#define STEPPER_H_
#include <cstdint>

struct stepper{
    int64_t in_pos = 0;
    bool in_reset  = false;
    bool out_dir   = false;
    bool out_step  = true; // start with false

    int64_t pos;
    void init();
    void tick();
};




#endif /* STEPPER_H_ */
