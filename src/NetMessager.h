/*
 * NetMessager.h
 *
 *  Created on: 2013-7-23
 *      Author: Administrator
 */

#ifndef NETMESSAGER_H_
#define NETMESSAGER_H_

#include <common.h>
#include <Serialize.h>
#include <Connection.h>

/**
#ifndef NET_MESSAGE_SIZE
#  define NET_MESSAGE_SIZE SERVER_IO_PACKAGE_SIZE
#endif


#ifndef NET_MESSAGE_MAX_TARGET_SIZE
#  ifdef MESSAGE_MAX_TARGET_SIZE
#    define NET_MESSAGE_MAX_TARGET_SIZE MESSAGE_MAX_TARGET_SIZE
#  else
#    error NET_MESSAGE_MAX_TARGET_SIZE is not defined
#  endif
#endif
*/

namespace zertcore{ namespace utils{

using namespace zertcore::net;

typedef uuid_t	target_type;

namespace messages {

enum {
	COMMAND_NONE							= 0,
	COMMAND_LOGIN							= 1,
	COMMAND_MESSAGE							= 10,
};

}

template <class StreamList = WriterOnlyStreamList>
class NetMessager;

template <class StreamList = WriterOnlyStreamList>
class NetMessageConnection :
		public Connection<NetMessageConnection<StreamList> >
{
private:
	static const uint32	cPACKAGE_SENTINEL;

public:
	typedef NetMessageConnection*			ptr;

public:
	struct PackageHeader
	{
		uint	command;
		uint	priority;
		uint32	index_size;
		uint32	data_size;
		byte	data[0];
/**
		target_type	target_indexes[0];
		byte*	data;
*/
	};

public:
	explicit NetMessageConnection(NetMessager<StreamList>& messager, asio::io_service& ios) :
		Connection<NetMessageConnection<StreamList> >(ios), messager_(messager) {

		registerReadHandler(bind(&NetMessageConnection::onRead, this, _1));
		registerConnectHandler(bind(&NetMessageConnection::onConnect, this));
		registerWriteHandler(bind(&NetMessageConnection::onWrite, this));
		registerErrorHandler(bind(&NetMessageConnection::onError, this, _1));
	}

public:
	bool send(const uint command, const BytePointer& bp,
			const BufferPointer<target_type>& targets = BufferPointer<target_type>(),
			const uint priority = 0) {
		size_t package_size = sizeof(PackageHeader);
		package_size += bp.size + sizeof(target_type) * targets.size + sizeof(cPACKAGE_SENTINEL);

		PackageHeader pkg = {command, priority, targets.size, bp.size};

		this->template beginFormatSend(package_size);
		*this << pkg
			  << targets
			  << cPACKAGE_SENTINEL
			  << bp;

		this->template endFormatSend();

		return true;
	}

public:
	void onConnect() {
		;
	}
	void onRead(const BytePointer& bp);
	void onWrite() {
		;
	}
	void onError(const ::zertcore::net::errors::type& error_code) {
		;
	}

private:
	NetMessager<StreamList>&	messager_;
};

template <class StreamList>
const uint32 NetMessageConnection<StreamList>::cPACKAGE_SENTINEL		= 0xdeadbeaf;

/**
 *
 */
template <class StreamList>
class NetMessager :
		public ConnectionOperator<NetMessager<StreamList>, NetMessageConnection<StreamList> >
{
public:
	typedef NetMessageConnection<StreamList>	connection;
	typedef typename connection::ptr			connection_ptr;

public:
	explicit NetMessager(StreamList& stream_list, io_service& ios) :
		ConnectionOperator<NetMessager<StreamList>, NetMessageConnection<StreamList> >(ios),
		stream_list_(stream_list) {
		;
	}
	explicit NetMessager(io_service& ios) :
		ConnectionOperator<NetMessager<StreamList>, NetMessageConnection<StreamList> >(ios),
		stream_list_(WriteronlyStreamlist) {
		;
	}

public:
	connection_ptr login(const string& host, int port, const string& essential) {
		connection_ptr client_conn = this->connect(host, port);
		client_conn->send(messages::COMMAND_LOGIN, BytePointer((const byte *)essential.c_str(), essential.size()));

		return client_conn;
	}
	void dispatch(const uint command, const BytePointer& bp, const BufferPointer<target_type>& targets) {
		stream_list_.template dispatch(command, bp, targets);
	}

private:
	StreamList& stream_list_;
};

template <class StreamList>
void NetMessageConnection<StreamList>::onRead(const BytePointer& bp) {
	const PackageHeader* pkg = (const PackageHeader *)(bp.data);

	/**
	 * check sentinel
	 */
	const uint32* sentinel = (uint32 *)(pkg->data + pkg->index_size * sizeof(target_type));
	if (sentinel && *sentinel == cPACKAGE_SENTINEL) {
		const byte* data = pkg->data + pkg->index_size * sizeof(target_type) + sizeof(cPACKAGE_SENTINEL);
		const target_type* targets = (const target_type*)pkg->data;

		messager_.template dispatch(pkg->command, BytePointer(data, pkg->data_size),
				BufferPointer<target_type>(targets, pkg->index_size));
	}
	else {
		printf("Sentinel Failed: %x\n", *sentinel);
	}
}

}}


#endif /* NETMESSAGER_H_ */
