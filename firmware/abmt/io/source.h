/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef _IO_UTILS_SOURCE_H_
#define _IO_UTILS_SOURCE_H_ _IO_UTILS_SOURCE_H_

#include <functional>
#include <abmt/blob.h>

namespace abmt{
namespace io{

class source{
public:

	struct source_concept{
		// interface
		std::function<size_t(abmt::blob&)> on_new_data;

		// boilerplate
		virtual void copy_to(void* p) const = 0;
		virtual ~source_concept() {};
		char object[16] = {};
	};

	template<typename T>
	struct model:source_concept{
		// forward the interface
		// boilerplate
		void copy_to(void* p) const {
			new (p) model<T>(*((T*)object));
		}
		model(T t){
			static_assert(sizeof(T) <= sizeof(object), "Pointer(-object) does not fit in memory! increase concept.object-size or change pointertype.");
			*((T*)object) = t; // copy t to object (object ist ein pointer auf t)
			t->on_new_data = [this](abmt::blob& b)->size_t{
				return on_new_data(b);
			};
		}
		virtual ~model(){
			(*((T*)object)).~T();
		}
	};

	class func_forward{
	public:
		source_concept* impl = 0;

		size_t operator()(abmt::blob& b){
			if(impl == 0){
				return b.size;
			}
			return impl->on_new_data(b);
		}

		template<typename T>
		typename std::enable_if< std::is_constructible< std::function<size_t(void*,size_t)> ,T >::value >::type
		operator=(T&& rhs){
			if(impl == 0){
				return;
			}
			impl->on_new_data = rhs;
		}

		// Helper-function to create the Parameter for the rhs-functionobject out of the data-pointer and size
		template<typename T>
		typename std::enable_if< !std::is_constructible< std::function<size_t(void*,size_t)> ,T >::value >::type
		operator=(T&& rhs){
			if(impl == 0){
				return;
			}
			impl->on_new_data = [rhs](abmt::blob& b)->size_t{ return rhs(b);};
		}
	};

	// forward the interface
	func_forward on_new_data;

	// Construction and boilerplate
	template <typename T>
	source(T inp){
		new(ptr) model<T>(inp); // emplace contruct the (template)implemationion on memory
		on_new_data.impl = (source_concept*)ptr;
		on_new_data = [](abmt::blob& b)->size_t{
			// default when not set: flush buffer
			return b.size;
		};
	}

	source& operator=(const source& i){
		auto tmp = ((source_concept*)ptr)->on_new_data; // keep the old function object witch knows what to do
		((source_concept*)ptr)->~source_concept();
		((source_concept*)i.ptr)->copy_to(ptr);
		((source_concept*)ptr)->on_new_data = tmp;
		return *this;
	}

	source& operator=(const source&& i){
		*this = i;
		return *this;
	}

	source(const source& i){
		on_new_data.impl = (source_concept*)ptr;
		((source_concept*)i.ptr)->copy_to(ptr);
	}

	source(const source&& i){
		on_new_data.impl = (source_concept*)ptr;
		((source_concept*)i.ptr)->copy_to(ptr);
	}


	~source(){
		((source_concept*)ptr)->~source_concept();
	}

	class null_impl{
	public:
		std::function<size_t(abmt::blob&)> on_new_data = [](abmt::blob& b){
			return b.size; // default flush buffer
		};
		void pop(size_t){

		}
		static null_impl* get(){
			static null_impl n;
			return &n;
		}
	};

	source():source(null_impl::get()){

	}

	char ptr[sizeof(source_concept)] = {};
};

}; // namespace io
}; // namespace abmt


#endif /* INCLUDE_IO_UTILS_SOURCE_H_ */
