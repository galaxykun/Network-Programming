#include <pcap.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

void pcap_callback(u_char * arg, const struct pcap_pkthdr * header, const u_char * content)
{
	static int d = 0;
	printf("\rNo.%5d captured", ++d);
	fflush(stdout);
	//dump to file
	pcap_dump(arg, header, content);
}

int main()
{
	char errbuf[PCAP_ERRBUF_SIZE];
	char *device = NULL;
	int n;
	 printf("Please enter how many Packets you want to get? ");
	 scanf("%d",&n);
	 printf("\n\n");
	//get default interface name
	device = pcap_lookupdev(errbuf);
	if(!device) {
		    fprintf(stderr, "pcap_lookupdev(): %s\n", errbuf);
			    exit(1);
	}//end if
	 
	printf("Sniffing: %s\n", device);
	 
	//open interface
	pcap_t *handle = pcap_open_live(device, 65535, 1, 1, errbuf);
	if(!handle) {
		    fprintf(stderr, "pcap_open_live(): %s\n", errbuf);
			    exit(1);
	}//end if
	//open file handler
	const char *filename = "saved.pcap";
	pcap_dumper_t *dumper = pcap_dump_open(handle, filename);
	if(!dumper) {
		    fprintf(stderr, "pcap_dump_open(): %s\n", pcap_geterr(handle));
			    pcap_close(handle);
				    exit(1);
	}//end if
	     
	printf("Saving to %s...\n", filename);
	pcap_dumper_t *pcap_dump_open(pcap_t *p, const char *fname);
	//start capture loop
	if(0 != pcap_loop(handle, n, pcap_callback, (u_char *)dumper)) {
		    fprintf(stderr, "pcap_loop(): %s\n", pcap_geterr(handle));
	}//end if
	//flush and close
	pcap_dump_flush(dumper);
	pcap_dump_close(dumper);
	printf("\nDone\n");
	//free
	pcap_close(handle);
	return 0;
}
