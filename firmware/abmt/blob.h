/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef SHARED_ABMT_BLOB_MBU_H_
#define SHARED_ABMT_BLOB_MBU_H_

#include <type_traits>
#include <cstdlib>
#include <cstring>
#include <string>
#include <memory>
#include <abmt/os.h>
#include <abmt/const_array.h>

namespace abmt{

/**
 * Helper for conversion form a block of memory(datapointer + size) to another type.
 */

class blob;
class blob_shared;

class blob{
public:
	char* data;
	size_t size;

	blob(){
		data = 0;
		size = 0;
	}

	blob(void* ptr, size_t s):data((char*)ptr),size(s){

	}

	// when len = 0 then the subblock contains the rest of the block;
	blob sub_blob(size_t offset, size_t len = 0){
		if(len == 0){
			if(offset > size){
				abmt::os::die("Offset can not be bigger then size.");
			}
			len = size-offset;
		}
		if(offset+len > size){
			abmt::os::die("unable to create subblock, because of sizelimit");
		}
		return blob((char*)data + offset, len);
	}

	void set_mem(const void* src, size_t len, size_t offset){
		if(len+offset > size){
			abmt::os::die("abmt::block to small to copy data from src...");
		}
		memcpy((char*) data + offset,src,len);
	}

	void get_mem(void* dst, size_t len, size_t offset){
		if(len+offset > size){
			abmt::os::die("abmt::block to small to copy data to src...");
		}
		memcpy(dst,(char*) data + offset,len);
	}

	template<typename T>
	inline T get(size_t offset){
		if(sizeof(T) + offset > size) abmt::os::die("Unable to convert memview. Size error");
		T res;
		// it is done that way because of the arm architecture and it's stupid alingment rules
		memcpy(&res,(data + offset),sizeof(T));
		return res;
	}

	template<typename T>
	inline void set(T&& obj, size_t offset){
		if(sizeof(T) + offset > size) abmt::os::die("Unable to convert memview. Size error");
		memcpy(data + offset, &obj, sizeof(T));
	}

	/// Like get without size check.
	template<typename T>
	inline T get2(size_t offset){
		T res;
		// it is done that way because of the arm architecture and it's stupid alingment rules
		memcpy(&res, data + offset, sizeof(T));
		return res;
	}

	/// Like set without size check.
	template<typename T>
	inline void set2(T&& data, size_t offset){
		memcpy(data + offset, &data, sizeof(T));
	}

	std::string str(size_t offset = 0, size_t len = 0){
		if(len == 0){
			if(offset > size){
				abmt::os::die("Offset can not be bigger then size.");
			}
			len = size-offset;
		}
		if(offset+len > size){
			abmt::os::die("unable to create object, because of sizelimit");
		}
		return std::string((char*)data +offset , len);
	}

	virtual ~blob(){

	}

protected:
	// for blob shared
	blob(const blob&) = default;
};


class blob_shared: public blob{
public:
	std::shared_ptr<char[]> shared_ptr;

	blob_shared(size_t s = 0):blob(0,s), shared_ptr(new char[s]){
		data = shared_ptr.get();
	}

	blob_shared(const blob_shared&) = default;

	void realloc(size_t s = 0, void* src_data_ptr_to_copy_from = 0){
		shared_ptr.reset(new char[s]);
		data = shared_ptr.get();
		size = s;
		if(src_data_ptr_to_copy_from != 0){
			set_mem(src_data_ptr_to_copy_from, s, 0);
		}
	}

	// resets the internal shared_ptr to new_data_ptr and takes over ownership
	void reset(size_t s = 0, void* new_data_ptr = 0){
		shared_ptr.reset((char*)new_data_ptr);
		data = (char*) new_data_ptr;
		size = s;
	}
};

} // namespace abmt

#endif /* _MBU_H_ */
