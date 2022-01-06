#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pcap.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include<time.h>

#define IPv4_TYPE 2048
#define IPv6_TYPE 34525
#define PPOE_TYPE 34916

// Custom enum for different protocol versions
typedef enum {TCP, UDP, ICMP, ICMPv6, UNKNOWN, EXTH} protocol_ver;
protocol_ver cur_protocol = UNKNOWN;

// Custom struct for an IPv6 header
struct ipv6 {
	uint32_t vtcfl;
	uint16_t length;
	uint8_t next_header;
	uint8_t hop_limit;
	struct in6_addr ip_src;
	struct in6_addr ip_dst;
};


void print_ipv4_header(struct ip*); 
void print_ipv6_header(struct ipv6*); 
void print_tcp_header(struct tcphdr*); 
void print_udp_header(struct udphdr*); 
void print_icmp_header(struct icmp*, protocol_ver); 
const char* get_protocol_name(int); 
const char* get_icmpv4_code(int); 
const char* get_icmpv6_code(int); 

int main(int argc, char **argv)
{
	struct pcap_pkthdr header;
	struct tm *tminfo;
	
	const u_char *cur_packet;
	pcap_t *handle;
	char errbuf[PCAP_ERRBUF_SIZE];
    char * devStr;
    devStr = pcap_lookupdev(errbuf);
    
    int n = 0;
    printf(" online enter 0 \n offline enter 1\n");
    scanf("%d",&n);
    //printf("%d\n",n);
    char a[100]={0};
    if(n){
        //printf("please enter file\n");
        scanf("%s",a);

        handle = pcap_open_offline(a, errbuf);
    }
    else
        handle = pcap_open_live(devStr, 65535, 1, 0, errbuf);
    
	if (handle == NULL) {
		printf("Couldn't open pcap file %s: %s\n", a, errbuf);
	}

	int count = 1; // Initialise a counter of packets
	
	// Loop through packets
	while (cur_packet = pcap_next(handle, &header)) {
		time_t t=header.ts.tv_sec;
		// Flags
		bool ipv4_flag = false;
		bool ipv6_flag = false;
		bool ppoe_flag = false;

		// 0. Standard information about packet,
		printf("Packet number.%d\n", count);
		tminfo=localtime(&t);
		printf("time : %s",asctime(tminfo));
		count = count + 1;
		//printf("Packet length: %d\n", header.len);

		int packet_type = ((int) (cur_packet[12]) << 8) | (int) cur_packet[13];
		// Set flags based on ether type.
		switch(packet_type)
		{
			case IPv4_TYPE:
			printf("Ether type: IPv4\n");
			ipv4_flag = true;
			break;

			case IPv6_TYPE:
			printf("Ether type: IPv6\n");
			ipv6_flag = true;
			break;

			case PPOE_TYPE:
			printf("Ether type: PPOE\n");
			ppoe_flag = true;
			break;

			default:
			printf("Unknown ether type.\n");
			break;
		}
		
		// 2. Get header info: from/to address, protocol.
		if (ipv4_flag) {
			cur_packet += 14;
			struct ip *ip_header = (struct ip*) cur_packet;
			print_ipv4_header(ip_header);
		}
		else if (ipv6_flag) {
			cur_packet += 14;
			struct ipv6 *ipv6_header = (struct ipv6*) cur_packet;
			print_ipv6_header(ipv6_header);
		}
		else if (ppoe_flag) {
			cur_packet += 42;
			struct ipv6 *ipv6_header = (struct ipv6*) cur_packet;
			print_ipv6_header(ipv6_header);
		}

		
		// 3. Get packet info: src/dst ports, ICMP info.
		// IPv4
		if (ipv4_flag && cur_protocol == TCP) {
			cur_packet += 20;
			struct tcphdr *tcp_header = (struct tcphdr*) cur_packet;
			print_tcp_header(tcp_header);
		}
		else if (ipv4_flag && cur_protocol == UDP) {
			cur_packet += 20;
			struct udphdr *udp_header = (struct udphdr*) cur_packet;
			print_udp_header(udp_header);
		}
		else if (ipv4_flag && cur_protocol == ICMP) {
			cur_packet += 20;
			struct icmp *icmp_header = (struct icmp*) cur_packet;
			print_icmp_header(icmp_header, ICMP);
		}
		// IPv6
		else if (ipv6_flag && cur_protocol == TCP) {
			cur_packet += 40;
			struct tcphdr *tcp_header = (struct tcphdr*) cur_packet;
			print_tcp_header(tcp_header);
		}
		else if (ipv6_flag && cur_protocol == UDP) {
			cur_packet += 40;
			struct udphdr *udp_header = (struct udphdr*) cur_packet;
			print_udp_header(udp_header);
		}
		else if (ipv6_flag && cur_protocol == ICMPv6) {
			cur_packet += 40;
			struct icmp *icmp_header = (struct icmp*) cur_packet;
			print_icmp_header(icmp_header, ICMPv6);
		}
		
		printf("\n");
	}
	printf("total pocket : %d\n",count-1);
	pcap_close(handle);
	return 0;
}

