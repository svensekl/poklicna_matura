#pragma once

#include <string>
#include <iostream>

#include "pc.hpp"
#include "events.hpp"

void eval_command(std::string_view command, evqptr events, devs_ptr devices);

constexpr uint32_t hash(std::string_view data) noexcept {
	uint32_t hash = 5381;

	for (auto &c : data)
		hash = ((hash << 5) + hash) + (unsigned char)c;

	return hash;
}

void command_thread(evqptr events, devs_ptr devices) {
	std::string command;

	while (true) {
		std::cout << "sync> ";
		std::cin >> command;

		eval_command(command, events, devices);
	}
}


void eval_command(std::string_view command, evqptr events, devs_ptr devices) {
	uint32_t hashed = hash(command);

	switch (hashed) {
	case hash("search"):
		std::cout << "searching..." << std::endl;
		events->push({broadcast_send_id, NULL});
		break;
	case hash("devices"):
		std::cout << "Available devices: \n";
		for (auto &device : *devices) {
			std::cout << '\t' << device.ip << '\n';
		}
		break;
	case hash("slices"): {
		std::string ip;
		std::cin >> ip;
		std::cout << "Slices available on " << ip << ":\n";
		for (pc_info &device : *devices) {
			if (device.ip == ip) {
				for (auto &slice : *device.slice_names) {
					std::cout << '\t' << slice << '\n';
				}
				break;
			}
		}
		break;
	}
	case hash("sync"): {
		std::string ip, slice, password;
		std::cin >> ip >> slice >> password;

		std::cout << "connecting to " << slice << '@' << ip << std::endl;
		SFTP_Client client(ip, 2222, slice, password);
		break;
	}
	default:
		std::cout << "Unrecognized command" << std::endl;
	}
}
