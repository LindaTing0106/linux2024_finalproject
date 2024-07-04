CFLAGS := -I.. -Wall -O2 -D_GNU_SOURCE
LDFLAGS := -lpthread

ALL := test_checking1 test_linux test_checking2
# test_checking2 test_checking3 test_checking4 test_checking5 test_checking6 
all: $(ALL)
.PHONY: all

# test_pthread: CFLAGS += -DUSE_PTHREADS
# test_pthread: main.c
# 	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)
	
test_checking1: CFLAGS += -DUSE_CHECKING -DMUTEX=1
test_checking1: main.c
	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)

test_checking2: CFLAGS += -DUSE_CHECKING -DMUTEX=2
test_checking2: main.c
	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)

# test_checking3: CFLAGS += -DUSE_CHECKING -DMUTEX=3
# test_checking3: main.c
# 	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)

# test_checking4: CFLAGS += -DUSE_CHECKING -DMUTEX=4
# test_checking4: main.c
# 	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)

# test_checking5: CFLAGS += -DUSE_CHECKING -DMUTEX=5
# test_checking5: main.c
# 	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)

# test_checking6: CFLAGS += -DUSE_CHECKING -DMUTEX=6
# test_checking6: main.c
# 	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)

test_linux: CFLAGS += -DUSE_LINUX
test_linux: main.c
	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)



clean:
	$(RM) $(ALL)
.PHONY: clean
