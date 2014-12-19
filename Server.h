/*
 * Server.h
 *
 *  Created on: 2013-7-22
 *      Author: Administrator
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <common.h>
#include <Connection.h>
#include <ActionMap.h>
#include <MultiEntranceMap.h>
#include <NetMessager.h>
#include <ServerConnection.h>

namespace zertcore{ namespace net{

/**
 * use Net Messager as its pipe
 */
namespace utils = zertcore::utils;
typedef utils::NetMessager<WriterOnlyStreamList>::connection messager_type;

/**
 * ServerConnectionManager
 */
class ServerConnectionManager
{
public:
	typedef ServerConnection::connection_map_type	connection_map_type;
	typedef ServerConnection::action_map_type		action_map_type;
	typedef action_map_type::connection				connection;

public:
	ServerConnection::ptr alloc(Server&, io_service&);
	ServerConnection::ptr get(uuid_t uid) {
		connection_map_type::node_ptr node = connection_map_.find(uid);
		return node? node->data(): Null;
	}

public:
	connection addCommand(const action_map_type::key_type command,
			ServerConnection::action_handler_type handler) {
		return action_map_.addHandler(command, handler);
	}
	void removeCommand(connection& conn) {
		conn.disconnect();
	}

private:
	connection_map_type		connection_map_;
	action_map_type			action_map_;
};

/**
 * Server
 */
class Server:
		public ConnectionOperator<Server, ServerConnection, ServerConnectionManager>
{
	friend class ServerConnection;
public:
	explicit Server(io_service& ios, ServerConnectionManager& manager, messager_type& messager) :
		ConnectionOperator<Server, ServerConnection, ServerConnectionManager>(ios, manager),
		messager_(messager) {
		;
	}

public:
	void newMessage(uuid_t uid, const BytePointer& bp) {
		messager_.send(utils::messages::COMMAND_MESSAGE, bp, BufferPointer<uuid_t>(&uid, 1));
	}

private:
	size_t	sent_bytes_;
	size_t	recv_bytes_;
	messager_type&	messager_;
};

inline ServerConnection::ptr ServerConnectionManager::alloc(Server& server, io_service& ios) {
	ServerConnection::ptr conn = load<ServerConnection, Server, ServerConnectionManager,
					asio::io_service> (server, *this, ios);
	conn->connection_node_ = connection_map_.insert(conn);
	return conn;
}

void ServerConnection::serverContent(uuid_t uid, const BytePointer& bp) {
	server_.newMessage(uid, bp);
}
void ServerConnection::serverAddSentBytes(size_t volume) {
	server_.sent_bytes_ += volume;
}
void ServerConnection::serverAddReceiveBytes(size_t volume) {
	server_.recv_bytes_ += volume;
}

}}


#endif /* SERVER_H_ */
