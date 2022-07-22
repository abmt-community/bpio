/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef _IO_UTILS_SINK_H_
#define _IO_UTILS_SINK_H_ _IO_UTILS_SINK_H_

#include <stdio.h>
#include <abmt/blob.h>

namespace abmt{
namespace io{

class sink{
public:

	struct sink_concept{
		// interface
		virtual void send(const void* data, size_t size) = 0;
		// boilerplate
		virtual void copy_to(void* p) const = 0;
		virtual ~sink_concept() {};
		char object[16] = {};
	};

	template<typename T>
	struct model:sink_concept{
		// forward the interface
		void send(const void* data, size_t size) override {
		   (*((T*)object))->send(data,size); // ugly c++-bullshit
		}
		// boilerplate
		void copy_to(void* p) const {
			new (p) model<T>(*((T*)object));
		}
		model(T t){
			static_assert(sizeof(T) <= sizeof(object), "Pointer(-object) does not fit in memory! increase concept.object-size or change pointertype.");
			*((T*)object) = t;
		}
		virtual ~model(){
			(*((T*)object)).~T();
		}
	};

	// forward the interface
	void send(const void* data, size_t size){
		((sink_concept*)ptr)->send(data,size);
	}

	// utilities

	void send(abmt::blob& b){
		send(b.data, b.size);
	}

	template<typename T>
	void send(T&& o){
		send(&o, sizeof(T));
	}

	template<typename T>
	void send(std::shared_ptr<T> o){
		send(&(*o), sizeof(T));
	}

	// Construction and boilerplate
	template <typename T>
	sink(T inp){
		new(ptr) model<T>(inp); // emplace contruct the (template)implemationion on memory
	}

	sink& operator=(const sink& i){
		((sink_concept*)ptr)->~sink_concept();
		((sink_concept*)i.ptr)->copy_to(ptr);
		return *this;
	}

	sink& operator=(const sink&& i){
		*this = i;
		return *this;
	}

	sink(const sink& i){
		((sink_concept*)i.ptr)->copy_to(ptr);
	}

	sink(const sink&& i){
		((sink_concept*)i.ptr)->copy_to(ptr);
	}


	~sink(){
		((sink_concept*)ptr)->~sink_concept();
	}

	class null_impl{
	public:
		void send(const void* data, size_t size){

		}
		static null_impl* get(){
			static null_impl n;
			return &n;
		}
	};

	sink():sink(null_impl::get()){

	}

	char ptr[sizeof(sink_concept)];
};


}; // namespace io
}; // namespace abmt

#endif /* INCLUDE_IO_UTILS_SINK_H_ */
