#include "seed.hpp"

#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

/**
 * @brief ビットミキシング関数（Xorshift風の簡易版）
 * @param x 入力値
 * @return ミキシング後の値
 */
namespace {
size_t mixBits(size_t x) {
	x ^= (x << 13);
	x ^= (x >> 17);
	x ^= (x << 5);
	return x;
}
} // namespace

// clang-format off
unsigned int generateSeed() {
	size_t
	seed  = static_cast< size_t >(std::time(NULL));
	seed ^= static_cast< size_t >(std::clock()) << 16;
	seed ^= reinterpret_cast< size_t >(&seed);

#if defined(__unix__) || defined(__APPLE__) || defined(__MACH__)
	seed ^= static_cast< size_t >(getpid()) << 8; // Process ID
	struct timeval tv; // gettimeofday()でマイクロ秒精度の時刻を取得
	if (gettimeofday(&tv, NULL) == 0) {
		seed ^= static_cast< size_t >(tv.tv_usec);
		seed ^= static_cast< size_t >(tv.tv_sec) << 24;
	}
	// /dev/urandomからエントロピー取得を試行
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd >= 0) {
		unsigned int entropy = 0;
		if (read(fd, &entropy, sizeof(entropy)) == sizeof(entropy)) {
			seed ^= static_cast< size_t >(entropy);
		}
		close(fd);
	}
#endif

	// ビットミキシングでエントロピー強化
	seed  = mixBits(seed);
	seed ^= mixBits(seed + 1);
	seed ^= mixBits(seed * 1103515245UL + 12345UL);

	return static_cast< unsigned int >(seed);
}
// clang-format on
