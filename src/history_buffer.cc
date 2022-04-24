#include "history_buffer.h"

node_wk_ptr_t HistoryBuffer::linkedListHead(MissAddress miss_address, const node_wk_ptr_t& old_head_ptr)
{
	// Push a new pointer in, link it up, and return the new pointer
	auto new_linked_list_head{pushNewAddress(miss_address)};
	new_linked_list_head.lock()->next = old_head_ptr;
	return new_linked_list_head;
}

node_wk_ptr_t HistoryBuffer::pushNewAddress(MissAddress address)
{
	auto to_push{std::make_shared<Node>(address)};
	fifo_queue.push_front(to_push);

	return {to_push};
}

std::optional<DeltaPair> HistoryBuffer::firstDeltaPair(const node_wk_ptr_t& list_head_ptr)
{
	std::optional<DeltaPair> none_type{};
	const auto& curr_ptr{list_head_ptr};
	if (curr_ptr.expired()) return none_type;
	auto next_ptr{curr_ptr.lock()->next};
	if (next_ptr.expired()) return none_type;
	auto next_next_ptr{next_ptr.lock()->next};
	if (next_next_ptr.expired()) return none_type;

	// Otherwise, we have at least 3 elements; return the delta pair.
	// Older delta first, because that is the ordering of the delta stream.
	// Recall head of GHB is newest.
	return {{next_ptr.lock()->data - next_next_ptr.lock()->data, curr_ptr.lock()->data - next_ptr.lock()->data}};
}

auto HistoryBuffer::listDeltas(const node_wk_ptr_t& list_head_ptr)
{
	std::vector<MissAddress> empty_vec{};
	auto curr{list_head_ptr};

	std::vector<MissAddress> list_miss_addresses{};
	std::vector<MissAddress> delta_stream{};

	// traverse the entire list backwards from the first element and collect the miss addresses
	while (!curr.expired()) {
		list_miss_addresses.emplace_back(curr.lock()->data);
		curr = curr.lock()->next;
	}

	// Because of our backwards linked-list traversal, list miss addresses are arranged in order of the newest first.
	// Reverse to set oldest at begin().
	std::ranges::reverse(list_miss_addresses);
	// to calculate deltas, reverse, and calculate differences between elements.
	// Elements in delta_stream are now ordered from oldest to newest, from begin to end.
	std::adjacent_difference(std::begin(list_miss_addresses), std::end(list_miss_addresses),
	                         std::back_inserter(delta_stream));

	// Return the forward delta stream, dropping the first element.
	delta_stream.erase(std::begin(delta_stream));
	return delta_stream;
}

std::vector<MissAddress> HistoryBuffer::prefetchDeltas(const node_wk_ptr_t& list_head_ptr,
                                                       const DeltaPair& first_delta_pair) const
{
	// Forward delta stream from the linked list
	auto delta_stream{listDeltas(list_head_ptr)};

	// If the delta stream is empty, return no prefetch deltas.
	if (delta_stream.empty()) return {};

	// *Earliest* occurrence of the first delta pair in the linked list
	auto earliest_occurrence_position{std::find_end(std::begin(delta_stream), std::end(delta_stream) - 2,
	                                            std::begin(first_delta_pair), std::end(first_delta_pair))};

	// No such pattern found; no prefetching
	if (earliest_occurrence_position == std::end(delta_stream)) return {};

	// Pattern found, return up to prefetch_degree deltas *forward*
	auto prefetches_left{
			std::min(static_cast<std::size_t>(std::distance(earliest_occurrence_position, std::end(delta_stream)) - 2),
	                 prefetch_degree)};

	return {earliest_occurrence_position + 2, earliest_occurrence_position + 2 + prefetches_left};
}