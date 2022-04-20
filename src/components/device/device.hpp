#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <QObject>
#include <boost/uuid/uuid.hpp>
#include "../share/share.hpp"
#include <vector>

class Device : QObject {
	Q_OBJECT

public:
	Device() = delete;
	Device(boost::uuids::uuid uuid);

private:
	boost::uuids::uuid uuid;
	std::vector<Share> shares;
};

#endif
