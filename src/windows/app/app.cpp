#include <QFileDialog>
#include <iostream>

#include "app.hpp"

MainWindow::MainWindow(Store &store) : store(store) {
	setupUi(this);

	connect(btnSearch, &QPushButton::clicked, this, &MainWindow::btn_search_click);
	connect(btnConnect, &QPushButton::clicked, this, &MainWindow::btn_connect_click);
	connect(btnCreateShare, &QPushButton::clicked, this, &MainWindow::btn_create_share_click);
}

void MainWindow::btn_search_click() {
	store.events.push_back({broadcast_send_id, NULL});
}

void MainWindow::btn_connect_click() {
	auto selected = lstShares->selectedItems();

	if (selected.length() != 1) {
		return;
	}

	QString dir = QFileDialog::getExistingDirectory(this);

	pc_info *device;
	share share;
	for (auto &d : store.devices) {
		for (auto &s : d->share_names) {
			if (selected.first()->text().toStdString() == s) {
				device = d;
				share = s;
				goto found;
			}
		}
	}
	return;

found:
	auto link = new share_link({*device, share, dir.toStdString()});
	std::cout << "connecting share " << dir.toStdString() << std::endl;
	store.events.push_back({connect_share, link});
}

void MainWindow::share_linked() {
	lstLinked->clear();

	for (auto &link : store.linked_shares) {
		lstLinked->addItem(QString::fromStdString(link.data + " -> " + link.directory));
	}
}

void MainWindow::device_connected() {
	lstShares->clear();
	for (auto &d : store.devices) {
		for (auto &share : d->share_names) {
			lstShares->addItem(QString::fromStdString(share));
		}
	}
}

void MainWindow::btn_create_share_click() {
	std::string *filename = new std::string(QFileDialog::getOpenFileName().toStdString());

	store.events.push_back({create_share, filename});
}

void MainWindow::my_shares_updated() {
	lstMyShares->clear();
	for (auto &share : store.me.share_names) {
		lstMyShares->addItem(QString::fromStdString(share));
	}
}
