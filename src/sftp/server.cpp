#include "server.hpp"
#include "server_handler.hpp"

#include <future>

bool authenticate(ssh_session session);
ssh_channel open_channel(ssh_session session);
bool sftp_subsystem_request(ssh_session session);
void sftp_command_loop(sftp_session sftp);

SFTP_Server::SFTP_Server(const unsigned int port) {
	/* int verbosity = SSH_LOG_PROTOCOL; */

	sshbind = ssh_bind_new();

	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY,
	                     "/home/a/Projects/sync/keys/ssh_host_rsa_key");
	ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &port);
	/* ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_LOG_VERBOSITY, &verbosity);
	 */

	if (ssh_bind_listen(sshbind) < 0) {
		throw std::runtime_error("ssh_bind_listen: " +
		                         std::string(ssh_get_error(sshbind)));
	}
	ssh_bind_set_blocking(sshbind, 0);
}

SFTP_Server::~SFTP_Server() {
	ssh_bind_free(sshbind);
	ssh_finalize();
}

void SFTP_Server::poll_accept() {
	static bool polling = false;
	static std::future<int> fut;
	static ssh_session session;

	if (polling) {
		if (fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			if (fut.get() != SSH_OK) {
				std::cerr << "Error ssh_bind_accept " << ssh_get_error(sshbind)
				          << std::endl;
			}
			std::thread(session_handler, session).detach();
			polling = false;
		} else {
			return;
		}
	}

	session = ssh_new();

	if (!session) {
		std::cerr << "Error ssh_new" << std::endl;
		return;
	}

	if (!sshbind) {
		std::cerr << "Ssh bind error" << std::endl;
		return;
	}

	fut = std::async(ssh_bind_accept, sshbind, session);
	polling = true;
}

void SFTP_Server::session_handler(ssh_session session) {
	sftp_session sftp = NULL;
	ssh_channel channel;

	if (ssh_handle_key_exchange(session) != SSH_OK) {
		std::cerr << "ssh_handle_key_exchange: " << ssh_get_error(session)
		          << std::endl;
		goto cleanup;
	}

	if (!authenticate(session)) {
		std::cerr << "Error authenticate: " << ssh_get_error(session) << std::endl;
		goto cleanup;
	}

	if (!(channel = open_channel(session))) {
		std::cerr << "Error opening channel: " << ssh_get_error(session)
		          << std::endl;
		goto cleanup;
	}

	if (!sftp_subsystem_request(session)) {
		std::cerr << "Error sftp subsystem request only: "
		          << ssh_get_error(session);
		goto cleanup;
	}

	if (!(sftp = sftp_server_new(session, channel))) {
		goto cleanup;
	}

	if (sftp_server_init(sftp) < 0) {
		goto cleanup;
	}

	sftp_command_loop(sftp);

cleanup:
	if (sftp != NULL) {
		sftp_free(sftp);
	}

	ssh_disconnect(session);
	ssh_free(session);
}

bool authenticate(ssh_session session) {
	ssh_message message;
	bool authenticated = false;

	while (!authenticated) {
		// wait for message
		if (!(message = ssh_message_get(session))) {
			return false;
		}

		// ignore if not authentication message
		if (ssh_message_type(message) != SSH_REQUEST_AUTH) {
			ssh_message_reply_default(message);
			ssh_message_free(message);
			continue;
		}

		// only supported authentication method is with password
		if (ssh_message_subtype(message) != SSH_AUTH_METHOD_PASSWORD) {
			ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PASSWORD);
			ssh_message_reply_default(message);
			ssh_message_free(message);
			continue;
		}

		/* std::string username = ssh_message_auth_user(message); */
		/* std::string password = ssh_message_auth_password(message); */

		std::cout << "logging in: " << ssh_message_auth_user(message)
		          << " with password: " << ssh_message_auth_password(message)
		          << std::endl;

		// authenticate user
		/* if (username == password) { */
		/* 	ssh_message_auth_reply_success(message, 0); */
		/* } else { */
		/* 	ssh_message_reply_default(message); */
		/* } */
		ssh_message_auth_reply_success(message, 0);

		ssh_message_free(message);
		authenticated = true;
	}
	return authenticated;
}

ssh_channel open_channel(ssh_session session) {
	ssh_channel channel = NULL;
	ssh_message message;

	while ((message = ssh_message_get(session))) {
		if (ssh_message_type(message) != SSH_REQUEST_CHANNEL_OPEN ||
		    ssh_message_subtype(message) != SSH_CHANNEL_SESSION) {
			ssh_message_reply_default(message);
			ssh_message_free(message);
			continue;
		}

		channel = ssh_message_channel_request_open_reply_accept(message);
		ssh_message_free(message);
		break;
	}

	return (channel);
}

bool sftp_subsystem_request(ssh_session session) {
	ssh_message message = NULL;

	while ((message = ssh_message_get(session))) {
		if (ssh_message_type(message) == SSH_REQUEST_CHANNEL &&
		    ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_SUBSYSTEM) {
			std::string subsystem = ssh_message_channel_request_subsystem(message);
			if (subsystem == "sftp") {
				ssh_message_channel_request_reply_success(message);
				ssh_message_free(message);
				return true;
			}
		}
		ssh_message_reply_default(message);
		ssh_message_free(message);
		continue;
	}
	return false;
}

void sftp_command_loop(sftp_session sftp) {
	sftp_client_message message;

	while ((message = sftp_get_client_message(sftp))) {
		handle_message(sftp, message);
		sftp_client_message_free(message);
	}
}
