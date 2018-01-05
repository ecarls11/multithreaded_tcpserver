#ifndef _SERVER_H
#define _SERVER_H

#include <thread>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "thread_map.h"
#include "msg_q.h"

#define DEFAULT_PORT 4000
#define LISTEN_QUEUE_SIZE 5
#define MAX_CLIENTS 10


class server_c
{
	public:
		server_c();
		virtual ~server_c();
		int init(int server_port=DEFAULT_PORT);
		bool is_running();
		void run();
		void stop();
		bool user_shutdown();

	private:
		void listener();
		void service_client(int sock_fd);
		msg_q_c m_msg_q;
		thread_map_c<int, std::thread*> m_thread_map;
		std::thread* m_ptr_newthread;
		std::thread m_listener_thread;
		bool m_initialized;
		bool m_running;
		int m_sockfd;
		int port;
		struct sockaddr_in m_server_desc;
};



#endif
