#ifndef PACKET_HPP
#define PACKET_HPP

#include <string>
#include <boost/json.hpp>

using namespace boost::json;

class Packet {
public:
	Packet(int port);
	Packet(std::string serializedData);

	std::string serialize(const Packet &p);

private:
	object data;
	
};

#endif
