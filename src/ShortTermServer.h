/*
 * ShortTermServer.h
 *
 *  Created on: 2013-9-23
 *      Author: Administrator
 */

#ifndef SHORTTERMSERVER_H_
#define SHORTTERMSERVER_H_

#include <common.h>
#include <Buffer.h>
#include <PoolObject.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace zertcore{ namespace net{

using namespace boost::asio;
using namespace boost::asio::ip;

using namespace zertcore::base;
using namespace zertcore::utils;

namespace errors {

typedef ::boost::mutex						mutex_type;
typedef ::boost::system::error_code			error_code_type;

}

template <typename T>
class Queue
{
public:
	typedef list<T>							container_type;

	Queue() {}

public:
	bool push(const T& item) {
		try {
			mutex_.lock();
		}
		catch(lock_error& ) {
			mutex_.unlock();
			return false;
		}

		container_.push_back(item);
		mutex_.unlock();

		return true;
	}

	bool push(const container_type& container) {
		try {
			mutex_.lock();
		}
		catch(lock_error& ) {
			mutex_.unlock();
			return false;
		}

		container_.insert(container_.end(), container.begin(), container.end());
		mutex_.unlock();

		return true;
	}

	bool popAll(container_type& container) {
		if (!mutex_.try_lock()) {
			return false;
		}

		container = container_;
		container_.clear();

		mutex_.unlock();

		return !container.empty();
	}

	bool pop(T& item) {

		if (!mutex_.try_lock()) {
			return false;
		}

		if (container_.empty()) {
			mutex_.unlock();
			return false;
		}

		item = container_.front();
		container_.pop_front();

		mutex_.unlock();
		return true;
	}

	size_t size() const {
		return container_.size();
	}

private:
	mutex_type					mutex_;
	container_type				container_;
};

/**
 * ShortTermConnection
 * bool Server.newMessage(const BytePointer& request, string& response);
 * if return true request was done, if return false disconnect directly
 */
template <class Server, size_t BufferSize>
class ShortTermConnection :
		public PoolObject<ShortTermConnection<Server, BufferSize> >
{
	typedef ShortTermConnection				self;
public:
	typedef Server							server_type;
	enum {
		MAX_BUFFER_SIZE						= BufferSize,
	};
	enum {
		ACTION_NONE							= 0,
		ACTION_WRITE						= 1,
		ACTION_SHUTDOWN						= 10,
	};

public:
	explicit ShortTermConnection(server_type& server, io_service& ios) :
		server_(server), socket_(ios), action_(ACTION_NONE), port_(0), package_size_(0),
		received_size_(0) {
		;
	}

	virtual ~ShortTermConnection() {
		;
	}

	void start() {
		socket_.async_read_some(buffer(request_, MAX_BUFFER_SIZE - 1),
	    		bind(&self::handleRead, this->thisPtr(),
	    			placeholders::error, placeholders::bytes_transferred));
	}

public:
	tcp::socket& socket() {
		return socket_;
	}

	string getRemoteAddress() {
		if (!ip_.empty()) {
			return ip_;
		}

		try {
			return ip_ = socket_.remote_endpoint().address().to_string();
		}
		catch(std::exception&) {
			;
		}

		return Null;
	}

	uint getRemotePort() {
		if (port_) return port_;

		try {
			return port_ = socket_.remote_endpoint().port();
		}
		catch(std::exception&) {
			;
		}

		return Null;
	}

private:
	void handleRead(const errors::error_code_type& error, size_t bytes_transferred) {
		if (!error) {
//			ByteBuffer response;

			if (getRemoteAddress().empty()) {
				shutdown();
				return ;
			}

			if (!package_size_) {
				if (bytes_transferred >= sizeof(uint)) {
					package_size_ = *((uint *)request_);

					if (package_size_ >= MAX_BUFFER_SIZE) {
//						::printf("handleRead() : Package Size : %u\n", package_size_);
						//shutdown();
						response_.append(string("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n<h1>Nothing's Here Guy.</h1>"));
						write();
						return ;
					}
				}
				else {
					shutdown();
					return ;
				}
			}

			received_size_ += bytes_transferred;
			if (received_size_ < package_size_) {
//				::printf("handleRead() : Receive size : %u, Package Size : %u\n", received_size_, package_size_);

				if (received_size_ >= MAX_BUFFER_SIZE) {
					shutdown();
					return ;
				}

				async_read(socket_, buffer(&request_[received_size_], package_size_ - received_size_),
			    		bind(&self::handleRead, this->thisPtr(),
			    			placeholders::error, placeholders::bytes_transferred));
				return ;
			}

			request_[received_size_] = 0;

			response_.clear();

			server_.template addInConn(this->thisPtr());
			/**
			if (server_.template newMessage(this->thisPtr(),
					BytePointer(request_ + sizeof(uint), received_size_ - sizeof(uint)), response_)) {

				try {
				async_write(socket_, buffer(response_.data(), response_.size()),
						bind(&self::handleWrite, this->thisPtr(), placeholders::error));
				}
				catch(...) {
					::printf("handleRead() : try catch failed\n");
					shutdown();
				}
			}
			else {
				::printf("handleRead() : newMessage failed\n");
				shutdown();
			}
			*/
		}
		else {
//			::printf("handleRead() failed : %s\n", error.message().c_str());
		}
	}

	void handleWrite(const errors::error_code_type& error) {
		if (error) {
//			::printf("handleWrite() failed : %s\n", error.message().c_str());
		}
		shutdown();
	}

	void shutdown() {
		errors::error_code_type error_code;
//		socket_.shutdown(tcp::socket::shutdown_both, error_code);
		socket_.close(error_code);
	}

private:
	void write() {
		try {
			async_write(socket_, buffer(response_.data(), response_.size()),
				bind(&self::handleWrite, this->thisPtr(), placeholders::error));
		}
		catch(...) {
//			::printf("handleRead() : try catch failed\n");
			shutdown();
		}
	}

public:
	string& getResponse() {
		return response_;
	}
	BytePointer getRequest() {
		return BytePointer(request_ + sizeof(uint), received_size_ - sizeof(uint));
	}

	void setAction(uint action) {
		action_ = action;
	}

	void action() {
		switch(action_) {
		case ACTION_WRITE:
			write();
			break;
		case ACTION_SHUTDOWN:
		default:
			shutdown();
			break;
		}
	}

private:
	server_type&				server_;
	tcp::socket					socket_;
	string						response_;

	uint						action_;

private:
	string						ip_;
	uint						port_;

private:
	uint						package_size_, received_size_;
	byte						request_[MAX_BUFFER_SIZE];
};

