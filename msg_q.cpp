#include "msg_q.h"

msg_q_c::msg_q_c()
{
	m_queue_notify.lock();
}

msg_q_c::~msg_q_c()
{
}

bool msg_q_c::send(void* p_msg)
{
	bool result = false;
	m_queue_mutex.lock();
	if (m_queue.size() < m_queue_max_size)
	{
		m_queue.push(p_msg);
		//let the receive function know that
		//there's something in the thread, stop blocking.
		m_queue_notify.unlock();
		result = true;
	}
	m_queue_mutex.unlock();
	return result;
}

bool msg_q_c::receive(void** pp_msg)
{
	m_queue_notify.lock();

	m_queue_mutex.lock();
	if (!m_queue.empty())
	{
		*pp_msg = m_queue.front();
		m_queue.pop();
		m_queue_mutex.unlock();
		return true;
	}
	m_queue_mutex.unlock();
	return false;
}

bool msg_q_c::is_empty(void)
{
	bool is_empty = m_queue.empty();
	return is_empty;
}
