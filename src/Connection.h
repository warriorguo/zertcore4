/*
 * Connection.h
 *
 *  Created on: 2013-7-16
 *      Author: Administrator
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <common.h>
#include <PoolObject.h>
#include <Runtime.h>
#include <Buffer.h>
#include <Log.h>

#include <boost/asio.hpp>
#include <iostream>

namespace zertcore{ namespace net{

/**
 *
 *
 *
 */
using namespace zertcore::base;
using namespace zertcore::utils;

using namespace asio::ip;

typedef asio::io_service					io_service;

namespace errors {
/**
 * avoid "system" namespace ambiguous
 */
typedef ::boost::system::error_code			error_code_type;

typedef uint	type;
enum {
	ERROR_NONE								= 0,
	ERROR_SOCK_SHUTDOWN						= 1,
	ERROR_UNKNOWN							= 10,
};

}

enum {
	TIMER_HEARTBEAT							= 10,
	TIMER_CONNECTION_EXPIRED				= 30,
	TIMER_INTERVAL							= 1,
};

namespace messages {

enum {
	COMMAND_KEY_BASE						= 0x7ffffff,
	COMMAND_KEY_HEARTBEAT					= COMMAND_KEY_BASE + 1,
	COMMAND_KEY_RECEIVE_HEARTBEAT			= COMMAND_KEY_BASE + 2,
};

}


enum {
/**
* "Size Head" means:
* struct package {
*   uint32 size;
*   byte data[size-sizeof(uint32)];
* }
*/
	PACKAGE_TYPE_SIZEHEAD					= 1 << 0,

/**
* "Special Tail" means:
* struct package {
*    byte data[ N ]; // N is unknown
*    T sign;
* }
*/
	PACKAGE_TYPE_SPECIALTAIL				= 1 << 1,

	PACKAGE_TYPE_BOTH						= PACKAGE_TYPE_SIZEHEAD | PACKAGE_TYPE_SPECIALTAIL,
};

