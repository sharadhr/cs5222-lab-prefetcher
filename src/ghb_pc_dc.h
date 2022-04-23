#pragma once

#include "history_buffer.h"
#include "lru_cache.h"

template<std::unsigned_integral IndexKey, typename MissAddress>
using IndexTable = lru_cache<IndexKey, node_wk_ptr_t<MissAddress>>;

using Address = unsigned long long int;

template<std::unsigned_integral IndexKey, std::unsigned_integral MissAddress>
class GHBPCDCPrefetcher
{
public:
	GHBPCDCPrefetcher(std::size_t entries, std::size_t prefetch_degree) noexcept :
		indexTable{entries}, historyBuffer{entries, prefetch_degree}
	{}

	std::vector<MissAddress> missOperate(IndexKey index_key, MissAddress miss_address);

private:
	IndexTable<IndexKey, MissAddress> indexTable{256u};
	HistoryBuffer<MissAddress> historyBuffer{256u};
};

template<std::unsigned_integral IndexKey, std::unsigned_integral MissAddress>
std::vector<MissAddress> GHBPCDCPrefetcher<IndexKey, MissAddress>::missOperate(IndexKey index_key,
                                                                               MissAddress miss_address)
{
	// is there an index table (IT) hit?
	auto possible_buffer_ptr{indexTable.get(index_key)};

	// no IT hit, or pointer expired: do no prefetching, but insert into the head of the buffer.
	if (!possible_buffer_ptr || possible_buffer_ptr->expired()) {
		auto new_buffer_ptr{historyBuffer.pushNewAddress(miss_address)};
		// If no IT hit, insert
		if (!possible_buffer_ptr) indexTable.insert(index_key, new_buffer_ptr);
		// hit, but expired:
		else if (possible_buffer_ptr->expired()) possible_buffer_ptr.value() = new_buffer_ptr;

		// Return empty vector--no prefetching
		return {};
	}

	// Pointer is *valid* here, insert new address at head. Pass in old pointer to link previous
	// linked list head
	auto linked_list_head{historyBuffer.linkedListHead(miss_address, possible_buffer_ptr.value())};
	// Now, get first delta pair in the linked list
	auto possible_first_delta_pair{historyBuffer.firstDeltaPair(linked_list_head)};

	// Finally, return up to prefetch_degree prefetch targets if there's a first delta pair at all
	if (!possible_first_delta_pair) return {};

	auto prefetch_deltas{historyBuffer.prefetchDeltas(linked_list_head, possible_first_delta_pair.value())};
	std::vector<MissAddress> prefetch_addresses{};
	std::ranges::transform(prefetch_deltas, std::back_inserter(prefetch_addresses),
	                       [miss_address](const auto& delta) { return miss_address + delta; });

	return prefetch_addresses;
}