#pragma once

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <boost/uuid/uuid.hpp>

enum event_type {
	broadcast_pc_connected,
	broadcast_send_id,
	connect_share,
	create_share,
};

struct event {
	event_type type;
	void *data;
};

typedef std::string share;

struct pc_info {
	std::string ip;
	std::vector<share> share_names;
	boost::uuids::uuid uuid;
};

struct share_link {
	pc_info &device;
	share data;
	std::string directory;
};

class Store {
public:
	Store();
	/* ~Store(); */

	const std::vector<pc_info *> &devices = devices_;
	pc_info &me = me_;
	std::deque<event> events;
	std::vector<share_link> linked_shares;

	void device_connect(pc_info &&pc);

private:
	std::vector<pc_info *> devices_;
	pc_info me_;
};