template <typename FC>
class Connection:
		public PoolObject<FC>,
		public Updater<FC>
{
public:
	enum {
		TYPE_NONE							= 0,
		TYPE_CLIENT							= 1 << 0,
		TYPE_SERVER							= 1 << 1,
		TYPE_BOTH							= TYPE_CLIENT | TYPE_SERVER,
	};
public:

public:
	typedef function<void (const BytePointer&)>
											read_handler_type;
	typedef function<void ()>				write_handler_type;
	typedef function<void ()>				connect_handler_type;
	typedef function<void (const errors::type&)>
											error_handler_type;

public:
	explicit Connection(io_service& ios, const uint package_type = PACKAGE_TYPE_BOTH,
								const size_t& max_size = SERVER_IO_PACKAGE_SIZE)
			: type_(TYPE_NONE), is_connected_(false), max_package_size_(max_size), holder_error_(0),
			  socket_(ios), package_type_(package_type), heartbeat_timer_(TIMER_HEARTBEAT),
			  expired_timer_(TIMER_CONNECTION_EXPIRED) {
	}
	virtual ~Connection() {
	}

public:
	tcp::socket& socket() {
		return socket_;
	}
	const tcp::socket& socket() const {
		return socket_;
	}

	void registerReadHandler(read_handler_type handler) {
		read_handler_ = handler;
	}
	void registerConnectHandler(connect_handler_type handler) {
		connect_handler_ = handler;
	}
	void registerWriteHandler(write_handler_type handler) {
		write_handler_ = handler;
	}
	void registerErrorHandler(error_handler_type handler) {
		error_handler_ = handler;
	}

	void shutdown() {
		if (isConnected()) {
			printf("shutdown()\n");
			try {
				socket_.shutdown(tcp::socket::shutdown_both);
			}
			catch(std::exception&) {
				;
			}
			setConnected(false);
		}
	}
	bool isConnected() const {
		return is_connected_;
	}
	void connect(tcp::resolver::iterator endpoint_iterator) {
		endpoint_iterator_ = endpoint_iterator;

		setType(TYPE_CLIENT);
		doConnect();
	}
	void reConnect() {
		printf("Reconnecting..\n");
		doConnect();
	}

	// send heart heat package
	void heartbeat() {
		if (heartbeat_timer_.expired(true)) {
			doSendCommand(messages::COMMAND_KEY_HEARTBEAT);
		}
	}
	bool isExpired() {
		return expired_timer_.expired();
	}
	bool update(const time_type& interval) {
		if (isExpired()) {
			LOG(NOTE) << "Expired with Connection [" << remote_address_.c_str() << "]";
			shutdown();
			return false;
		}

		heartbeat();
		return true;
	}
	void startListen() {
		setType(TYPE_SERVER);
		setConnected();

		getRemoteInfo();
		LOG(NOTE) << "Establish Connection with " << remote_address_.c_str() << "";

		if (connect_handler_)
			connect_handler_();

		doRead();
	}
	void setType(const uint type) {
		type_ |= type;
	}
	uint getType() {
		return type_;
	}

	template <typename T>
	Connection& operator << (const T& d) {
		send_buffer_.append(&d, sizeof(T));
		return *this;
	}

	template <typename T>
	Connection& operator << (const BufferPointer<T>& bp) {
		if (bp.data)
			send_buffer_.append(bp.data, bp.size * sizeof(T));

		return *this;
	}

	template <typename T>
	Connection& operator >> (T& d) {
		recv_buffer_ >> d;
		return *this;
	}

protected:
	void doSendCommand(uint command) {
		BytePointer bp(reinterpret_cast<byte *>(&command), sizeof(command));
		doSend(bp);
	}

	void doFormatSend(const BytePointer& bp, errors::type error = errors::ERROR_NONE) {
		beginFormatSend(bp.size);
		formatSending(bp);
		endFormatSend(error);

	}
	void beginFormatSend(size_t data_size) {
		uint32 total_size = (uint32)data_size;

		if (package_type_ & PACKAGE_TYPE_SPECIALTAIL) {
			total_size += sizeof(eof_key_type);
		}
		if (package_type_ & PACKAGE_TYPE_SIZEHEAD) {
			total_size += sizeof(uint32);
			send_buffer_.append(total_size);
		}
	}
	void formatSending(const BytePointer& bp) {
		send_buffer_.append(bp);
	}
	void endFormatSend(errors::type error = errors::ERROR_NONE) {
		if (package_type_ & PACKAGE_TYPE_SPECIALTAIL) {
			eof_key_type key = (eof_key_type)SERVER_IO_EOF;
			send_buffer_.append(key);
		}

		holder_error_ = error;
		doSendBuffer();
	}
	void doConnect() {
		socket_.async_connect(*endpoint_iterator_, bind(&Connection::handleConnect, this->template thisPtr(),
			asio::placeholders::error));
	}
	void doSend(const BytePointer& bp, errors::type error = errors::ERROR_NONE) {
		if (bp.data) {
			send_buffer_.append(bp);
		}

		holder_error_ = error;

		doSendBuffer();
	}
	void doRead() {
		if (isConnected()) {
			socket_.async_read_some(
				asio::buffer(recv_buffer_.writeBuffer(max_package_size_), max_package_size_),
				bind(&Connection::handleRead, this->template thisPtr(), asio::placeholders::error,
					asio::placeholders::bytes_transferred));
		}
	}

	void handleConnect(const errors::error_code_type& error_code) {
		if (!error_code) {
			LOG(NOTE) << "Connected with " << remote_address_.c_str();

			getRemoteInfo();
			setConnected();

			if (connect_handler_)
				connect_handler_();

			doRead();
		}
	}
	void handleRead(const errors::error_code_type& error_code, size_t bytes_transferred) {
		if (!error_code) {
			recv_buffer_.writeSize(bytes_transferred);

			// printf("bytes_transferred:%ld\n", bytes_transferred);

			if (bytes_transferred >= sizeof(uint32) && handleRequest()) {
				doRead();
				return ;
			}
		}
		else {
			LOG(NOTE) << "ASIO SYS ERROR[" << error_code << "]";
		}
		if (error_handler_)
			error_handler_(errors::ERROR_SOCK_SHUTDOWN);

		if (type_ & TYPE_CLIENT) {
			reConnect();
		}
		else {
			shutdown();
		}
	}
	bool handleRequest() {
		bool first_loop = true;

		do {
			size_t begin = 0, end = 0;
			bool fetch_succeed = false;

			if (recv_buffer_.empty())
				return !first_loop;

			// recv_buffer_.print();

			if (first_loop) {
			/**
			 * reset expired & heart beat timer if any message arrived.
			 */
				expired_timer_.reset();
				heartbeat_timer_.reset();
			}

			if (checkCommandKey())
				/**
				 * if the first package is command key, set first loop is over
				 */
				first_loop = false;

			do {
				if (package_type_ & PACKAGE_TYPE_SIZEHEAD) {
					fetch_succeed = fetchPackageBySizeHead(begin, end);
					if (!fetch_succeed) break;
				}
				if (package_type_ & PACKAGE_TYPE_SPECIALTAIL) {
					fetch_succeed = fetchPackageBySpecialTail(begin, end);
				}
			}while(false);

			if (!fetch_succeed) {
				/**
				 * if the package is in error format, shutdown it to avoid further damage.
				 */
				return first_loop? false: true;
			}
			first_loop = false;

			if (begin < end && read_handler_) {
				BytePointer tmp_bp(recv_buffer_.readBuffer(begin), end - begin);
				read_handler_(tmp_bp);
			}
			recv_buffer_.erase(0, end);
		}
		while(true);

		// never reach here.
		return false;
	}
	bool fetchPackageBySizeHead(size_t& begin, size_t& end) {
		if (recv_buffer_.size() < sizeof(uint32))
			return false;

		/**
		 * this size is calculating including itself.
		 */
		uint32 size = recv_buffer_.offset<uint32>(begin);

		printf("fetchPackageBySizeHead(): head size:%u buffer size:%ld\n", size, recv_buffer_.size());

		if (size <= recv_buffer_.size()) {
			end = begin + size;
			begin += sizeof(uint32);

			return true;
		}

		return false;
	}
	bool fetchPackageBySpecialTail(size_t& begin, size_t& end) {
		/**
		 * if end is setting, means it should be check [end] offset is set up with the EOF key
		 */
		if (end > 0) {
			if (end < sizeof(eof_key_type))
				return false;

			eof_key_type key = recv_buffer_.offset<uint32>(end - sizeof(eof_key_type));
			if (key != SERVER_IO_EOF) {
				return false;
			}

			return true;
		}

		// Deprecated!!
		return false;
	}
	bool checkCommandKey() {
		uint32 key = recv_buffer_.offset<uint32>(0);

		if (key > messages::COMMAND_KEY_BASE) {
			switch (key) {
			case messages::COMMAND_KEY_HEARTBEAT:
				// send heart heat package;
				doSendCommand(messages::COMMAND_KEY_RECEIVE_HEARTBEAT);
				break;
			case messages::COMMAND_KEY_RECEIVE_HEARTBEAT:
				// wouldnt do anything since the expiredtimer is updated
				// when receive the message
				break;

			default:
				break;// handle other command key
			}

			recv_buffer_.erase(0, sizeof(key));
			return true;
		}

		return false;
	}

public:
	string getRemoteAddress() const {
		return remote_address_;
	}

private:
	void getRemoteInfo() {
		try {
			tcp::endpoint remote_ep = socket_.remote_endpoint();
			address remote_ad = remote_ep.address();

			remote_address_ = remote_ad.to_string();
		}
		catch(std::exception&) {
			;
		}
	}

	void doSendBuffer() {
		if (!send_buffer_.empty() && isConnected()) {
			errors::error_code_type error_code;

			asio::write(socket_, asio::buffer(send_buffer_, send_buffer_.size()), error_code);
			/**
			asio::async_write(socket_, asio::buffer(send_buffer_, send_buffer_.size()),
					bind(&Connection::handleWrite, this->template thisPtr(), asio::placeholders::error));
			*/
			handleWrite(error_code);
		}
	}

	void handleWrite(const errors::error_code_type& error_code) {
		if (!error_code) {
			send_buffer_.clear();

			if (write_handler_)
				write_handler_();
		}
		else {
			setConnected(false);
			if (type_ & TYPE_CLIENT)
				reConnect();
			else
				shutdown();
		}
	}

	void setConnected(bool flag = true) {
		is_connected_ = flag;
		if (is_connected_) {
			expired_timer_.reset();
			heartbeat_timer_.reset();

			this->template enableUpdate(TIMER_INTERVAL);
			doSendBuffer();
		}
		else {
			this->template disableUpdate();
		}
	}

private:
	uint						type_;
	bool						is_connected_;

	string						remote_address_;
	const size_t				max_package_size_;
	uint						holder_error_;

	ByteBuffer					recv_buffer_;
	ByteBuffer					send_buffer_;

	tcp::socket					socket_;

	const uint					package_type_;

	connect_handler_type		connect_handler_;
	read_handler_type			read_handler_;
	write_handler_type			write_handler_;
	error_handler_type			error_handler_;

	ExpiredTimer				heartbeat_timer_;
	ExpiredTimer				expired_timer_;

	tcp::resolver::iterator		endpoint_iterator_;
};

