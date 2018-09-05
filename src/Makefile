%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FPIC) -c -o $@ $<

compile: invoke_cli invoke_serv notify_serv notify_cli event_serv event_cli ubustest

event_serv: event_serv.o
	$(CC) $(LDFLAGS) -o $@ $^

event_cli: event_cli.o
	$(CC) $(LDFLAGS) -o $@ $^

notify_serv: notify_serv.o
	$(CC) $(LDFLAGS) -o $@ $^

notify_cli: notify_cli.o
	$(CC) $(LDFLAGS) -o $@ $^

invoke_serv: invoke_serv.o
	$(CC) $(LDFLAGS) -o $@ $^

invoke_cli: invoke_cli.o
	$(CC) $(LDFLAGS) -o $@ $^


ubustest: server.o count.o
	$(CC) $(LDFLAGS) -o $@ $^


install: compile
	mkdir -p $(DESTDIR)/usr/bin
	#cp ubustest $(DESTDIR)/usr/bin/ubustest

clean:
	rm -f ubustest *.o
