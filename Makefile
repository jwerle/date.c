
all: ok test

ok:
	$(MAKE) -C deps/ok

test:
	$(CC) deps/ok/libok.a date.c test.c -o test-date
	./test-date

clean:
	rm -f test-date
	$(MAKE) clean -C deps/ok