/**
 * ConnectionAllocator<Server, Session>
 */
template <class Server, class Session>
class ConnectionAllocator
{
public:
	typename Session::ptr alloc(Server& server, io_service& ios) const {
		return load<Session, Server, asio::io_service> (server, ios);
	}

public:
	static ConnectionAllocator<Server, Session> instance;
};

template <class Server, class Session>
ConnectionAllocator<Server, Session> ConnectionAllocator<Server, Session>::instance;


/**
 *
 * ConnectionOperator<FC, Session, SessionAllocator>
 *
 */
template <class FC, class Session, class SessionAllocator = ConnectionAllocator<FC, Session> >
class ConnectionOperator
{
public:
	typedef typename Session::ptr			session_ptr;
	typedef SessionAllocator&				session_allocator_type;

public:
	explicit ConnectionOperator(io_service& ios,
			session_allocator_type session_allocator = SessionAllocator::instance)
		: io_service_(ios), acceptor_(ios), resolver_(ios),
		  session_allocator_(session_allocator) {
		;
	}

	virtual ~ConnectionOperator() {}

public:
	void bind(const string& host, int port) {
		address addr; addr.from_string(host);
		tcp::endpoint endpoint(addr, port);

		acceptor_.open(endpoint.protocol());
		acceptor_.set_option(tcp::acceptor::reuse_address(true));
		acceptor_.bind(endpoint);
		acceptor_.listen();

		startAccept();
	}

	session_ptr connect(const string& host, int port) {
		tcp::resolver::query query(tcp::v4(), host, lexical_cast<string>(port));
		tcp::resolver::iterator iterator = resolver_.resolve(query);

		session_ptr conn = session_allocator_.template alloc(dynamic_cast<FC&>(*this), io_service_);
		conn->connect(iterator);

		return conn;
	}

private:
	void startAccept() {
		session_ptr conn = session_allocator_.template alloc(dynamic_cast<FC&>(*this), io_service_);
		acceptor_.async_accept(conn->socket(),
			::boost::bind(&ConnectionOperator::handleAccept, this, conn, asio::placeholders::error));
	}
	void handleAccept(session_ptr conn, const errors::error_code_type& error) {
		conn->startListen();
		startAccept();
	}

private:
	tcp::endpoint& buildPoint(const string& host, int port) {
		address addr; addr.from_string(host);
		static tcp::endpoint endpoint(addr, port);

		return endpoint;
	}

private:
	io_service&					io_service_;
	tcp::acceptor				acceptor_;
	tcp::resolver				resolver_;
	session_allocator_type		session_allocator_;
};

}}

#endif /* CONNECTION_H_ */
