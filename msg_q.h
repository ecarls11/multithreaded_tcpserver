#ifndef MSG_Q_H_
#define MSG_Q_H_

#include <mutex>
#include <stdint.h>
#include <queue>


//simple wrapper around std::queue to make it thread safe, with a slight twist
//Receive is a blocking call when the queue is empty.
//this is accomplished through the use of a mutex.

class msg_q_c {
public:
	msg_q_c();
	virtual ~msg_q_c();

	bool send(void *p_msg);
	bool receive(void **pp_msg);
	bool is_empty(void);

protected:
	std::queue<void*> m_queue;
	std::mutex m_queue_mutex;
	std::mutex m_queue_notify;
	static const uint32_t m_queue_max_size = 100;
};

#endif /* MSG_Q_H_ */
