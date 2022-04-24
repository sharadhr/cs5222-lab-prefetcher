/*
* File:   lrucache.hpp
* Author: Alexander Ponomarev
* Source: https://github.com/lamerman/cpp-lru-cache/blob/master/include/lrucache.hpp
* Created on June 20, 2013, 5:09 PM
*
* Modified to remove namespaces, and use std::optional by
* Sharadh Rajaraman (c) 2022
*/

#pragma once

#include <cstddef>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>

// a cache which evicts the least recently used item when it is full
template<typename key_t, typename value_t>
class lru_cache
{
public:
	typedef typename std::pair<key_t, value_t> key_value_pair_t;
	typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

	explicit lru_cache(size_t max_size) : _max_size(max_size) {}

	void put(const key_t& key, const value_t& value)
	{
		auto it = _cache_items_map.find(key);
		_cache_items_list.push_front(key_value_pair_t(key, value));
		if (it != _cache_items_map.end()) {
			_cache_items_list.erase(it->second);
			_cache_items_map.erase(it);
		}
		_cache_items_map[key] = _cache_items_list.begin();

		if (_cache_items_map.size() > _max_size) {
			auto last = _cache_items_list.end();
			last--;
			_cache_items_map.erase(last->first);
			_cache_items_list.pop_back();
		}
	}

	std::optional<value_t> get(const key_t& key)
	{
		auto it = _cache_items_map.find(key);
		if (it == _cache_items_map.end()) {
			return {};
		} else {
			_cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
			return it->second->second;
		}
	}

	bool exists(const key_t& key) const { return _cache_items_map.find(key) != _cache_items_map.end(); }

	[[nodiscard]] std::size_t size() const { return _cache_items_map.size(); }

private:
	std::list<key_value_pair_t> _cache_items_list;
	std::unordered_map<key_t, list_iterator_t> _cache_items_map;
	size_t _max_size;
};
