scan: scan.c
	gcc -Wall -o scan scan.c

inject: inject.c
	gcc -Wall -o inject inject.c

dis: dis.c
	gcc -Wall -o dis dis.c -lcapstone

.PHONY:
clean:
	rm dis inject scan
