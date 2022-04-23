#pragma once

#include "ghb_pc_dc.h"
#include <boost/circular_buffer.hpp>
#include <concepts>
#include <numeric>
#include <optional>
#include <ranges>

template<std::unsigned_integral MissAddress>
struct Node;

template<std::unsigned_integral MissAddress>
using node_shr_ptr_t = std::shared_ptr<Node<MissAddress>>;

template<std::unsigned_integral MissAddress>
using node_wk_ptr_t = std::weak_ptr<Node<MissAddress>>;

template<std::unsigned_integral MissAddress>
using DeltaPair = std::array<MissAddress, 2>;


template<std::unsigned_integral MissAddress>
struct Node {
	explicit Node(MissAddress addressType) : data{addressType}, next{} {};
	MissAddress data{};
	node_wk_ptr_t<MissAddress> next{};
};

template<typename MissAddress>
class HistoryBuffer
{
public:
	explicit HistoryBuffer(std::size_t entries, std::size_t prefetch_degree) :
		entries{entries}, prefetch_degree{prefetch_degree}, fifo_queue{entries}
	{}

	[[nodiscard]] node_wk_ptr_t<MissAddress> linkedListHead(MissAddress address,
	                                                        node_wk_ptr_t<MissAddress>& old_head_ptr);
	[[nodiscard]] node_wk_ptr_t<MissAddress> pushNewAddress(MissAddress address);

	std::optional<DeltaPair<MissAddress>> firstDeltaPair(node_wk_ptr_t<MissAddress>& ghb_element);
	std::vector<MissAddress> prefetchDeltas(node_wk_ptr_t<MissAddress>& list_head_ptr,
	                                        DeltaPair<MissAddress>& first_delta_pair);

private:
	auto listDeltas(node_wk_ptr_t<MissAddress>& list_head_ptr);

	std::size_t entries{256u};
	std::size_t prefetch_degree{4u};
	boost::circular_buffer<node_shr_ptr_t<MissAddress>> fifo_queue{entries};
};

template<typename MissAddress>
node_wk_ptr_t<MissAddress> HistoryBuffer<MissAddress>::linkedListHead(MissAddress miss_address,
                                                                      node_wk_ptr_t<MissAddress>& old_head_ptr)
{
	// Push a new pointer in, link it up, and return the new pointer
	auto new_linked_list_head{pushNewAddress(miss_address)};
	new_linked_list_head.lock()->next = old_head_ptr;
	return new_linked_list_head;
}

template<typename MissAddress>
node_wk_ptr_t<MissAddress> HistoryBuffer<MissAddress>::pushNewAddress(MissAddress address)
{
	auto to_push{std::make_shared<Node<MissAddress>>(address)};
	fifo_queue.push_front(to_push);

	return {to_push};
}

template<typename MissAddress>
std::optional<DeltaPair<MissAddress>>
HistoryBuffer<MissAddress>::firstDeltaPair(node_wk_ptr_t<MissAddress>& ghb_element)
{
	auto curr_ptr{ghb_element.lock()};
	auto next_ptr{curr_ptr->next.lock()};
	if (!next_ptr) return {{}};
	auto next_next_ptr{next_ptr->next.lock()};
	if (!next_next_ptr) return {{}};

	// Otherwise,  return the delta pair. Older delta first, because that is the ordering of the delta stream.
	// Recall head of GHB is newest.
	return {{next_ptr->data - next_next_ptr->data, curr_ptr->data - next_ptr->data}};
}

template<typename MissAddress>
std::vector<MissAddress> HistoryBuffer<MissAddress>::prefetchDeltas(node_wk_ptr_t<MissAddress>& list_head_ptr,
                                                                    DeltaPair<MissAddress>& first_delta_pair)
{
	// Forward delta stream from the linked list
	auto delta_stream{listDeltas(list_head_ptr)};

	// If the delta stream is empty, return no prefetch deltas.
	if (delta_stream.empty()) return {};

	// Latest occurrence of the first delta pair in the linked list
	auto latest_occurrence_position{std::find_end(std::begin(delta_stream), std::end(delta_stream),
	                                              std::begin(first_delta_pair), std::end(first_delta_pair))};

	// No such pattern found; no prefetching
	if (latest_occurrence_position == std::end(delta_stream)) return {};

	// Pattern found, return up to prefetch_degree deltas *forward*
	auto prefetches_left{std::distance(latest_occurrence_position, std::end(delta_stream)) - 2};

	return {latest_occurrence_position + 2, latest_occurrence_position + prefetches_left};
}

template<typename MissAddress>
auto HistoryBuffer<MissAddress>::listDeltas(node_wk_ptr_t<MissAddress>& list_head_ptr)
{
	// skip first two elements, if third element is null, again, no prefetching
	auto curr{list_head_ptr};
	curr = curr.lock()->next;
	if (curr.lock()->next.expired()) return std::vector<MissAddress>{};

	std::vector<MissAddress> list_miss_addresses{};
	std::vector<MissAddress> delta_stream{};

	// traverse the entire list backwards from the third element and collect the miss addresses
	while (!curr.expired()) {
		list_miss_addresses.emplace_back(curr.lock()->data);
		curr = curr.lock()->next;
	}

	// list miss addresses are arranged in order of the newest first;
	// to calculate deltas, reverse, and calculate differences between elements.
	// Elements in delta_stream are now ordered from oldest to newest, from begin to end.
	std::adjacent_difference(std::rbegin(list_miss_addresses), std::rend(list_miss_addresses),
	                         std::back_inserter(delta_stream));

	// Return the forward delta stream, dropping the first element.
	delta_stream.erase(std::begin(delta_stream));
	return delta_stream;
}
