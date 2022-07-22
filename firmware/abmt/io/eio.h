/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef INC_EIO_H_
#define INC_EIO_H_ INC_EIO_H_

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <deque>
#include <abmt/blob.h>

namespace abmt{
namespace io{

// todo: when you are sending a lot of data you might get a tcp::send: Resource temporarily unavailable
//       fix with copy and queue

class event;


using e_ptr = std::shared_ptr<event>;

class event_list{
	int list_fd;
	bool break_loop = false; // use to break the loop after an event; Needed after socket close to not read on closed socket...
public:

	event_list();
	virtual ~event_list();
	void add(event& e);
	void remove(event& e);
	void modify(event& e); /// updates e.ev

	void wait(uint64_t timeout_ms = 1000);
};


/**
 * event
 *
 * Usage:
 *  - derive from event
 *  - configure fd
 *  - call lst.add(*this); !!!!
 *
 *  lst.remove will be called for you in ~event();
 */
class event{
public:

	int fd;
	epoll_event ev;
	event_list& lst;

	std::function<void()> epoll_in;
	std::function<void()> epoll_out;
	std::function<void()> epoll_hup; // hang_up

	event(event_list& lst);
	event(const event&) = delete; // Important: Never copy because [this]-capture cannot be copied
	event(event&&) = delete;      // Important: Never copy because [this]-capture cannot be copied
	virtual ~event();
};


const size_t ms   = 1;
const size_t sec  = 1000*ms;
const size_t min  = 60*sec;
const size_t hour = 60*min;
const size_t day  = 24*hour;
const size_t year = 365*day;

class timer: public event{
	uint64_t interval_ns;
public:
	std::function<void()> on_expiraton;


	timer(event_list& lst, size_t interval_ms = 0, std::function<void()> on_expiraton = []{});

	uint64_t get_interval_ms();
	void set_interval_ms(uint64_t ms);
	void stop();
};

class ip_addr{
public:
	in_addr adr;
	ip_addr(const char* s);
	/// Resolves str. Can also be a ip-address in string from. Waring: blocking call. See resolve().
	ip_addr(std::string str);
	ip_addr(in_addr adr);
	ip_addr(in_addr_t ip);
	// todo: compare etc..
	std::string str();
};

/// Resolves the ip address for a given name. You can also give an ip-addess as string.
/// Then the same ip address is returned. This makes it easier to handle parameters.
/// "" resolves to localhost. "any", "INADDR_ANY" and "inaddr_any" resolve to INADDR_ANY;
/// Warning: Call is blocking.
std::vector<abmt::io::ip_addr> resolve(std::string name);

class end_point{
public:
	ip_addr ip;
	uint16_t port;

	end_point(uint16_t port = 0): ip(INADDR_ANY), port(port){

	}

	end_point(ip_addr ip, uint16_t port): ip(ip), port(port){

	}

	operator sockaddr_in(){
		sockaddr_in target;
		target.sin_addr = ip.adr;
		target.sin_family = AF_INET;
		target.sin_port = htons(port);
		return target;
	}

	//end_point(std::string); // example: localhost:5000
};

struct buffer{
	char* data;
	size_t size;
	size_t offset;
	buffer(const void* src_data_to_copy, size_t data_size);
	buffer(const buffer& copy) = delete;

	~buffer();
};

/**
 * Input-Buffer-Handling:
 * io_base holds a buffer, that is passed as data on on_new_data with the number of bytes in the buffer
 * as size. "on_new_data" returns the number of bytes that are read from the upper layer.
 * io_base does the memove for the remaining bytes in the buffer.
 */

class io_base: public event{
public:
	std::function<size_t(abmt::blob&)> on_new_data = [](abmt::blob& b)->size_t{ return b.size;};
	std::deque<buffer> out_data_que;

	io_base(event_list& lst, size_t buffer_size = 64*1024);
	void set_buffer_size(size_t s);
	virtual void send(const void* data, size_t len) = 0;

	void send(abmt::blob&);

	template<typename T>
	void send(T&& o){
		send(&o, sizeof(T));
	}

	template<typename T>
	void send(std::shared_ptr<T> o){
		send(&(*o), sizeof(T));
	}

	virtual ~io_base();

private:
	char*  in_buffer = 0;
	size_t in_buffer_size  = 0;
	size_t bytes_read = 0;
};

/**
 * Input-Buffer-Notice:
 * you must set the the Buffer to an Object large enough
 * to hold a complete data. The rest, that does not fit
 * in your the buffer, will be ignored...
 */
class udp: public io_base{
public:
	end_point src;
	end_point dst;

	/// creates a udp connection to dst; src is random
	udp(event_list& lst, end_point dst);

	/// creates a udp connection that listens to src and sends to dst. You can initialize
	/// dst with {} when you don't care and only want to have a server that listens to src...
	udp(event_list& lst, end_point dst, end_point src);

	void send_to(const void* data, size_t len, end_point dst);

	using io_base::send;
	virtual void send(const void* data, size_t len);

};
using udp_ptr = std::shared_ptr<udp>;

class tcp;
using tcp_ptr = std::shared_ptr<tcp>;


class tcp: public io_base{
public:
	static tcp_ptr connect_to(event_list& lst, end_point dst);
	static tcp_ptr connect(event_list& lst, end_point src, end_point dst);

public:
	tcp(event_list& lst);

	bool connected = false;

	std::function<void()> on_open;
	std::function<void()> on_ready_to_send;
	std::function<void()> on_close;


	using io_base::send;
	virtual void send(const void* data, size_t len);

	void setup_fd();
	void bind(end_point);
	void connect(end_point);
	void close();
};


class tcp_server: public event{
public:
	tcp_server(event_list& lst, end_point p);
	std::function<void(tcp_ptr)> on_new_connection = [](tcp_ptr p){};
};
using tcp_server_ptr = std::shared_ptr<tcp_server>;


class serial: public io_base{
public:
	serial(event_list& lst, std::string device = "", int baudrate = 19200);
	void open(std::string device, int baudrate = 19200);
	void set_baudrate(int baudrate);
	virtual void send(const void* data, size_t len);
};

using serial_ptr = std::shared_ptr<serial>;


} // namespace io
} // namespace abmt

#endif /* INC_EIO_H_ */
