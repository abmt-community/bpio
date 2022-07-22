/*
 * stepper.cpp
 *
 *  Created on: 23.11.2021
 *      Author: hva
 */

#include "stepper.h"

void stepper::init(){
    pos = in_pos;
}

void stepper::tick(){
    if(in_reset){
        pos = in_pos;
    }

    if(out_step == false){
        // complete the step
        out_step = true;
        return;
    }

    if(in_pos == pos){
        return;
    }

    // you only come here when out_step is high
    // from last step and there are steps to do.
    // -> set out_step to low and set dir
    out_step = false;
    if(in_pos > pos){
        ++pos;
        out_dir = false; // dir == low when move in positive direction
    }
    if(in_pos < pos){
        --pos;
        out_dir = true;
    }
}

