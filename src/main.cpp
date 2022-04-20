#include <QApplication>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>
#include <QMessageBox>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "broadcast/broadcast.hpp"
#include "events.hpp"
#include "sftp/client.hpp"
#include "sftp/getfile.hpp"
#include "sftp/server.hpp"
#include "storage/storage.hpp"

#include "windows/app/app.hpp"

int main(int argc, char *argv[]) {
	std::unique_ptr<Store> store(new Store);
	std::future<bool> got_file;

	// gui
	QApplication gui(argc, argv);
	MainWindow win(*store);
	win.show();

	// broadcast
	boost::asio::io_context net_io;
	Broadcast broadcast(net_io, *store);

	// sftp server
	SFTP_Server sftp_server(2222);

	while (true) {
		gui.processEvents();
		net_io.poll();
		sftp_server.poll_accept();

		if (got_file.valid() && got_file.wait_for(std::chrono::minutes(0)) ==
		                            std::future_status::ready) {
			QMessageBox show;
			if (got_file.get()) {
				show.setText("Got File.");
			} else {
				show.setText("File download failed.");
			}
			show.exec();
		}

		// process events
		for (auto &e : store->events) {
			switch (e.type) {
			case broadcast_send_id:
				broadcast.broadcast_id();
				break;
			case broadcast_pc_connected:
				win.device_connected();
				break;
			case connect_share: {
				std::shared_ptr<share_link> link((share_link *)(e.data));
				std::cout << "connected " << link->directory << std::endl;
				store->linked_shares.push_back(*link);
				got_file = std::async(get_file, link);
				win.share_linked();
				break;
			}
			case create_share: {
					std::string *filename = (std::string *) (e.data);
					store->me.share_names.push_back(*filename);
					delete filename;
					win.my_shares_updated();
				}
			}
		}
		store->events.clear();
	}

	return (0);
}
