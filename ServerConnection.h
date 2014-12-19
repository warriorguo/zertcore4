/*
 * ServerConnection.h
 *
 *  Created on: 2013-7-19
 *      Author: Administrator
 */

#ifndef SERVERCONNECTION_H_
#define SERVERCONNECTION_H_

#include <common.h>
#include <ActionMap.h>
#include <MultiEntranceMap.h>
#include <Connection.h>

namespace zertcore{ namespace net{

using namespace zertcore::base;
using namespace zertcore::utils;

namespace errors {


enum {
	ERROR_VERIFY_FAILED			= 13,
};

}

class Server;
class ServerConnectionManager;

/**
 * each Server Connection connects an object, several connections may connects one object
 */
class ServerConnection :
		public Connection<ServerConnection>
{
	friend class ServerConnectionManager;
public:
	typedef ServerConnection*				ptr;

public:
	typedef MultiEntranceMap<uuid_t, ptr>	connection_map_type;
	typedef connection_map_type::node_ptr	connection_node_ptr;

public:
	typedef ActionMap<uint, void (ptr, const uuid_t uid, const BytePointer&)>
											action_map_type;
	typedef action_map_type::action_handler_type
											action_handler_type;

public:
	typedef array<byte, SERVER_IO_PACKAGE_SIZE>
											buffer_type;

public:
	struct Encrypt
	{
		enum {
			none				= 0,
			rsa					= 10,
			md5					= 11,
		};

		enum {
			md5_size			= 32,
		};

		uint32	type;

		typedef byte	md5_data_type[md5_size];
	};

	struct package
	{
		uuid_t					uid;
		Encrypt					encrypt;
		action_index_t			action;

		union {
			struct {
				Encrypt::md5_data_type	md5;
				byte			content[0];
			};
			byte				data[0];
		};
	};

public:
	explicit ServerConnection(Server& server, ServerConnectionManager& manager,
			asio::io_service& ios) : Connection<ServerConnection>(ios), server_(server) {
		registerReadHandler(bind(&ServerConnection::onRead, this, _1));
		registerWriteHandler(bind(&ServerConnection::onWrite, this));
		registerErrorHandler(bind(&ServerConnection::onError, this, _1));
	}

public:
	void onRead(const BytePointer& bp) {
		const package* pkg = reinterpret_cast<const package *>(bp.data);

		switch(pkg->encrypt.type) {
		case Encrypt::none:
			serverContent(pkg->uid, BytePointer(pkg->data, bp.size - STRUCT_OFFSET(package, data)));
			break;

		case Encrypt::rsa: {
			decryptRSA(BytePointer(pkg->data, bp.size - STRUCT_OFFSET(package, data)), buffer_);
			serverContent(pkg->uid, BytePointer(buffer_.data(), buffer_.size()));
			break;
		}

		case Encrypt::md5:
			if (!verifyMD5(pkg->md5, BytePointer(pkg->data, bp.size - STRUCT_OFFSET(package, content)))) {
				// errors::ERROR_VERIFY_FAILED;
				return ;
			}

			serverContent(pkg->uid, BytePointer(pkg->content, bp.size - STRUCT_OFFSET(package, content)));
			break;

		default:
			return ;
		}
	}
	void onWrite() {
		;
	}
	void onError(const errors::type& error_code) {
		;
	}

	template <size_t N>
	void decryptRSA(const BytePointer& bp, array<byte, N>& buffer) {
		;
	}
	bool verifyMD5(const Encrypt::md5_data_type md5, const BytePointer& bp) {
		return true;
	}

protected:
	void serverContent(uuid_t uid, const BytePointer&);
	void serverAddSentBytes(size_t volume);
	void serverAddReceiveBytes(size_t volume);

private:
	buffer_type	buffer_;

//	action_map_type&	action_map_;
	connection_node_ptr	connection_node_;
	Server&		server_;
};

}}

#endif /* SERVERCONNECTION_H_ */
