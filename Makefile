NAME		:= webserv

UNAME_S 	:= $(shell uname -s)

CXX			:= c++
CXXFLAG		:= -Wall -Wextra -Werror -std=c++98 -pedantic
OPT			:= -O3
RM			:= rm -rf
DEFINE		:= -D_GLIBCXX_USE_CXX11_ABI=0

ROOT_DIR	:= .
SRC_DIR		:= $(ROOT_DIR)/src
OBJ_DIR		:= $(ROOT_DIR)/obj
CONF_DIR	:= $(ROOT_DIR)/config
CONF		:= $(CONF_DIR)/default.yaml
LOG_DIR		:= $(ROOT_DIR)/logs
TEST_DIR	:= $(ROOT_DIR)/test

# Docker settings
DOCKER_IMAGE	:= webserv-devenv
DOCKER_TAG		:= latest

VENV_DIR	:= $(ROOT_DIR)/.venv

SRC 	:= $(shell find $(SRC_DIR) -path '*/test' -prune -o -name '*.cpp' -print)
OBJ		:= $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# =============== 42 RULES ==============

.PHONY: all clean fclean re

all:
	$(MAKE) $(NAME) -j $(shell nproc)

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

# =========== ORIGINAL RULES ============

.PHONY: run

# Build and run
run: $(NAME)
	./$(NAME) $(CONF)

