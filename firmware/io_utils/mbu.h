/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef _MBU_H_
#define _MBU_H_

#include <type_traits>
#include <cstdlib>
#include <cstring>
#include <min_os.h>
#include <memory>

namespace mbu{ // mem_bock_utils

/**
 * Helper for conversion form a block-memory(datapointer + size) to another type.
 */

class blob;
class blob_shared;
template<class T> class array;

class blob{
public:
	void* ptr;
	size_t size;

	blob(){
		ptr = 0;
		size = 0;
	}

	blob(void* ptr, size_t s):ptr(ptr),size(s){

	}


	template<typename T>
	static blob from(T& d){
		blob res;
		static_assert(std::is_base_of<blob, T>::value == false, "Copying a block is not allowed. You may forgot an & in a function-parameter.");
		res.ptr = &d;
		res.size = sizeof(T);
		return res;
	}

	// when len = 0 then the subblock contains the rest of the block;
	blob sub_block(size_t offset, size_t len = 0){
		if(len == 0){
			if(offset > size){
				min_os.die("Offset can not be bigger then size.");
			}
			len = size-offset;
		}
		if(offset+len > size){
			min_os.die("unable to create subblock, because of sizelimit");
		}
		return blob((char*)ptr + offset, len);
	}

	void set_mem(const void* src, size_t len, size_t offset = 0){
		if(len+offset > size){
			min_os.die("mbu::block to small to copy data from src...");
		}
		memcpy((char*) ptr + offset,src,len);
	}

	void get_mem(void* dst, size_t len, size_t offset = 0){
		if(len+offset > size){
			min_os.die("mbu::block to small to copy data to src...");
		}
		memcpy(dst,(char*) ptr + offset,len);
	}

	template<typename T>
	inline T& as(size_t offset = 0){
		if(sizeof(T) + offset > size) min_os.die("Unable to convert memview. Size error");
		return *(T*) ((char*) ptr + offset);
	}

	template<typename T>
	inline T& as_2(size_t offset = 0){
		return *(T*) ((char*) ptr + offset);
	}

	// todo: vllt abmt::const_array hier integrieren
	template<typename T>
	array<T> as_array_of(size_t offset = 0){
		return array<T>((char*)ptr + offset, size-offset);
	}

	template<typename T>
	T make(size_t offset = 0, size_t len = 0){
		if(len == 0){
			if(offset > size){
				min_os.die("Offset can not be bigger then size.");
			}
			len = size-offset;
		}
		if(offset+len > size){
			min_os.die("unable to create object, because of sizelimit");
		}
		return T((char*)ptr +offset , len);
	}

	virtual ~blob(){

	}
};


class blob_shared: public blob{
public:
	std::shared_ptr<char[]> shared_ptr;

	blob_shared(size_t s = 0):blob(0,s), shared_ptr(new char[s]){
		ptr = shared_ptr.get();
	}

	void realloc(size_t s = 0, void* src_data_ptr_to_copy_from = 0){
		shared_ptr.reset(new char[s]);
		ptr = shared_ptr.get();
		size = s;
		if(src_data_ptr_to_copy_from != 0){
			set_mem(src_data_ptr_to_copy_from,s);
		}
	}

	// resets the internal shared_ptr to new_data_ptr and takes over ownership
	void reset(size_t s = 0, void* new_data_ptr = 0){
		shared_ptr.reset((char*)new_data_ptr);
		ptr = new_data_ptr;
		size = s;
	}
};

template<typename T>
class array: public blob{
	friend class blob;
	// Important: Never add attributes to this class! It needs the same memorylayout as block
	array(void* data, size_t s):blob(data,s){

	}

public:

	array(const array&) = delete; // never copy; eg. would make problems with reference_count with block_shared

	size_t length(){
		return size/(sizeof(T) );
	}

	T* begin(){
		return (T*)ptr;
	}

	T* end(){
		return (T*)ptr + length();
	}


	T& operator[](size_t idx){
		if(idx > length()){
			min_os.die("out of memory");
		}
		return ((T*)ptr)[idx];
	}
};

} // namespace mbu

#endif /* _MBU_H_ */
