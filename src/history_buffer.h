#pragma once

#include <boost/circular_buffer.hpp>
#include <memory>
#include <numeric>
#include <optional>

struct Node;

using MissAddress = unsigned long long;

struct Node {
	explicit Node(MissAddress addressType) : data{addressType}, next{} {};
	MissAddress data{};
	std::weak_ptr<Node> next{};
};

using node_shr_ptr_t = std::shared_ptr<Node>;
using node_wk_ptr_t = std::weak_ptr<Node>;
using DeltaPair = std::array<MissAddress, 2>;

class HistoryBuffer
{
public:
	[[maybe_unused]] HistoryBuffer(std::size_t entries, std::size_t prefetch_degree) :
		prefetch_degree{prefetch_degree}, fifo_queue{entries}
	{}

	[[nodiscard]] node_wk_ptr_t linkedListHead(MissAddress address, const node_wk_ptr_t& old_head_ptr);
	[[nodiscard]] node_wk_ptr_t pushNewAddress(MissAddress address);

	static std::optional<DeltaPair> firstDeltaPair(const node_wk_ptr_t& list_head_ptr);
	[[nodiscard]] std::vector<MissAddress> prefetchDeltas(const node_wk_ptr_t& list_head_ptr,
	                                                      const DeltaPair& first_delta_pair) const;

private:
	static auto listDeltas(const node_wk_ptr_t& list_head_ptr);

	std::size_t prefetch_degree{4u};
	boost::circular_buffer<node_shr_ptr_t> fifo_queue{256u};
};