# Clean log files
clog:
	$(RM) $(LOG_DIR)/*.log*

# Aliases
c: clog
	$(RM) $(OBJ_DIR)
f: c
	$(RM) $(NAME)
r: f all

$(LOG_DIR):
	@mkdir -p $(LOG_DIR)

setuphooks:
	@git config --local core.hooksPath .githooks
	@chmod -R 744 .githooks/

play-netpractice: $(NAME) submodule
	./$(NAME) $(CONF)

# ============ DEBUG RULE ============

.PHONY: debug debug-run valgrind debug-clash

debug: OPT		:= -g -O1 -fno-omit-frame-pointer -fsanitize=address,leak
debug: DEFINE	:= -DDEBUG_MODE=DEBUG_ALL
debug: fclean
	$(MAKE) $(NAME) OPT="$(OPT)" DEFINE="$(DEFINE)" -j $(shell nproc)

debug-run: debug
	ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1 ./$(NAME) $(CONF)

valgrind: $(NAME)
	valgrind \
	--tool=memcheck \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-fds=yes \
	--trace-children=yes \
	--num-callers=50 \
	--error-exitcode=42 \
	--log-file=valgrind.%p.log \
	./$(NAME) $(CONF) \
	2>&1 | tee valgrind_run.log

debug-clash: SRC	+= $(ROOT_DIR)/test/debug/wrap_syscalls.cpp
debug-clash: LFLAG	+= -Wl,--wrap=accept -Wl,--wrap=epoll_ctl
debug-clash: OPT	:= -g -O0 -fno-omit-frame-pointer
debug-clash: fclean
	$(MAKE) $(NAME) OPT="$(OPT)" DEFINE="$(DEFINE)" SRC="$(SRC)" LFLAG="$(LFLAG)" -j $(shell nproc)
	export WRAP_ACCEPT_CRASH_AT=300; \
	WRAP_EPOLL_CTL_FAIL_AT=200;

# =========== PYTEST ENVIRONMENT ============

.PHONY: pyinit test

# Create a virtual environment and install dependencies
pyinit:
	python3 -m venv $(VENV_DIR)
	@. $(VENV_DIR)/bin/activate; \
	python -m pip install --upgrade pip; \
	if ! command -v uv >/dev/null 2>&1; then \
		python -m pip install -U uv >/dev/null 2>&1 || true; \
	fi; \
	if command -v uv >/dev/null 2>&1; then \
		uv pip install -r $(TEST_DIR)/requirements.txt; \
	else \
		echo "uv install failed or unavailable; using pip"; \
		python -m pip install -r $(TEST_DIR)/requirements.txt; \
	fi
test:
	. $(VENV_DIR)/bin/activate && pytest $(TEST_DIR)/test_suite

# ============= STATIC ANALYSIS =============

.PHONY: tidy check

# clang-tidy rule
TIDY := clang-tidy
TIDYFLAGS := --warnings-as-errors=* -checks=*

tidy: $(SRC)
	$(TIDY) $(TIDYFLAGS) $^ -- -std=c++98 $(CXXFLAG)
	@echo "================================"
	@echo "== Static Analysis Complete! =="
	@echo "================================"

# cppcheck rule
CPPCHECK := cppcheck
CPPCHECKFLAGS := --enable=all --inconclusive --std=c++03 --force --quiet

check:
	@$(CPPCHECK) $(CPPCHECKFLAGS) $(SRC)
	@echo "================================"
	@echo "== Static Analysis Complete! =="
	@echo "================================"

# ============= BUILD RULES =============

# Check if OS is Linux (Ubuntu), otherwise use Docker
ifeq ($(UNAME_S),Linux)
$(NAME): $(OBJ) | $(LOG_DIR)
	$(CXX) $(CXXFLAG) $(OPT) $(IDFLAG) $(LFLAG) $(DEFINE) -o  $@ $^
	@echo "====================="
	@echo "== Build Complete! =="
	@echo "====================="
	@echo "[Executable]: $(NAME)"
	@echo "[OS/Arch]: $(UNAME_S)"
	@echo "[Config]: $(CONF)"
	@echo "[Include]: $(INC_DIR)"
	@echo "[Compiler flags/CXXFLAG]: $(CXXFLAG)"
	@echo "[Linker flags/LFLAG]: $(LFLAG)"
	@echo "[Optimizer flags/OPT]: $(OPT)"
	@echo "[DEFINE]: $(DEFINE)"
	@echo "====================="
else
$(NAME):
	@echo "=========================="
	@echo "== Building with Docker =="
	@echo "=========================="
	@echo "[OS/Arch]: $(UNAME_S)"
	@echo "[Docker Image]: $(DOCKER_IMAGE):$(DOCKER_TAG)"
	@if ! docker image inspect $(DOCKER_IMAGE):$(DOCKER_TAG) >/dev/null 2>&1; then \
		echo "Building Docker image..."; \
		docker buildx build --load -t $(DOCKER_IMAGE):$(DOCKER_TAG) -f $(ROOT_DIR)/Dockerfile $(ROOT_DIR); \
	fi
	@echo "Building $(NAME) in Docker container..."
	@docker run --rm -v $(ROOT_DIR):/workspace -w /workspace $(DOCKER_IMAGE):$(DOCKER_TAG) \
		make $(NAME) OPT="$(OPT)" DEFINE="$(DEFINE)" -j $$(nproc)
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAG) $(OPT) $(IDFLAG) $(DEFINE) -fPIC -MMD -MP  -c $< -o $@

# ============= DOCKER RULES =============

.PHONY: docker-build docker-clean

docker-build:
	@echo "Building Docker image: $(DOCKER_IMAGE):$(DOCKER_TAG)"
	@docker buildx build --load -t $(DOCKER_IMAGE):$(DOCKER_TAG) -f $(ROOT_DIR)/Dockerfile $(ROOT_DIR)

docker-clean:
	@echo "Removing Docker image: $(DOCKER_IMAGE):$(DOCKER_TAG)"
	@docker rmi $(DOCKER_IMAGE):$(DOCKER_TAG) 2>/dev/null || echo "Image not found or already removed"

# ================ MISC =================

.PHONY: nm nmbin printsrc printobj fill view submodule help

nm:
	@nm $(OBJ) | grep ' U ' | awk '{print $$2}' | sort | uniq

nmbin:
	@nm $(NAME) | grep ' U ' | awk '{print $$2}' | sort | uniq

printsrc:
	@echo $(SRC) | tr ' ' '\n' | sort

printobj:
	@echo $(OBJ) | tr ' ' '\n' | sort

fill:
	@./tools/fillEmptyDir.sh

view:
	@./tools/rawCodeViewer.sh

submodule:
	git submodule update --init --recursive

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all              Build all targets"
	@echo "  run              Run the program"
	@echo "  clean            Clean object files"
	@echo "  fclean           Fully clean (clean + remove executable)"
	@echo "  re               Rebuild (fclean + all)"
	@echo "  clog             Clean log files"
	@echo "  c                Alias for 'clean' and 'clog'"
	@echo "  f                Alias for 'fclean' and 'clog'"
	@echo "  r                Alias for 're' (fclean + all) and 'clog'"
	@echo "  debug            Build with debug flags"
	@echo "  setuphooks       Set up git hooks"
	@echo "  play-netpractice Build and run with netpractice config"
	@echo "  pyinit           Initialize Python virtual environment for tests"
	@echo "  test             Run Python tests using pytest"
	@echo "  tidy             Run static analysis using clang-tidy"
	@echo "  check            Run static analysis using cppcheck"
	@echo "  docker-build     Build Docker image for cross-platform compilation"
	@echo "  docker-clean     Remove Docker image"
	@echo "  nm               List undefined symbols in object files"
	@echo "  nmbin            List undefined symbols in the executable"
	@echo "  printsrc         Print source files"
	@echo "  printobj         Print object files"
	@echo "  fill             Fill empty directories"
	@echo "  view             View source code"
	@echo "  submodule        Update and initialize git submodules"
	@echo "  help             Print this help message"
