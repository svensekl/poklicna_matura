#ifndef BROADCAST_H
#define BROADCAST_H

#include <QObject>
#include <QUdpSocket>

#include <boost/json.hpp>

#include "../device/device.hpp"

class BroadcastHandler : public QObject {
	Q_OBJECT

public:
	BroadcastHandler();
	void broadcast(const boost::json::object& data);

signals:

private:
	QUdpSocket sock;

private slots:
	void sockReadyRead();
};

#endif
