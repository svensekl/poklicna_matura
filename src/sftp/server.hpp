#include <iostream>
#include <memory>
#include <thread>

#include <libssh/libssh.h>
#include <libssh/server.h>
#define WITH_SERVER 1 /**< Compile with SFTP server support */
#include <libssh/sftp.h>

class SFTP_Server {
public:
	SFTP_Server(const unsigned int port);

	~SFTP_Server();

	// this function should be in a loop
	void poll_accept();

private:
	ssh_bind sshbind = NULL;

	static void session_handler(ssh_session session);
};
