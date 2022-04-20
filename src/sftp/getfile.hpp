#pragma once

#include "storage/storage.hpp"
#include "client.hpp"
#include <future>

bool get_file(std::shared_ptr<share_link> link) {
	SFTP_Client client(link->device.ip, 2222, link->data, "password");
	return client.get_file(link->data, link->directory);
}