/**
 * ShortTermServer
 */
class ShortTermServer
{
	typedef ShortTermServer					self;

public:
	typedef ShortTermConnection<ShortTermServer, SERVER_IO_PACKAGE_SIZE>
											conn_type;
	typedef conn_type::ptr					conn_ptr;
	typedef Queue<conn_ptr>					conn_list_type;

public:
	explicit ShortTermServer(io_service& ios) : stop_flag_(false), ios_(ios), acceptor_(ios) {
		;
	}
	virtual ~ShortTermServer() {
		;
	}

public:
	void listen(const string& host, int port) {
		address addr; addr.from_string(host);
		tcp::endpoint endpoint(addr, port);

		acceptor_.open(endpoint.protocol());
		acceptor_.set_option(tcp::acceptor::reuse_address(true));
		acceptor_.bind(endpoint);
		acceptor_.listen();

		stop_flag_ = false;
		new thread(bind(&ShortTermServer::run, this));
	}

	void close() {
		stop_flag_ = true;
	}

public:
	virtual bool newMessage(conn_ptr conn, const BytePointer& request, string& response) {
		return false;
	}

public:
	bool addInConn(conn_ptr conn) {
		return conn_in_list_.push(conn);
	}
	bool addOutConn(conn_ptr conn) {
		return conn_out_list_.push(conn);
	}

public:
	/**
	 * must be working in the game thread
	 */
	size_t workOnce() {
		conn_ptr conn;
		size_t handle_amount = 0;

		conn_list_type::container_type list, push_list;
		if (conn_in_list_.popAll(list)) {
			for (conn_list_type::container_type::iterator it = list.begin();
					it != list.end(); ++it) {
				conn_ptr conn = *it;
				if (!conn) continue;

				if (newMessage(conn, conn->getRequest(), conn->getResponse())) {
					conn->setAction(conn_type::ACTION_WRITE);
					handle_amount++;
				}
				else {
					conn->setAction(conn_type::ACTION_SHUTDOWN);
				}

				push_list.push_back(conn);
			}
		}
		conn_out_list_.push(push_list);

		return handle_amount;
	}

protected:
	void startAccept() {
		conn_ptr conn = load<conn_type>(*this, ios_);
		acceptor_.async_accept(conn->socket(),
				bind(&self::handleAccept, this, conn, placeholders::error));
	}

	void handleAccept(conn_ptr conn, const errors::error_code_type& error) {
		if (!error) {
			conn->start();
		}

		startAccept();
	}

	void run() {
		startAccept();

		while(true) {
			errors::error_code_type error_code;
			ios_.poll_one(error_code);

			conn_list_type::container_type list, push_list;
			if (conn_out_list_.popAll(list)) {
				for (conn_list_type::container_type::iterator it = list.begin();
						it != list.end(); ++it) {
					conn_ptr conn = *it;
					if (!conn) continue;
					conn->action();
				}
			}
			else {
				usleep(1);
			}

			if (stop_flag_)
				break;
		}

		errors::error_code_type error;
		acceptor_.close(error);
	}

private:
	conn_list_type				conn_in_list_, conn_out_list_;

private:
	bool						stop_flag_;
	io_service&					ios_;
	tcp::acceptor				acceptor_;
};

}}

#endif /* SHORTTERMSERVER_H_ */
