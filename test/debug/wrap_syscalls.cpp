// src/debug/wrap_syscalls.cpp
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

static long getenv_long(const char *name, long defv) {
	const char *s = std::getenv(name);
	if (!s || !*s)
		return defv;
	return std::atol(s);
}

extern "C" {

// ---- accept ----
int __real_accept(int, struct sockaddr *, socklen_t *);
int __wrap_accept(int fd, struct sockaddr *addr, socklen_t *len) {
	static long call = 0;
	++call;

	const long crash_at = getenv_long("WRAP_ACCEPT_CRASH_AT", -1);
	const long fail_at = getenv_long("WRAP_ACCEPT_FAIL_AT", -1);

	if (crash_at > 0 && call == crash_at) {
		// SIGABRT: Valgrindがリークサマリを出せることが多い
		std::abort();
	}
	if (fail_at > 0 && call >= fail_at) {
		errno = EMFILE; // “FD枯渇っぽい”失敗を擬似的に
		return -1;
	}
	return __real_accept(fd, addr, len);
}

// ---- epoll_ctl ----
int __real_epoll_ctl(int, int, int, struct epoll_event *);
int __wrap_epoll_ctl(int epfd, int op, int fd, struct epoll_event *ev) {
	static long call = 0;
	++call;

	const long crash_at = getenv_long("WRAP_EPOLL_CTL_CRASH_AT", -1);
	const long fail_at = getenv_long("WRAP_EPOLL_CTL_FAIL_AT", -1);

	if (crash_at > 0 && call == crash_at) {
		std::abort();
	}
	if (fail_at > 0 && call >= fail_at) {
		errno = EINVAL; // 適当なエラー（シナリオに応じて変更）
		return -1;
	}
	return __real_epoll_ctl(epfd, op, fd, ev);
}

} // extern "C"
