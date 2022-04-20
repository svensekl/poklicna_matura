#include <memory>
#include <vector>

#include <boost/json.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../events.hpp"
#include "broadcast.hpp"

using boost::asio::ip::udp;
namespace ip = boost::asio::ip;

Broadcast::Broadcast(boost::asio::io_context &io_context, Store &store)
    : socket(io_context, ip::udp::endpoint(ip::udp::v4(), 8888)),
      broadcast_endpoint(ip::address_v4::broadcast(), 8888),
      store(store) {
	socket.set_option(boost::asio::socket_base::broadcast(true));
	start_receive();
}

void Broadcast::broadcast_id() {
	namespace json = boost::json;
	std::string uuid = boost::uuids::to_string(store.me.uuid);
	json::array shares;
	for (auto &share : store.me.share_names) {
		shares.push_back(json::string(share));
	}
	json::object packet;
	packet["uuid"] = uuid;
	packet["slices"] = shares;
	socket.async_send_to(boost::asio::buffer(json::serialize(packet)),
	                     broadcast_endpoint,
	                     boost::bind(&Broadcast::handle_send, this,
	                                 boost::asio::placeholders::error));
}

void Broadcast::start_receive() {
	static udp::endpoint sender;
	static char buffer[1024];
	socket.async_receive_from(
	    boost::asio::buffer(buffer), sender,
	    boost::bind(&Broadcast::handle_receive, this, buffer, sender,
	                boost::asio::placeholders::error,
	                boost::asio::placeholders::bytes_transferred));
}

void Broadcast::handle_receive(char message[], udp::endpoint sender,
                               const boost::system::error_code &error,
                               std::size_t /*bytes_transferred*/) {
	if (error) {
		std::cerr << "Error: " << error.what() << std::endl;
		return;
	}

	namespace json = boost::json;

	json::object obj = json::parse(message).as_object();
	std::string uuid_str = obj["uuid"].as_string().c_str();
	boost::uuids::string_generator gen;
	auto uuid = gen(uuid_str);

	/* if (uuid == me.uuid) { */
	/* 	// ignore packets from self */
	/* 	return; */
	/* } */

	for (auto &d : store.devices) {
		if (d->uuid == uuid) {
			return;
		}
	}

	json::array slices = obj["slices"].as_array();
	std::vector<std::string> vslices;
	for (auto &slice : slices) {
		vslices.push_back(slice.as_string().c_str());
	}

	store.device_connect({sender.address().to_string(), vslices, uuid});

	// rebroadcast my id
	broadcast_id();

	start_receive();
}

void Broadcast::handle_send(const boost::system::error_code &error) {
	if (error) {
		std::cerr << "error sending: " << error.what() << std::endl;
	}
}
