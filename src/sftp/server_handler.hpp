#include <libssh/libssh.h>
#include <libssh/server.h>
#define WITH_SERVER 1 /**< Compile with SFTP server support */
#include <libssh/sftp.h>

void handle_message(sftp_session, sftp_client_message);
