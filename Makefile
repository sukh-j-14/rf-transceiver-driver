CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Werror -O2
.PHONY: test test-demo
test:
	mkdir -p build
	$(CC) $(CFLAGS) src/radio.c src/mock.c src/main.c -o build/driver-tests
	./build/driver-tests
	$(CC) $(CFLAGS) src/radio.c src/mock.c src/edge_tests.c -o build/edge-tests
	./build/edge-tests
test-demo:
	node demo/model.test.cjs
