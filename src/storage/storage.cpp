#include "storage.hpp"

#include <boost/uuid/uuid_generators.hpp>

Store::Store() {
	boost::uuids::random_generator gen;
	me_ = {
	    "127.0.0.1",
	    {},
	    gen(),
	};
}

void Store::device_connect(pc_info &&pc) {
	pc_info *info = new pc_info(pc);
	devices_.push_back(info);
	events.push_back({broadcast_pc_connected, &pc});
}
