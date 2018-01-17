#include "server.h"
#include <iostream>
#include <sstream>
#include <assert.h>


#define SERVER_SHUTDOWN 0


server_c::server_c()
{
	//init some values
	m_initialized = false;
	m_running = false;
	m_thread_map.m_capacity = MAX_CLIENTS;
	m_ptr_newthread = nullptr;
}

server_c::~server_c()
{
	stop();
}

int server_c::init(int server_port)
{
	//create our socket and grab its file desc
	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (m_sockfd == 0)
	{
		std::cerr << "socket init error" << std::endl;
		return -1;
	}

	//zero out our server descriptor
	memset(&m_server_desc, 0, sizeof(m_server_desc));

	//set descriptor settings
	m_server_desc.sin_family = AF_INET;
	m_server_desc.sin_addr.s_addr = INADDR_ANY;
	m_server_desc.sin_port = htons(server_port);

	//bind the socket to our descriptor
	if (bind(m_sockfd, (struct sockaddr *) &m_server_desc, sizeof(m_server_desc)))
	{
		std::cerr << "socket bind error" << std::endl;
		return -1;
	}

	//start listening for traffic on the socket
	if (listen(m_sockfd, LISTEN_QUEUE_SIZE))
	{
		std::cerr << "listen error" << std::endl;
		return -1;
	}

	//good to go, set init flag
	m_initialized = true;

	return 0;
}

//main server thread
void server_c::run()
{
	//don't let them call run while we're already running
	//or they didnt init
	if (!m_initialized || m_running) return;

	//set our run flag
	m_running = true;

	//start the listener thread
	m_listener_thread = std::thread(&server_c::listener, this);

	//while the server runs...
	while(m_running)
	{
		//monitor the msg_q for any commands
		//all it is used for right now is to
		//delete threads that are dynamically
		//created to handle incoming clients
		int sock;
		m_msg_q.receive((void**)&sock);

		//check for shutdown message
		if(sock == 0)
		{
			m_running = false;
			break;
		}


		m_thread_map[sock]->join();
		delete(m_thread_map[sock]);
		m_thread_map.erase(sock);
	}

	m_listener_thread.join();
}

void server_c::stop()
{
	//clear out running flag
	m_running = false;

	//close the listener thread
	shutdown(m_sockfd, SHUT_RDWR);
	close(m_sockfd);
	m_listener_thread.join();

	//cleanup our tmp thread pointer if the listener
	//thread was terminated before the new thread made
	//it into the socket map
	if (m_ptr_newthread != nullptr)
	{
		m_ptr_newthread->join();
		delete(m_ptr_newthread);
	}

	//loop through the rest of the active sockets
	//and shut them down: this unblocks calls such as recv and
	//accept with an error code, allowing the thread to cleanly exit
	for (auto active_sock : m_thread_map)
	{
		shutdown(active_sock.first, SHUT_RDWR);
		close(active_sock.first);

		//possible bug? this thread owns the listener
		//thread so it might be ok for it to join a
		//thread spawned by the listener. Seems to work for now..
		active_sock.second->join();

		delete(active_sock.second);
	}

	m_thread_map.clear();
}

bool server_c::is_running()
{
	return m_running;
}


void server_c::listener()
{
	//this is the listening thread, that accepts new clients as they
	//come in.

	struct sockaddr_in cli_adr;
	int new_sock;
	socklen_t addr_size = sizeof(cli_adr);

	//while the server is running....
	while (m_running)
	{
		//look for any new clients. this is blocking until we pull
		//the plug on the file descriptor its inspecting. This breaks
		//out of this call and the following check allows the thread to cleanly exit
		new_sock = accept(m_sockfd, (struct sockaddr*)&cli_adr, &addr_size);

		//make sure the socket is valid before doing anything with it.
		if (new_sock > 0 && m_thread_map.size() < MAX_CLIENTS)
		{
			//store the new thread object pointer into a temp variable
			m_ptr_newthread = new std::thread(&server_c::service_client, this, new_sock);

			//check to see if 'new' failed
			if (m_ptr_newthread == nullptr)
			{
				//allocate failed, something went wrong, so just break out.
				std::cerr << "thread allocate error" << std::endl;
				break;
			}

			//add this socket to the map of active sockets
			m_thread_map.insert(new_sock, m_ptr_newthread);
			m_ptr_newthread = nullptr;
		}
	}
}

//this is the actual service that interacts with a client
void server_c::service_client(int sock_fd)
{
	//while the server is actually running...
	while(m_running)
	{
		//block on recv call, wait for our client to say something.
		char header_buff[1024] = {0};
		int bytes_read = recv(sock_fd, header_buff, 1024, 0);

		//do something with the received info...
	}

	//post to the msg q saying that this thread has finished execution and is safe to delete
	assert(m_msg_q.send((void *)(intptr_t) sock_fd) == true);
	return;
}

bool server_c::user_shutdown()
{
	assert(m_msg_q.send((void *) SERVER_SHUTDOWN) == true);
	return true;
}
