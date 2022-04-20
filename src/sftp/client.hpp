#pragma once

#include <iostream>

#include <libssh/libssh.h>
#include <libssh/sftp.h>

class SFTP_Client {
public:
	SFTP_Client(const std::string &address, const unsigned int port,
	            const std::string &username, const std::string &password);

	~SFTP_Client();

	bool get_file(const std::string &from, const std::string &to);

private:
	ssh_session session;
	sftp_session sftp;
};
