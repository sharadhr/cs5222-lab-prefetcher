// Global History Buffer Program Counter/Delta Correction Prefetcher
// Sharadh Rajaraman
// A0189906L

#include "ghb_pc_dc.h"
#include "prefetcher.h"
#include <cstdio>

static GHBPCDCPrefetcher<Address, Address> ghbPcDcPrefetcher{256u, 4u};

[[maybe_unused]] void l2_prefetcher_initialize([[maybe_unused]] int cpu_num)
{
	// We do nothing here. The prefetcher can't really be initialised in a function
	// because it's static, and it's static because the header may not be changed,
	// by the rules of the competition.
	printf("Global History Buffer Prefetcher with Program Counter-Localised Delta Correction (GHB PC/DC)\n");
	// you can inspect these knob values from your code to see which configuration you're running in
	printf("Knobs visible from prefetcher: %d %d %d\n", knob_scramble_loads, knob_small_llc, knob_low_bandwidth);
}

[[maybe_unused]] void l2_prefetcher_operate([[maybe_unused]] int cpu_num, Address addr, Address ip, int cache_hit)
{
	// If a cache hit, no prefetching
	if (!cache_hit) return;

	// otherwise, retrieve list of addresses to prefetch
	auto to_prefetch{ghbPcDcPrefetcher.missOperate(ip, addr)};

	// prefetch all addresses listed
	std::ranges::for_each(to_prefetch, [addr](const auto& prefetch_address) {
		l2_prefetch_line(0, addr, prefetch_address, get_l2_mshr_occupancy(0) < 8 ? FILL_L2 : FILL_LLC);
	});

	// uncomment this line to see all the information available to make prefetch decisions
	// printf("(0x%llx 0x%llx %d %d %d) ", addr, ip, cache_hit, get_l2_read_queue_occupancy(0), get_l2_mshr_occupancy(0));
}

[[maybe_unused]] void l2_cache_fill([[maybe_unused]] int cpu_num, [[maybe_unused]] Address addr, [[maybe_unused]] int set,
                   [[maybe_unused]] int way, [[maybe_unused]] int prefetch, [[maybe_unused]] Address evicted_addr)
{
	// uncomment this line to see the information available to you when there is a cache fill event
	// printf("0x%llx %d %d %d 0x%llx\n", addr, set, way, prefetch, evicted_addr);
}

[[maybe_unused]] void l2_prefetcher_heartbeat_stats([[maybe_unused]] int cpu_num) { printf("Prefetcher heartbeat stats\n"); }

[[maybe_unused]] void l2_prefetcher_warmup_stats([[maybe_unused]] int cpu_num) { printf("Prefetcher warmup complete stats\n\n"); }

[[maybe_unused]] void l2_prefetcher_final_stats([[maybe_unused]] int cpu_num) { printf("Prefetcher final stats\n"); }
