/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef RUNTIME_INTERFACE_S2P_BASE_H_
#define RUNTIME_INTERFACE_S2P_BASE_H_

#include <functional>
#include <string.h>
#include <abmt/io/sink.h>
#include <abmt/io/source.h>

namespace abmt{
namespace io{

class s2p{
private:
	abmt::io::sink sink;
	abmt::io::source source;

public:
	std::function<void(size_t id, void* data, size_t size)> on_new_pack;
	s2p(abmt::io::sink si = {}, abmt::io::source so = {}){
		set_sink(si);
		set_source(so);
		on_new_pack = [](size_t id, void* data, size_t size){

		};
	}

	void set_sink(abmt::io::sink si){
		sink = si;
	}

	void set_source(abmt::io::source so){
		source = so;
		source.on_new_data = [this](abmt::blob& b)->size_t{
			return handle_data((uint8_t*)b.data,b.size);
		};
		read_state = search_start;
	}

	enum {search_start,check_for_header,wait_for_pack} read_state = search_start;

	size_t current_id = 0;
	size_t current_pack_size = 0;


	size_t handle_data(uint8_t* buffer, uint32_t pos){
		if(buffer == 0){
			return pos;
		}

		// std::cout << "rcv: " << pos << std::endl;
		const size_t header_size = 9;
		const size_t crc_size = 2;

		size_t bytes_availible = pos;
		size_t bytes_to_pop = 0;

		auto abort = [&]{

			++bytes_to_pop; // nur ein byte; irgenwo im header könnte der neue start sein
			read_state = search_start;
			// std::cout << "abort... " << bytes_to_pop << std::endl;
		};


		while( bytes_to_pop < pos ){
			if(read_state == search_start){
				while(bytes_to_pop < pos){
					if(buffer[bytes_to_pop] == 'U'){
						read_state = check_for_header;
						// std::cout << "state change to: check_for_header... " << bytes_to_pop << std::endl;
						break;
					}
					++bytes_to_pop;
				}

			}

			bytes_availible = pos - bytes_to_pop;
			if(read_state == check_for_header){
				if(bytes_availible >= header_size){
					uint8_t header_check = buffer[bytes_to_pop + 0] ^ buffer[bytes_to_pop + 1] ^ buffer[bytes_to_pop + 2] ^ buffer[ bytes_to_pop +3];
					header_check        ^= buffer[bytes_to_pop + 4] ^ buffer[bytes_to_pop + 5] ^ buffer[bytes_to_pop + 6] ^ buffer[ bytes_to_pop +7];
					bool type_ok = buffer[bytes_to_pop + 1] == 'A';
					bool hdr_chk_ok = buffer[bytes_to_pop + 8] == header_check;
					if( type_ok && hdr_chk_ok ){
						current_pack_size = *(uint32_t*)(&buffer[bytes_to_pop + 4]);
						// std::cout << "rcv header for size: " << current_pack_size << std::endl;
						current_id = *(uint8_t*)(&buffer[bytes_to_pop + 2]);
						read_state = wait_for_pack;
						// std::cout << "state change to: wait_for_pack" << std::endl;
					}else{
						// std::cout << "error: header_check_failed; type_ok:" << type_ok << " hdr_ok:" << hdr_chk_ok << " pos:" << bytes_to_pop << std::endl;
						// std::cout << "      " << buffer[bytes_to_pop + 0] << buffer[bytes_to_pop + 1] << " " << (int) header_check << " " << (int) buffer[bytes_to_pop + 8] << std::endl;
						abort();
					}
				}else{
					// not enaugh bytes
					// std::cout << "wait_for_bytes" << std::endl;
					break;
				}
			}

			if(read_state == wait_for_pack){
				if(bytes_availible >= header_size + crc_size + current_pack_size){
					uint16_t crc = make_checksum(&buffer[bytes_to_pop + header_size], current_pack_size);
					uint16_t ctc_tmp = crc;
					uint16_t ctc_pack = *(uint16_t*)(&buffer[bytes_to_pop + header_size + current_pack_size ]);
					if(ctc_tmp == ctc_pack){
						// std::cout << "rcv_pack :)" << std::endl;
						on_new_pack(current_id,&buffer[bytes_to_pop + header_size],current_pack_size);
						bytes_to_pop += header_size + current_pack_size + crc_size;
						read_state = search_start;
					}else{
						// std::cout << "crc_error: " << buffer[0]<<  std::endl;
						abort();
					}
				}else{
					// not enaugh bytes
					break;
				}
			}

		} // while

		return bytes_to_pop;
	}

	/**
	 * ideen für types:
	 * A: 32-Bit size
	 */
	void send(uint8_t id, const void* data, uint32_t size){
		static uint8_t seed = 0;

		char header[9]; // start + type + id + seed + size + header_check
		header[0] = 'U';
		header[1] = 'A';
		header[2] = id;
		header[3] = seed;
		uint8_t header_check = header[0] ^ header[1] ^ header[2] ^ header[3];
		char* size_as_char = (char*)&size;
		for(unsigned int i = 0; i < sizeof(size); ++i){
			header[4 + i] = size_as_char[i];
			header_check ^= size_as_char[i];
		}
		header[8] = header_check;
		sink.send(header,9);

		// Data
		sink.send(data,size);
		uint16_t crc = make_checksum(data, size);
		sink.send(&crc,2);

	}

	uint16_t make_checksum(const void* data_ptr, size_t len) {
		// Bis revision 508 waren hier noch weitere Checksummen zum Testen auskommentiert
		unsigned int sum1 = 0;
		unsigned int sum2 = 0;
		const uint8_t* data = (const uint8_t*)data_ptr;

		for( unsigned int index = 0; index < len;  ) {
			unsigned int blocklen = 5802;
			if(blocklen > len-index){
				blocklen = len-index;
			}
			for( unsigned j = 0; j < blocklen; ++j ) {
				sum1 = (sum1 + data[index+j]);
				sum2 = (sum2 + sum1);
			}
			index += blocklen;
			sum1 = sum1 % 255;
			sum2 = sum2 % 255;

		}
		return (sum2 << 8) | sum1;
	}


};

}; // namespace io
}; // namespace abmt

#endif /* RUNTIME_INTERFACE_S2P_BASE_H_ */
