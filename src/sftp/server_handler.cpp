#include "server_handler.hpp"

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string.h>

typedef void (*message_handler)(sftp_session, sftp_client_message);

std::map<std::string, int> file_handles;

void open_handler(sftp_session, sftp_client_message);
void close_handler(sftp_session, sftp_client_message);
void read_handler(sftp_session, sftp_client_message);
void cwd_handler(sftp_session, sftp_client_message);

const std::map<int, message_handler> sftp_handler = {
    {SSH_FXP_OPEN, open_handler},
    {SSH_FXP_CLOSE, close_handler},
    {SSH_FXP_READ, read_handler},
    {SSH_FXP_REALPATH, cwd_handler},
};

void handle_message(sftp_session session, sftp_client_message message) {
	message_handler handler;
	try {
		handler = sftp_handler.at(message->type);
	} catch (std::out_of_range) {
		std::cerr << "unsupported: " << message << std::endl;
		sftp_reply_status(message, SSH_FX_OP_UNSUPPORTED,
		                  "Operation not supported");
		return;
	}

	handler(session, message);
}

void open_handler(sftp_session session, sftp_client_message message) {
	std::cout << "open" << std::endl;
	std::string file_name = sftp_client_message_get_filename(message);

	struct sftp_client_message_struct client_message_data = *message;
	int message_flags = client_message_data.flags;
	int flags = 0;
	if (((message_flags & SSH_FXF_READ) == SSH_FXF_READ) &&
	    ((message_flags & SSH_FXF_WRITE) == SSH_FXF_WRITE)) {
		flags = O_RDWR;
	} else if ((message_flags & SSH_FXF_READ) == SSH_FXF_READ) {
		flags = O_RDONLY;
	} else if ((message_flags & SSH_FXF_WRITE) == SSH_FXF_WRITE) {
		flags = O_WRONLY;
	}

	if ((message_flags & SSH_FXF_APPEND) == SSH_FXF_APPEND) {
		flags |= O_APPEND;
	}

	if ((message_flags & SSH_FXF_CREAT) == SSH_FXF_CREAT) {
		flags |= O_CREAT;
	}

	if ((message_flags & SSH_FXF_TRUNC) == SSH_FXF_TRUNC) {
		flags |= O_TRUNC;
	}

	if ((message_flags & SSH_FXF_EXCL) == SSH_FXF_EXCL) {
		flags |= O_EXCL;
	}
	int fd = open(file_name.c_str(), flags);
	if (fd == -1) {
		std::cout << "error opening file: " << file_name << ": " << strerror(errno)
		          << std::endl;

		if (errno == EACCES) {
			sftp_reply_status(message, SSH_FX_PERMISSION_DENIED,
			                  "The requested access to the file is not allowed.");
		} else if (errno == ENOENT) {
			sftp_reply_status(message, SSH_FX_NO_SUCH_FILE,
			                  "The requested file does not exist.");
		} else {
			sftp_reply_status(message, SSH_FX_FAILURE,
			                  "Could not access requested file.");
		}
		return;
	}
	std::cout << "opened file " << file_name << std::endl;

	file_handles[file_name] = fd;
	char *info = new char[file_name.size() + 1];
	strcpy(info, file_name.c_str());
	ssh_string handle = sftp_handle_alloc(session, info);
	sftp_reply_handle(message, handle);
}

void close_handler(sftp_session session, sftp_client_message message) {
	std::string file_name = (char *)sftp_handle(session, message->handle);

	if (file_handles.count(file_name)) {
		int fd = file_handles[file_name];
		::close(fd);
		file_handles.erase(file_name);
		sftp_reply_status(message, SSH_FX_OK, "Success");
	} else {
		std::cerr << "File not opened: " << file_name << std::endl;
		sftp_reply_status(message, SSH_FX_FAILURE, "File not opened.");
	}
}

void read_handler(sftp_session session, sftp_client_message message) {
	std::string file_name = (char *)sftp_handle(session, message->handle);

	if (file_handles.count(file_name)) {
		int fd = file_handles[file_name];
		lseek(fd, message->offset, SEEK_SET);
		char *buffer = new char[message->len];
		int n = read(fd, buffer, message->len);
		if (n > 0) {
			sftp_reply_data(message, buffer, n);
		} else {
			sftp_reply_status(message, SSH_FX_EOF, "End-of-file encountered");
		}
		delete[] buffer;
	} else {
		std::cerr << "file not opened: " << file_name;
		sftp_reply_status(message, SSH_FX_FAILURE, "File not opened");
	}
}

void cwd_handler(sftp_session, sftp_client_message message) {
	std::cout << "handling realpath" << std::endl;
	std::string long_file_name;
	const char *file_name = sftp_client_message_get_filename(message);

	struct sftp_attributes_struct attr;
	sftp_reply_names_add(message, file_name, file_name, &attr);
	sftp_reply_names(message);
}
