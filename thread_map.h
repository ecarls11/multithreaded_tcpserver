/*
 * map.h
 *
 *  Created on: Dec 31, 2017
 *      Author: eric
 */

#ifndef THREAD_MAP_H_
#define THREAD_MAP_H_

#include <map>
#include <mutex>

//a class wrapper around the std::map to make writes
//thread safe. This boils down to overriding insert and erase

template <class K, class D>
class thread_map_c : public std::map<K, D> {

	public:
		inline thread_map_c() { m_capacity = 10; };
		inline virtual ~thread_map_c() { };

		void insert(const K key, const D data)
		{
			m_mutex.lock();
			if (!contains(key))
			{
				if (this->size() >= m_capacity)
				{
					m_mutex.unlock();
					return;
				}
			}

			this->operator[](key) = data;
			m_mutex.unlock();
			return;
		};

		void erase(const K key)
		{
			m_mutex.lock();
			if (contains(key))
			{
				this->erase(key);
			}
			m_mutex.unlock();
		};

		bool contains(K key)
		{
			if (this->find(key) == this->end())
			{
				return false;
			}
			return true;
		};

		size_t m_capacity;

private:
	std::mutex m_mutex;
};



#endif /* THREAD_MAP_H_ */
