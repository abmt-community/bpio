/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef UTIL_INC_SIMPLE_JSON_H_
#define UTIL_INC_SIMPLE_JSON_H_

#include <string>
#include <vector>
#include <map>
#include <initializer_list>
#include <memory>

namespace abmt{
class json;
struct json_hash_entry;

typedef double num_t;
typedef std::string string_t;
typedef std::vector<json> array_t;
typedef std::vector<json_hash_entry> hash_t;

namespace json_details{

	class iterator; // forward decl

	struct iterator_output{
		json&   value;
		string_t key;
		size_t   index;
		iterator& it;
		iterator_output(iterator& it, json& v, string_t k, size_t i);
		void erase();
	};

	class iterator{
	public:
		json& src;
		size_t obj_itr = 0;
		size_t arr_itr = 0;
		size_t index = 0;

		size_t num_erased = 0;

		iterator(json& src);
		iterator operator++();
	    iterator_output operator*();
		bool operator!=(iterator& lhs);
	};

	std::string escaped_str(std::string str);
	std::string un_escaped_str(const std::string& str);

} // namspace json_details

class json{
public:
	enum types {not_init, boolean, num, str_type, obj, arr};
	union type_data{
		bool boolean;
		num_t number;
		string_t* str;
		array_t* arr;
		hash_t* obj;
	};

	types type = not_init;
	type_data v = {};

public:
	json();
	~json();

	void destruct(); /// resets to type no init

	json(bool val);
	json(string_t val);
	json(const char* val);
	json(num_t n);
	json(int n);
	json(std::initializer_list<json> list);

	template<typename T>
	json(const std::vector<T>& src){
		type = arr;
		v.arr = new array_t;
		for(auto i:src){
			v.arr->push_back(i);
		}
	}

	template<typename T>
	json(const std::vector<std::shared_ptr<T>>& src){
		type = arr;
		v.arr = new array_t;
		for(auto i:src){
			v.arr->push_back(*i);
		}
	}

	template<typename T>
	json(const std::map<std::string, T>& src){
		type = obj;
		v.obj = new hash_t;
		for(auto i:src){
			v.obj->operator [](i.first) = i.second;
		}
	}

	template<typename T>
	json(const std::map<std::string, std::shared_ptr<T>>& src){
		type = obj;
		v.obj = new hash_t;
		for(auto i:src){
			v.obj->operator [](i.first) = *(i.second);
		}
	}

	static json array(std::initializer_list<json> list = {});
	static json object();
	static json string(string_t s = "");
	static json number(num_t n = 0);

	void merge(json obj);
	void swap(json& partner);

	json(const json& copy);
	json& operator=(const json& rhs);

	json(json&& rhs);
	json& operator=(json&& rhs);

	bool is_object();
	bool is_array();
	bool is_num();
	bool is_bool();
	bool is_string();

	operator string_t();
	operator num_t();

	string_t str();

	json& operator[](json idx);
	json& operator[](const char* idx);
	json& operator[](string_t idx);
	json& operator[](int idx);

	size_t count(string_t idx);
	bool key_exsists(string_t idx);
	size_t empty();
	size_t len();
	size_t length();

	void clear();

	void erase(size_t idx);
	void erase(string_t key);
	void push_back(const json& val);
	void push_back(json&& val);
	void pop_back(json val);

	json& front();
	json& back();

	std::string dump(std::string intend = "", bool in_one_line = false);
	static json parse(void* data_ptr, size_t data_len);
	static json parse(const string_t string);

	// Wird nur für String gebraucht
	bool operator==(const char* val);
	bool operator==(const string_t& s);
	bool operator!=(const char* val);
	bool operator!=(const string_t& s);
	json& trim();

	void log(string_t s);

	json_details::iterator begin();
	json_details::iterator end();

}; // value

struct json_hash_entry{
	string_t key;
	json value;
};


} // namespace abmt


#endif /* UTIL_INC_SIMPLE_JSON_H_ */
