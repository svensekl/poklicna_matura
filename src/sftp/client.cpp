#include "client.hpp"

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <errno.h>
#include <string.h>

#include <libssh/libssh.h>
#include <libssh/sftp.h>

bool authenticate_server(ssh_session session);

SFTP_Client::SFTP_Client(const std::string &address, const unsigned int port,
                         const std::string &username,
                         const std::string &password) {
	session = ssh_new();
	if (!session) {
		throw std::runtime_error("ssh_new");
	}

	ssh_options_set(session, SSH_OPTIONS_HOST, address.c_str());
	ssh_options_set(session, SSH_OPTIONS_PORT, &port);

	if (ssh_connect(session) != SSH_OK) {
		throw std::runtime_error("ssh_connect: " +
		                         std::string(ssh_get_error(session)));
	}

	if (!authenticate_server(session)) {
		throw std::runtime_error("failed to authenticate server");
	}

	if (ssh_userauth_password(session, username.c_str(), password.c_str()) !=
	    SSH_AUTH_SUCCESS) {
		throw std::runtime_error("failed to authenticate user");
	}

	if (!(sftp = sftp_new(session))) {
		throw std::runtime_error("sftp_new error: " +
		                         std::string(ssh_get_error(session)));
	}

	if (sftp_init(sftp) != SSH_OK) {
		throw std::runtime_error("sftp_init error");
	}
}

SFTP_Client::~SFTP_Client() {
	if (sftp) {
		sftp_free(sftp);
	}
	ssh_disconnect(session);
	ssh_free(session);
}

bool authenticate_server(ssh_session session) {
	bool authenticated = false;
	unsigned char *hash = NULL;
	size_t hlen;
	ssh_key pubkey = NULL;
	if (ssh_get_server_publickey(session, &pubkey) < 0) {
		return (false);
	}

	int rc =
	    ssh_get_publickey_hash(pubkey, SSH_PUBLICKEY_HASH_SHA1, &hash, &hlen);
	ssh_key_free(pubkey);
	if (rc < 0) {
		return (false);
	}

	switch (ssh_session_is_known_server(session)) {
	case SSH_KNOWN_HOSTS_ERROR:
	case SSH_KNOWN_HOSTS_CHANGED:
	case SSH_KNOWN_HOSTS_OTHER:
		// for security reasons, they are untrusted
		break;
	case SSH_KNOWN_HOSTS_NOT_FOUND:
	case SSH_KNOWN_HOSTS_UNKNOWN:
		// server is unknown. query user if trusted. for rn, it trusts auto
		if (ssh_session_update_known_hosts(session) < 0) {
			break;
		}
		[[fallthrough]];
	case SSH_KNOWN_HOSTS_OK:
		authenticated = true;
		break;
	}
	ssh_clean_pubkey_hash(&hash);
	return (authenticated);
}

bool SFTP_Client::get_file(const std::string &from, const std::string &to) {
	sftp_file file;
	const int chunk_size = 1024;
	char buffer[chunk_size];
	int nbytes;

	std::string dest_file = to + "/" + basename(from.c_str());

	std::cout << "getting file" << std::endl;

	file = sftp_open(sftp, from.c_str(), O_RDONLY, 0);
	if (file == NULL) {
		std::cerr << "Can't open file for reading: " << ssh_get_error(session)
		          << std::endl;
		return false;
	}
	std::cout << "reading" << std::endl;

	int fd = open(dest_file.c_str(), O_CREAT);
	if (fd == -1) {
		std::cerr << "failed opening/creating new file: " << dest_file << " due to "
		          << strerror(errno) << std::endl;
		sftp_close(file);
		return false;
	}

	for (;;) {
		nbytes = sftp_read(file, buffer, chunk_size);
		if (nbytes < 0) {
			std::cerr << "Error while reading file: " << ssh_get_error(session)
			          << std::endl;
			sftp_close(file);
			close(fd);
			return false;
		} else {
			write(fd, buffer, nbytes);
			if (nbytes < chunk_size) {
				// got to eof
				break;
			}
		}
	}

	close(fd);
	if (sftp_close(file) != SSH_OK) {
		std::cerr << "Can't close the read file: " << ssh_get_error(session)
		          << std::endl;
		return false;
	}

	return true;
}
