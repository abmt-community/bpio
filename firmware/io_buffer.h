/*
 * io_buffer.h
 *
 *  Created on: 23.11.2021
 *      Author: hva
 */

#ifndef IO_BUFFER_H_
#define IO_BUFFER_H_


#include <stddef.h>
#include <functional>
#include <abmt/os.h>
#include <abmt/blob.h>

/// When the send function is called, data is pushed into the buffer and on_new_data
/// is called. The default on_new_data returns zero, so everything stays in the buffer.
/// Use as sink: the source calls send and the default on_new_data function keeps everything
/// the buffer. Use as source: The source overrides the on_new_data function. When you call
/// the buffers send(ptr,size) function, new data is put into the buffer, and the complete
/// buffer given to on_new_date of the source. The used data is then poped out of the buffer.
struct buffer{
	uint8_t buffer[128] = {};
	size_t bytes_used = 0;

	std::function<size_t(abmt::blob&)> on_new_data = [this](abmt::blob& b)->size_t{
		return 0;
	};


	/// copies data to buffer. Does not call on_new_data
	void push_back(const void* data, size_t size){
		size_t bytes_to_copy = size;
		if(bytes_to_copy + bytes_used > sizeof(buffer)){
			bytes_to_copy = sizeof(buffer) - bytes_used;
			// todo error_handling
		}
		memcpy(buffer+bytes_used,data,bytes_to_copy);
		bytes_used += bytes_to_copy;
	}

	void pop_front(size_t nbytes){
		if(nbytes != 0){
			if(bytes_used - nbytes == 0){
				bytes_used = 0;
				return;
			}
			memmove(buffer, buffer+nbytes, bytes_used - nbytes);
			bytes_used -= nbytes;
		}
	}

	/// Copies data to buffer and calls send();
	void send(const void* data, size_t size){
		push_back(data,size);
		send();
	}

	// calles on_new_data and pops received bytes
	void send(){
		abmt::blob b(buffer,bytes_used);
		size_t bytes_to_pop = on_new_data(b);
		pop_front(bytes_to_pop);
	}

};


#endif /* IO_BUFFER_H_ */
