#include "Write.hpp"
#include <errno.h>
#include <unistd.h>

namespace Write {

void xwrite(int fd, const char *data, size_t len) {
	size_t writtenTotal = 0;
	while (writtenTotal < len) {
		ssize_t w = write(fd, data + writtenTotal, len - writtenTotal);
		if (w > 0) {
			writtenTotal += static_cast< size_t >(w);
			continue;
		}
		if (w < 0 && errno == EINTR) {
			continue; // 再試行
		}
		break; // それ以外のエラーは諦める
	}
}

} // namespace Write
