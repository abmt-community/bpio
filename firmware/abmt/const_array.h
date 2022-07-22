/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef SHARED_ABMT_CONST_ARRAY_H_
#define SHARED_ABMT_CONST_ARRAY_H_

namespace abmt{

template<class T>
class const_array{
public:
	unsigned int length = 0;
	T* array = 0;

	const_array() = default;
	const_array(const_array& c) = delete; // could mixup underling reference_counts

	template<int N>
	const_array( T(&ptr)[N] ){
		length = N;
		array = ptr;
	}

	const_array( T* ptr, unsigned int len){
			length = len;
			array = ptr;
		}


	T& operator[](unsigned int pos) const {
		if(pos > length -1){
			throw "operator[] out of range";
		}
		return array[pos];
	}

	T* begin(){
		return array;
	}
	T* end(){
		return array + length;
	}
};


} // namespace abmt

#endif /* SHARED_ABMT_CONST_ARRAY_H_ */
