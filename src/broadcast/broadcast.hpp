#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

#include "../storage/storage.hpp"

using boost::asio::ip::udp;

namespace ip = boost::asio::ip;

class Broadcast {
public:
	Broadcast(boost::asio::io_context& io_context, Store &store);

	void broadcast_id();

private:
	void start_receive();

	void handle_receive(char message[],
										 udp::endpoint sender,
										 const boost::system::error_code& error,
										 std::size_t bytes_transferred);

	void handle_send(const boost::system::error_code &error);

	udp::socket socket;
  ip::udp::endpoint broadcast_endpoint;
	Store &store;
};
