

uint64 xTSC() { asm volatile ("mrs %0, cntvct_el0;" : "=r"(cntvct) :: "memory"); }

static inline uint64 xTSC() { uint64_t cntvct; asm volatile ("mrs %0, cntvct_el0;" : "=r"(cntvct) :: "memory"); return cntvct; }