// Prints details of an IPv4 header.
void print_ipv4_header(struct ip* ip_header) {
	printf("From: %s\n", inet_ntoa(ip_header->ip_src)); // inet_ntoa() is deprecated in favour of inet_ntop(), but bypasses the char[] arguments
	printf("To: %s\n", inet_ntoa(ip_header->ip_dst));
	printf("Protocol: %s\n", get_protocol_name(ip_header->ip_p));
}

// Prints details of an IPv6 header.
void print_ipv6_header(struct ipv6* ipv6_header) {
	char srcaddr[46];
	char dstaddr[46];
	inet_ntop(AF_INET6, &ipv6_header->ip_src, srcaddr, sizeof(srcaddr)); // inet_ntop() takes a binary address (both IP versions) and returns in text form
	inet_ntop(AF_INET6, &ipv6_header->ip_dst, dstaddr, sizeof(dstaddr));
	printf("From: %s\n", srcaddr);
	printf("To: %s\n", dstaddr);
	printf("Protocol: %s\n", get_protocol_name(ipv6_header->next_header));
}

// Prints details of a TCP header.
void print_tcp_header(struct tcphdr* tcp_header) {
	printf("Src port: %d\n", ntohs(tcp_header->th_sport)); // ntohs() takes a 16-bit number in TCP/IP network byte order and returns in host byte order
	printf("Dst port: %d\n", ntohs(tcp_header->th_dport));
}

// Prints details of a UDP header.
void print_udp_header(struct udphdr* udp_header) {
	printf("Src port: %d\n", ntohs(udp_header->uh_sport));
	printf("Dst port: %d\n", ntohs(udp_header->uh_dport));
}

// Prints details of an ICMP (v6 too) header.
void print_icmp_header(struct icmp* icmp_header, protocol_ver version) {
	if (version == ICMP) {
		printf("ICMP type %d: %s\n", icmp_header->icmp_type, get_icmpv4_code(icmp_header->icmp_type));
	}
	else if (version == ICMPv6) {
		printf("ICMPv6 type %d: %s\n", icmp_header->icmp_type, get_icmpv6_code(icmp_header->icmp_type));
	}
}

// Uses a protocol number and returns a string based off that number from a subset of protocols.
const char* get_protocol_name(int value) {
	//printf("VALUE: %d\n", value);
	const char *protocol = " ";
	switch(value)
	{
		case 0:
		protocol = "IPv6 Extension - Hop-by-hop";
		cur_protocol = EXTH;
		return protocol;

		case 1:
		protocol = "ICMP";
		cur_protocol = ICMP;
		return protocol;

		case 6:
		protocol = "TCP";
		cur_protocol = TCP;
		return protocol;

		case 17:
		protocol = "UDP";
		cur_protocol = UDP;
		return protocol;

		case 43:
		protocol = "IPv6 Extension - Routing";
		cur_protocol = EXTH;
		return protocol;

		case 44:
		protocol = "IPv6 Extension - ESP";
		cur_protocol = EXTH;
		return protocol;

		case 50:
		protocol = "IPv6 Extension - Destination options";
		cur_protocol = EXTH;
		return protocol;

		case 51:
		protocol = "IPv6 Extension - AH";
		cur_protocol = EXTH;
		return protocol;

		case 58:
		protocol = "ICMPv6";
		cur_protocol = ICMPv6;
		return protocol;

		case 60:
		protocol = "IPv6 Extension - Destination options";
		cur_protocol = EXTH;
		return protocol;

		case 135:
		protocol = "IPv6 Extension - Mobility";
		cur_protocol = EXTH;
		return protocol;

		default:
		protocol = "Unknown";
		cur_protocol = UNKNOWN;
		return protocol;
	}
}

// Uses an ICMP type number and returns the code corresponding to that number from a subset of codes.
const char* get_icmpv4_code(int value) {
	const char *code = " ";
	switch(value)
	{
		case 0:
		code = "Echo reply (used to ping)";
		return code;

		case 8:
		code = "Echo request (used to ping)";
		return code;

		default:
		code = "Unknown";
		return code;
	}
}

// Uses an ICMPv6 type number and returns the code corresponding to that number from a subset of codes.
const char* get_icmpv6_code(int value) {
	const char *code = " ";
	switch(value)
	{
		case 134:
		code = "Router Advertisement (NDP)";
		return code;

		default:
		code = "Unknown";
		return code;
	}
}
