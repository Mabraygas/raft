#ifndef RAFT_LOCKFREE_MAP_H
#define RAFT_LOCKFREE_MAP_H
/**
 * @file    raft_lockfree_map.h
 * @brief   支持多线程操作的stl map
 * @author  maguangxu
 * @version 1.0
 * @date    2015-09-08
 */
#include <map>

#include <eagle_lock.h>

namespace RAFT
{

using namespace std;
using namespace eagle;

template<typename K, typename V, typename D = std::map<K, V> >
class mthread_map : public ThreadLock
{
public:
	typedef D map_type;

public:
	
	mthread_map() { }
	
	void update(const K& key, const V& val);

	void copy(map_type& des);

	void clear();

	size_t size();
	
private:
	map_type _map;
};

template<typename K, typename V, typename D>
void mthread_map<K, V, D>::update(const K& key, const V& val) {
	Lock lock(*this);
	
	_map[key] = val;
}

template<typename K, typename V, typename D>
void mthread_map<K, V, D>::copy(map_type& des) {
	Lock lock(*this);
	
	des = _map;
}

template<typename K, typename V, typename D>
void mthread_map<K, V, D>::clear() {
	Lock lock(*this);

	_map.clear();
}

template<typename K, typename V, typename D>
size_t mthread_map<K, V, D>::size() {
	Lock lock(*this);
	
	return _map.size();
}
	
}


#endif //RAFT_LOCKFREE_MAP_H

// vim: ts=4 sw=4 nu

