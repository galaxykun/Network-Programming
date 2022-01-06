all:
	gcc hw3.c -o hw3 -lpcap
	gcc savePacket.c -o savePacket -lpcap
clean:
	rm hw3
run:
	sudo ./savePacket
	sudo ./hw3