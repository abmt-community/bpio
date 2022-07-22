/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#include <abmt/json.h>
#include <io_utils/mbu.h>
#include <sstream>
#include <exception>

namespace abmt{
using namespace json_details;


json::json(){
	type = not_init;
	v.obj = 0;
}

json::~json(){
	destruct();
}

void json::destruct(){
	switch (type){
	case str_type: delete v.str; break;
	case obj: delete v.obj; break;
	case arr: delete v.arr; break;
	default: break;
	}
	type = not_init;
}

json::json(bool val){
	v.boolean = val;
	type = boolean;
}
json::json(string_t val){
	v.str = new string_t(val);
	type = str_type;
}
json::json(const char* val){
	v.str = new string_t(val);
	type = str_type;
}

json::json(num_t n){
	type = num;
	v.number = n;
}

json::json(int n){
	type = num;
	v.number = n;
}

json::json(std::initializer_list<json> list){
	if(list.size() == 2 && list.begin()->type == str_type){
		// Object initialisation!!
		auto itr = list.begin();
		auto key = *itr;
		++itr;
		auto value = *itr;
		type = obj;
		v.obj = new hash_t;
		this->operator [](*(key.v.str)) = value;
	}else{
		bool all_obj = true;
		for(auto &i:list){
			if(i.type != obj){
				all_obj = false;
				break;
			}
		}
		if(all_obj){
			type = obj;
			v.obj = new hash_t;
			for(auto& o:list){
				for(auto& i:*(o.v.obj)){
					if(i.value.type != not_init){
						this->operator [](i.key) = i.value;
					}
				}
			}
		}else{
			*this = json::array(list);
		}
	}
}

json json::array(std::initializer_list<json> list){
		json res;
		res.type = arr;
		res.v.arr = new array_t(list);
		return res;
}
json json::object(){
		json res;
		res.type = obj;
		res.v.obj = new hash_t();
		return res;
}

json json::string(string_t s){
		json res;
		res.type = str_type;
		res.v.str = new string_t(s);
		return res;
}
json json::number(num_t n){
		json res;
		res.type = num;
		res.v.number = n;
		return res;
}


void json::merge(json val){
	if(type == obj && val.is_object()){
		for(auto& i:*(val.v.obj)){
			if(i.value.type != not_init){
				this->operator [](i.key) = i.value;
			}
		}
	}else if(type == arr && val.is_array()){
		for(auto v:val){
			push_back(v.value);
		}
	}
}
void json::swap(json& partner){
	auto old_type = type;
	auto old_v = v;
	type = partner.type;
	v = partner.v;
	partner.type = old_type;
	partner.v = old_v;
}

json::json(const json& copy){
	*this = copy;
}
json& json::operator=(const json& rhs){
	//std::cout << "copy" << rhs.type << std::endl;
	destruct();
	type = rhs.type;
	switch (type){
	case num:
		v.number = rhs.v.number;
		break;
	case boolean:
		v.boolean = rhs.v.boolean;
		break;
	case str_type:
		v.str = new string_t(*rhs.v.str);
		break;
	case obj:
		v.obj = new hash_t(*(rhs.v.obj));
		break;
	case arr:
		v.arr = new array_t(*(rhs.v.arr));
		break;
	default: break;
	}
	return *this;
};

json::json(json&& rhs){
	destruct();
	type = rhs.type;
	switch (type){
	case num:
		v.number = rhs.v.number;
		break;
	case boolean:
		v.boolean = rhs.v.boolean;
		break;
	case str_type:
		v.str = rhs.v.str;
		break;
	case obj:
		v.obj = rhs.v.obj;
		break;
	case arr:
		v.arr = rhs.v.arr;
		break;
	default: break;
	}
	rhs.type = not_init;
}

json& json::operator=(json&& rhs){
	destruct();
	//std::cout << "move" << rhs.type << std::endl;
	type = rhs.type;
	switch (type){
	case num:
		v.number = rhs.v.number;
		break;
	case boolean:
		v.boolean = rhs.v.boolean;
		break;
	case str_type:
		v.str = rhs.v.str;
		break;
	case obj:
		v.obj = rhs.v.obj;
		break;
	case arr:
		v.arr = rhs.v.arr;
		break;
	default: break;
	}
	rhs.type = not_init;
	return *this;
}

bool json::is_object(){
	if(type == obj){
		return true;
	}
	return false;
}

bool json::is_array(){
	if(type == arr){
		return true;
	}
	return false;
}
bool json::is_num(){
	if(type == num){
		return true;
	}
	return false;
}
bool json::is_bool(){
	if(type == boolean){
		return true;
	}
	return false;
}
bool json::is_string(){
	if(type == str_type){
		return true;
	}
	return false;
}

json::operator string_t(){
	switch (type){
		case not_init: return string_t("");
		case num: return dump();
		case str_type: return *v.str;
	}
}

string_t json::str() {
	switch (type){
		case not_init: return string_t("");
		case num: return dump();
		case str_type: return *v.str;
	}
};


json::operator num_t(){
	switch (type){
		case not_init: return 0;
		case boolean: return v.boolean;
		case num: return v.number;
		case str_type: return std::stod(*v.str);
	}
}

json& json::operator[](json idx){
	switch (idx.type){
		case num:
			return this->operator[]((int)idx);
		case str_type:
			return this->operator[](idx.str());
	}
}

json& json::operator[](const char* idx){
	string_t idx2(idx);
	return this->operator[](idx2);
}

json& json::operator[](string_t idx){
	if (type == not_init){
			type = obj;
			v.obj = new hash_t;
	}
	if (type == obj){
		for(auto& h: *v.obj){
			if(h.key == idx){
				return h.value;
			}
		}
		v.obj->push_back({idx, {}});
		return v.obj->back().value;
	}
}

json& json::operator[](int idx){
	if(type == arr){
		return *(v.arr->begin() + idx);
	}
	if(type == obj){
		if(v.obj->size() > (size_t)idx){
			auto it = v.obj->begin();
			for(int i = 0; i<idx; ++i){
				++it;
			}
			return it->value;
		}
		// todo: fehlerbehanldung
	}
}

size_t json::count(string_t idx){
	switch (type){
		case not_init:
			type = obj;
			v.obj = new hash_t;
			return 0;
		case obj:
			for(auto p: *v.obj){
				if(p.key == idx){
					return 1;
				}
			}
			return 0;
	}
}

bool json::key_exsists(string_t idx){
	return count(idx);
}

size_t json::empty(){
	switch (type){
		case str_type: return v.str->empty(); break;
		case obj: return v.obj->empty(); break;
		case arr: return v.arr->empty(); break;
		default: break;
	}
	return 0;
}

size_t json::len(){
	switch (type){
		case str_type: return v.str->size(); break;
		case obj: return v.obj->size(); break;
		case arr: return v.arr->size(); break;
		default: break;
	}
	return 0;
}

size_t json::length(){
	return len();
}

void json::clear(){
	switch (type){
		case num: v.number = 0; break;
		case boolean: v.boolean = 0; break;
		case str_type: v.str->clear(); break;
		case obj: v.obj->clear(); break;
		case arr: v.arr->clear(); break;
		default: break;
	}
}

void json::erase(size_t idx){
	if(type == arr){
		v.arr->erase(v.arr->begin() + idx);
	}
	if(type == obj){
		if(v.obj->size() > idx){
			auto it = v.obj->begin();
			for(size_t i = 0; i<idx; ++i){
				++it;
			}
			v.obj->erase(it);
		}
		// todo: fehlerbehanldung;
	}
}

void json::erase(string_t k){
	if(type == obj){
		auto vec = *v.obj;
		for(auto i = vec.begin(); i != vec.end(); ++i){
			if( i->key == k){
				vec.erase(i);
				return;
			}
			++i;
		}
	}
}

void json::push_back(const json& val){
	if(type == not_init){
		type = arr;
		v.arr = new array_t;
	}
	v.arr->push_back(val);
}

void json::push_back(json&& val){
	if(type == not_init){
		type = arr;
		v.arr = new array_t;
	}
	v.arr->push_back(std::move(val));
}

void json::pop_back(json val){
	if(type == not_init){
		type = arr;
		v.arr = new array_t;
		return;
	}
	v.arr->pop_back();
}


json& json::front(){
	switch (this->type){
		case json::obj: return v.obj->begin()->value;
		case json::arr: return v.arr->front();
		default: break;
	}
	return *this;
}

json& json::back(){
	switch (this->type){
		case json::obj: return v.obj->rbegin()->value ;
		case json::arr: return v.arr->back();
		default: break;
	}
	return *this;
}


bool json::operator==(const char* val){
	if(type == str_type){
		string_t s(val);
		return *v.str == s;
	}
	return false;
}

bool json::operator==(const string_t& s){
	if(type == str_type){
		return *v.str == s;
	}
	return false;
}

bool json::operator!=(const char* val){
	if(type == str_type){
		string_t s(val);
		return *v.str != s;
	}
	return false;
}

bool json::operator!=(const string_t& s){
	if(type == str_type){
		return *v.str != s;
	}
	return false;
}

json& json::trim(){
	if(type == not_init){
		type = str_type;
		v.str = new string_t();
	}
	if(type == str_type){
		v.str->erase(0, v.str->find_first_not_of(" \n\r\t"));
		v.str->erase(v.str->find_last_not_of(" \n\r\t")+1);
	}
	return *this;
}


void json::log(string_t s){
	//std::cout << s << std::endl;
}

iterator json::begin(){
	iterator itr(*this);
	if(itr.src.type == json::obj){
		itr.obj_itr = 0;
	}
	if(itr.src.type == json::arr){
		itr.arr_itr = 0;
	}
	return itr;
}

iterator json::end(){
	iterator itr(*this);
	if(itr.src.type == json::obj){
		itr.obj_itr = itr.src.v.obj->size();
	}
	if(itr.src.type == json::arr){
		itr.arr_itr = itr.src.v.arr->size();
	}
	return itr;
}

iterator_output::iterator_output(iterator& it, json&v, string_t k, size_t i): value(v), key(k), index(i), it(it){

}

void iterator_output::erase(){
	switch (it.src.type){
		case json::obj:
			it.src.v.obj->erase(it.src.v.obj->begin() +  it.obj_itr - it.num_erased);
			++(it.num_erased);
			break;
		case json::arr:
			it.src.v.arr->erase(it.src.v.arr->begin() +  it.arr_itr - it.num_erased);
			++(it.num_erased);
			break;
		default: break;
	}
}

iterator::iterator(json& s):src(s){

}

iterator iterator::operator++(){
	++index;
	switch (src.type){
		case json::obj: ++obj_itr; break;
		case json::arr: ++arr_itr; break;
		default: break;
	}
	return *this;
};

iterator_output iterator::operator*(){
	switch (src.type){
		case json::obj: return iterator_output(*this, src.v.obj->operator[](obj_itr - num_erased).value, src.v.obj->operator[](obj_itr - num_erased).key, index);
		case json::arr: return iterator_output(*this, src.v.arr->operator[](arr_itr - num_erased),"",index);
		default: break;
	}
	return iterator_output(*this,src,"",index);
};

bool iterator::operator!=(iterator &lhs){
	switch (src.type){
		case json::obj: return (obj_itr) != lhs.obj_itr;
		case json::arr: return (arr_itr) != lhs.arr_itr;
		default: break;
	}
	return false;
};


std::string json_details::escaped_str(std::string str){
	auto str_replace = [](std::string str,std::string search, std::string replace){
		size_t pos = 0;
		while((pos = str.find(search, pos)) != std::string::npos) {
			 str.replace(pos, search.length(), replace);
			 pos += replace.length();
		}
		return str;
	};

	str = str_replace(str, "\\", "\\\\");
	str = str_replace(str, "\"", "\\\"");
	str = str_replace(str, "\b", "\\b");
	str = str_replace(str, "\f", "\\f");
	//str = str_replace(str, "\v", "\\v");
	str = str_replace(str, "\n", "\\n");
	str = str_replace(str, "\r", "\\r");
	str = str_replace(str, "\t", "\\t");
	return str;
}

std::string un_escaped_str(const std::string& str){
	std::string res;
	for(auto c_it= str.begin(); c_it != str.end();++c_it){
		if(*c_it != '\\'){
			res += *c_it;
		}else{
			++c_it;
			if (c_it == str.end()){
				break;
			}
			switch(*c_it){
			case '\\': res += '\\'; break;
			case '"': res += '"'; break;
			case 'b': res += '\b'; break;
			case 'f': res += '\f'; break;
			case 'n': res += '\n'; break;
			case 'r': res += '\r'; break;
			case 't': res += '\t'; break;
			case '\'': res += '\''; break;
			}
		}
	}
	return res;
}

json json::parse(void* data_ptr, size_t data_len){
	mbu::blob data(data_ptr, data_len);
	json res;
	json* curr_obj = &res;
	bool parse_obj_key  = false;

	std::vector<json> stack;
	size_t num_stack_elements = 0;
	std::vector<std::string> key_stack;
	size_t pos = 0;
	size_t current_line = 0;
	size_t current_line_pos = 0;


	enum class state{
		find_type,
		parse_obj,
		parse_array,
		parse_string,
		parse_num,
		parse_true,
		parse_false,
		parse_null,
		parse_arr_next_or_close,
		parse_obj_next_or_close
	};

	state s = state::find_type;

	auto ws = [&](char c){
		if(c == ' ' || c == '\t' || c == '\r'){
			return true;
		}else if(c == '\n'){
			current_line++;
			current_line_pos = 0;
			return true;
		}
		return false;
	};

	auto inc = [&](){
		++pos;
		++current_line_pos;
	};

	auto err = [&](std::string err_msg){
	};

	auto chk_after_set = [&](){
		if(num_stack_elements > 0){
			if(curr_obj->is_object()){
				curr_obj->v.obj->push_back({key_stack.back(), std::move(res)});
				res.destruct(); // in case of doubled keys
				s = state::parse_obj_next_or_close;
			}else{
				curr_obj->v.arr->emplace_back();
				curr_obj->v.arr->back().swap(res);
				s = state::parse_arr_next_or_close;
			}

		}else{
			// end; when there is no object/array on the stack the parsed
			// the parse can stop...
			pos = data_len;
		}
	};

	while(pos < data_len){
		if(s == state::find_type){
			do{
				char current_char = data.as_2<char>(pos);
				if(current_char == '{'){
					s = state::parse_obj;
					break;
				} else if(current_char == '['){
					s = state::parse_array;
					break;
				} else if(current_char == '"'){
					s = state::parse_string;
					break;
				} else if( (current_char >= '0' && current_char <= '9') || current_char == '-'){
					s = state::parse_num;
					break;
				} else if(current_char == 't'){
					s = state::parse_true;
					break;
				} else if(current_char == 'f'){
					s = state::parse_false;
					break;
				}else if(current_char == 'n'){
					s = state::parse_null;
					break;
				}else if(current_char == ']'){
					s = state::parse_arr_next_or_close;
					break;
				} else if(ws(current_char)){
					// whitespace do nothing
				}else{
					err("expecting object, array, string, number, true, false or null...");
					return {};
				}
				inc();
			}while(pos < data_len);
		} // s == state::find_type
		// after here pos is still standing on the found char

		if(s == state::parse_obj){
			inc();
			parse_obj_key = true;
			stack.emplace_back(json::object());
			++num_stack_elements;
			curr_obj = &stack.back();
			key_stack.emplace_back("");
			while(pos < data_len){
				char current_char = data.as_2<char>(pos);
				if(current_char == '"'){
					s = state::parse_string;
					break;
				}else if(ws(current_char)){
					// whitespace do nothing
				}else if(current_char == '}'){
					s = state::parse_obj_next_or_close;
					break;
				}else {
					err("expecting key (\"key\")");
					return {};
				}
				inc();
			}
		} // s == state::parse_obj

		if(s == state::parse_array){
			inc();
			s = state::find_type;
			stack.emplace_back(json::array());
			++num_stack_elements;
			curr_obj = &stack.back();
			continue;
		} // s == state::parse_array

		if(s == state::parse_string){
			inc();
			int start = pos;
			std::string str;
			while(pos < data_len){
				char current_char = data.as_2<char>(pos);
				if( current_char == '"' ){
					break;
				} else if( current_char == '\\' ){
					if(pos - start > 0){
						// string not empty
						str += data.make<std::string>(start, pos - start);
					}
					inc();
					current_char = data.as<char>(pos);
					if( current_char == 'n' ){
						str +='\n';
					}else if( current_char == 'r' ){
						str +='\r';
					}else if( current_char == '"' ){
						str +='"';
					}else if( current_char == 't' ){
						str +='\t';
					}else if( current_char == '\\' ){
						str +='\\';
					}else if( current_char == '/' ){
						str +='/';
					}else if( current_char == 'b' ){
						str +='\b';
					}else if( current_char == 'f' ){
						str +='\f';
					}else if( current_char == 'u' ){
						inc();inc();inc();inc(); // todo
					}else{
						err("un allowed escape");
					}
					start = pos+1;
				}
				inc();
			}

			if(pos - start > 0){
				str += data.make<std::string>(start, pos - start);
			}

			if(parse_obj_key){
				parse_obj_key = false;
				key_stack.back() = std::move(str);
				inc();// step over "
				while(pos < data_len){
					char current_char = data.as_2<char>(pos);
					if( current_char == ':' ){
						s = state::find_type;
						inc();// step over :
						break;
					}else if(ws(current_char)){
						// whitespace do nothing
					}else{
						err("expecting : after key");
					}
					inc();
				}
				continue;
			}else{
				res = std::move(str);
				inc();// step over "
				chk_after_set();
				continue;
			}
			// do nothing here but in if above to make it more obvious
		} // s == state::parse_string

		if(s == state::parse_num){
			auto start = pos;
			inc();
			while(pos < data_len){
				char current_char = data.as_2<char>(pos);
				if( (current_char >= '0' && current_char <= '9')
					|| current_char == '.'
					|| current_char == '-'
					|| current_char == '+'
					|| current_char == 'e'
					|| current_char == 'E'
				){
					// valid char
				}else{
					// char is not part of number; most likely a ws
					break;
				}
				inc();
			};

			std::string s = data.make<std::string>(start, pos - start);
			res = std::stod(s);
			chk_after_set();
		} // s == state::parse_num

		if(s == state::parse_true){
			if((pos +3) < data_len && data.as_2<char>(pos+1) == 'r' && data.as_2<char>(pos+2) == 'u' && data.as_2<char>(pos+3) == 'e'){
				inc(); inc(); inc(); inc();
				res = true;
				chk_after_set();
				continue;
			}else{
				err("expecting object, array, string, number, true, false or null... 2");
				return {};
			}
		} // s == state::parse_true

		if(s == state::parse_false){
			if((pos +4) < data_len &&  data.as_2<char>(pos+1) == 'a' && data.as_2<char>(pos+2) == 'l' && data.as_2<char>(pos+3) == 's' && data.as_2<char>(pos+4) == 'e'){
				inc(); inc(); inc();inc(); inc();
				res = false;
				chk_after_set();
				continue;
			}else{
				err("expecting object, array, string, number, true, false or null... 3");
				return {};
			}
		} // s == state::parse_false

		if(s == state::parse_null){
			if((pos +3) < data_len && data.as_2<char>(pos+1) == 'u' && data.as_2<char>(pos+2) == 'l' && data.as_2<char>(pos+3) == 'l'){
				inc(); inc(); inc(); inc();
				res = {};
				chk_after_set();
				continue;
			}else{
				err("expecting object, array, string, number, true, false or null... 4");
				return {};
			}
		} // s == state::parse_null

		if(s == state::parse_arr_next_or_close){
			while(pos < data_len){
				char current_char = data.as_2<char>(pos);
				if( current_char == ',' ){
					s = state::find_type;
					inc();
					break;
				}else if( current_char == ']' ){
					res.swap(*curr_obj);
					stack.pop_back();
					--num_stack_elements;
					curr_obj = &stack.back();
					chk_after_set();
					inc();
					break;
				}else if(ws(current_char)){
					// whitespace do nothing
				}else{
					err("expecting , or ]");
				}
				inc();
			}
			continue;
		} // s == state::parse_arr_next_or_close

		if(s == state::parse_obj_next_or_close){
			while(pos < data_len){
				char current_char = data.as_2<char>(pos);
				if( current_char == ',' ){
					parse_obj_key = true;
					inc();
					while(pos < data_len){
						char current_char = data.as_2<char>(pos);
						if(current_char == '"'){
							s = state::parse_string;
							break;
						}else if(ws(current_char)){
							// whitespace do nothing
						}else{
							err("expecting key (\"key\")");
							return {};
						}
						inc();
					}
					break;
				}else if( current_char == '}' ){
					res.swap(*curr_obj);
					stack.pop_back();
					--num_stack_elements;
					curr_obj = &stack.back();
					key_stack.pop_back();
					chk_after_set();
					inc();
					break;
				}else if(ws(current_char)){
					// whitespace do nothing
				}else{
					err("expecting , or }");
				}
				inc();
			}
		} // s == state::parse_obj_next_or_close
	}

	return res;
}

json json::parse(const std::string s){

	return parse((void*)s.c_str(), s.size());

	/***
	 * Wasn't the old parser a beauty? :(
	 ***
	typedef std::string::iterator sit;

	// Variables that are parsed at a certain moment
	string_t	cur_string;
	num_t		cur_num;
	value		cur_val;
	std::vector<string_t> obj_key_stack;
	std::vector<value> object_stack;
	std::vector<value> array_stack;

	//extractors
	auto ex_cur_str = axe::e_ref([&](sit a,sit b){cur_string = std::string(a,b);});


	auto json_spaces = *axe::r_any(" \t\n\r");
	auto json_hex =  axe::r_many(axe::r_hex(), 4);
	auto json_escaped = axe::r_lit('"') | '\\' | '/' | 'b' | 'f' | 'n' | 'r' | 't' | ('u' & json_hex);
	auto json_char = (axe::r_any() - '"' - '\\') | ('\\' & json_escaped);

	auto json_string = '"' & (*json_char) >> ex_cur_str  & '"' ;

	axe::r_rule<std::string::iterator> json_value;
	auto json_array = json_spaces &  axe::r_lit('[') >>axe::e_ref([&](...){array_stack.push_back(value::array());})
					& axe::r_many(json_spaces
							& json_value  >> axe::e_ref([&](...){array_stack.back().push_back(cur_val);cur_val = value();})
							& json_spaces, ',', 0
							)
					& json_spaces & ']' ;

	auto json_object =  json_spaces & axe::r_lit('{') >>axe::e_ref([&](...){object_stack.push_back(value::object());})
						& axe::r_many(json_spaces
									& json_string >>axe::e_ref([&](...){obj_key_stack.push_back(cur_string);})
									& json_spaces & ':'
									& json_spaces & json_value >> axe::e_ref([&](...){object_stack.back()[obj_key_stack.back()] = cur_val;obj_key_stack.pop_back();})
									& json_spaces, ',' , 0
						) & json_spaces & axe::r_lit('}');

	json_value	= json_string 				>> axe::e_ref([&](...){cur_val = un_escaped_str(cur_string);})
				| axe::r_double(cur_num)	>> axe::e_ref([&](...){cur_val = cur_num;})
				| json_object				>> axe::e_ref([&](...){cur_val = object_stack.back(); object_stack.pop_back();})
				| json_array				>> axe::e_ref([&](...){cur_val = array_stack.back(); array_stack.pop_back();})
				| axe::r_lit("true")		>> axe::e_ref([&](...){cur_val = value(true);})
				| axe::r_lit("false")		>> axe::e_ref([&](...){cur_val = value(false);})
				| axe::r_lit("null")		>> axe::e_ref([&](...){cur_val = value();})
				;
	json_value(s.begin(),s.end());

	return cur_val;
	*/
}

std::string json::dump(std::string intend, bool in_one_line){
	//std::stringstream stream;
	std::string res;
	bool first = true; // für komma
	if(in_one_line == true){
		intend = "";
	}
	switch (type){
		case json::num:
			res += v.number;
			break;
		case json::boolean:
			if(v.boolean){
				res += "true";
			}else{
				res += "false";
			}
			break;
		case json::str_type:
			res += "\"" + escaped_str(*(v.str)) + "\"";
			break;
		case json::obj:
			res += "{";
			for(auto i: *this){
				if(i.value.type != not_init){
					if(first == false){
						res += ",";
					}
					if(in_one_line == false){
						res += '\n';
					}
					first = false;
					res += intend + "  \""  + i.key + "\": " + i.value.dump(intend + "  ", in_one_line);
				}else{
					// Vllt null? ansonsten geht der key verloren, oder ein leeres Objekt, weil man json oft mit {} initialisiert
				}
			}
			if(in_one_line == false){
				res += '\n';
			}
			res += intend + "}";
			break;
		case json::arr:
			res += "[";
			for(auto i: *this){
				if(first == false){
					res += ",";
				}
				if(in_one_line == false){
					res += '\n';
				}
				first = false;
				res += intend + "  " + i.value.dump(intend + "  ", in_one_line);
			}
			if(in_one_line == false){
				res += '\n';
			}
			res += intend +  "]";
			break;
		default: break;
	}
	return res;
}

} // namespace abmt
