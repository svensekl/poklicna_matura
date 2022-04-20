#include "./broadcast.hpp"
#include "boost/json.hpp"
using namespace boost::json;

#include <iostream>
#include <string>

BroadcastHandler::BroadcastHandler() {
	sock.bind(); // bind to random port

	connect(&sock, &QUdpSocket::readyRead, this, &BroadcastHandler::sockReadyRead);
}

void BroadcastHandler::broadcast(const object& data) {
	QString packet = QString::fromStdString(serialize(data));
	sock.write(packet.toUtf8());
}

void BroadcastHandler::sockReadyRead() {
	QByteArray data;
	while (sock.hasPendingDatagrams()) {
		data.resize(int(sock.pendingDatagramSize()));
		sock.readDatagram(data.data(), data.size());
		std::cout << "Read data: " << data.constData() << std::endl;
	}
}
