/* ISA Project DNS */
/* Author: Adam Kala */
/* Login: xkalaa00 */
/* Year 2023 */

/* 	Main code was from open source:
	https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/socket.h>

/* List of DNS Servers registered on the system */
char dns_servers[10][100];

#define A 1		/* Ipv4 Address */
#define AAAA 28 /* Ipv6 Address */
#define NS 2	/* Nameserver */
#define CNAME 5 /* Canonical Name */
#define SOA 6	/* Start of Authority Zone */
#define PTR 12	/* Domain Name Pointer */
#define MX 15	/* Mail Server */

#define BUFFERSIZE 65535 /* Defines basic buffer size*/
#define ERROR -1		 /* Defines return code for error */

/* DNS header structure */
struct DNS_HEADER
{
	uint16_t id;

	/* Setting only one bit */
	unsigned char rd : 1; /* Recursion */
	unsigned char tc : 1; /* Truncated */
	unsigned char aa : 1; /* Authoritive */
	unsigned char opcode : 4;
	unsigned char qr : 1; /* Query/Response Flag */

	unsigned char rcode : 4; /* Response */
	unsigned char cd : 1;	 /* Checking */
	unsigned char ad : 1;	 /* Authenticated */
	unsigned char z : 1;	 /* z! Reserved */
	unsigned char ra : 1;	 /* Recursion Available */

	uint16_t q_count;	 /* Number Of Question Entries */
	uint16_t ans_count;	 /* Number Of Answer Entries */
	uint16_t auth_count; /* Number Of Authority Entries */
	uint16_t add_count;	 /* Number Of Resource Entries */
};

/* Struct for the Question we ask */
struct QUESTION
{
	uint16_t qtype;
	uint16_t qclass;
};

/* Recieved data from the Question */
#pragma pack(push, 1)
struct R_DATA
{
	uint16_t type;
	uint16_t _class;
	uint32_t ttl;
	uint16_t data_len;
};
#pragma pack(pop)

struct RES_RECORD
{
	unsigned char *name;
	struct R_DATA *resource;
	unsigned char *rdata;
};

typedef struct
{
	unsigned char *name;
	struct QUESTION *ques;
} QUERY;

/* Variables for parsed Arguments */
struct filter_variables
{
	bool recurse;	   /* -r */
	bool reverse;	   /* -x */
	bool six;		   /* -6 */
	bool server;	   /* -s */
	bool port_enabled; /* -p */
	int port_number;   /* port number after -p*/
	bool address;	   /* address name/IP */
	int type;
};

// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------

void recieved_info(struct QUESTION *, unsigned char *, struct DNS_HEADER *, unsigned char *, struct RES_RECORD *, struct RES_RECORD *, struct RES_RECORD *, unsigned char *, struct filter_variables *, struct sockaddr_in);
unsigned char *read_name(unsigned char *, unsigned char *, int *);
void get_dns_servers();
void change_to_dns(unsigned char *, unsigned char *);
const char *get_type(int);
const char *reverse_IPv6(const char *);
char *reverse_IPv4(char *);
void ngethostbyname(const char *, const char *, int, struct filter_variables *);

// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------

void recieved_info(struct QUESTION *qinfo, unsigned char *questioned_address,
				   struct DNS_HEADER *dns, unsigned char *reader,
				   struct RES_RECORD *answers, struct RES_RECORD *auth, struct RES_RECORD *addit,
				   unsigned char *buf, struct filter_variables *filters, struct sockaddr_in a)
{
	int i, j, stop;
	// ------------------------------------------------------------------------------------------------------
	printf("Authoritative: %s, Recursive: %s, Truncated: %s\n",
		   (dns->aa == 1) ? "Yes" : "No", (dns->rd == 1) ? "Yes" : "No", (dns->tc == 1) ? "Yes" : "No");

	// ------------------------------------------------------------------------------------------------------
	const char *type = get_type(htons(qinfo->qtype));
	printf("Question section (%d)\n", ntohs(dns->q_count));
	printf("  %s, %s, IN\n", questioned_address, type);
	// ------------------------------------------------------------------------------------------------------

	/* Read Answers, store them in answers[] */
	stop = 0;
	for (i = 0; i < ntohs(dns->ans_count); i++)
	{
		answers[i].name = read_name(reader, buf, &stop);
		reader = reader + stop;

		answers[i].resource = (struct R_DATA *)(reader);
		reader = reader + sizeof(struct R_DATA);

		/* Read IPv4 or IPv6 */
		if (ntohs(answers[i].resource->type) == 1 || ntohs(answers[i].resource->type) == 28)
		{
			answers[i].rdata = (unsigned char *)malloc(ntohs(answers[i].resource->data_len));

			for (j = 0; j < ntohs(answers[i].resource->data_len); j++)
			{
				answers[i].rdata[j] = reader[j];
			}

			answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

			reader = reader + ntohs(answers[i].resource->data_len);
		}
		else
		{
			answers[i].rdata = read_name(reader, buf, &stop);
			reader = reader + stop;
		}
	}

	printf("Answer section (%d)\n", ntohs(dns->ans_count));
	for (i = 0; i < ntohs(dns->ans_count); i++)
	{
		printf("  %s, ", answers[i].name);

		/* Get The Query Type */
		unsigned short qtype = ntohs(answers[i].resource->type);
		const char *type_name = get_type(qtype);
		printf("%s", type_name);
		printf(", IN");

		/* Extract TTL */
		unsigned int ttl = ntohl(answers[i].resource->ttl);
		printf(", %u", ttl);

		if (qtype == A)
		{
			/* https://stackoverflow.com/questions/16373248/convert-ip-for-reverse-ip-lookup */
			if (filters->reverse == true)
			{
				long *p;
				p = (long *)answers[i].rdata;
				a.sin_addr.s_addr = (*p);
				unsigned long addr = a.sin_addr.s_addr;
				/* Reversing the IP with >> x */
				unsigned long reversedAddr = ((addr & 0xFF000000) >> 24) |
											 ((addr & 0x00FF0000) >> 8) |
											 ((addr & 0x0000FF00) << 8) |
											 ((addr & 0x000000FF) << 24);

				printf(", %ld.%ld.%ld.%ld.in-addr.arpa", (reversedAddr) & 0xFF, (reversedAddr >> 8) & 0xFF, (reversedAddr >> 16) & 0xFF, (reversedAddr >> 24) & 0xFF);
			}
			else
			{
				long *p;
				p = (long *)answers[i].rdata;
				a.sin_addr.s_addr = (*p);
				printf(", %s", inet_ntoa(a.sin_addr));
			}
		}
		else if (qtype == AAAA)
		{
			if (filters->reverse == true)
			{
				struct in6_addr ipv6_addr;
				memcpy(&ipv6_addr, answers[i].rdata, sizeof(struct in6_addr));

				char ipv6_str[INET6_ADDRSTRLEN];
				char *ptr_ptr = ipv6_str;
				int len = INET6_ADDRSTRLEN;

				for (int i = len - 1, count = 0; i >= 0; i--)
				{
					if (ipv6_str[i] != ':')
					{
						*ptr_ptr = ipv6_str[i];
						ptr_ptr++;
						count++;
						if (count % 4 == 0)
						{
							*ptr_ptr = '.';
							ptr_ptr++;
						}
					}
				}
				strncpy(ptr_ptr, "ip6.arpa", 10);
				printf(", %s", ipv6_str);
			}
			else
			{
				struct in6_addr ipv6_addr;
				memcpy(&ipv6_addr, answers[i].rdata, sizeof(struct in6_addr));

				char ipv6_str[INET6_ADDRSTRLEN];
				if (inet_ntop(AF_INET6, &ipv6_addr, ipv6_str, INET6_ADDRSTRLEN) != NULL)
				{
					printf(", %s", ipv6_str);
				}
				else
				{
					fprintf(stderr, "Failed to convert IPv6 address to string.\n");
				}
			}
		}
		else if (qtype == CNAME)
		{
			printf(", %s", answers[i].rdata);
		}
		printf("\n");
		free(answers[i].rdata);
	}

	// ------------------------------------------------------------------------------------------------------
	/* Read Authorities, , store them in auth[]  */
	for (i = 0; i < ntohs(dns->auth_count); i++)
	{
		auth[i].name = read_name(reader, buf, &stop);
		reader += stop;

		auth[i].resource = (struct R_DATA *)(reader);
		reader += sizeof(struct R_DATA);

		auth[i].rdata = read_name(reader, buf, &stop);
		reader += stop;
	}

	/* Print Authorities */
	printf("Authority section (%d)\n", ntohs(dns->auth_count));
	for (int i = 0; i < ntohs(dns->auth_count); i++)
	{
		printf("  %s", auth[i].name);

		if (auth[i].resource != NULL)
		{
			uint16_t type = ntohs(auth[i].resource->type);
			uint32_t ttl = ntohl(auth[i].resource->ttl);

			const char *type_name = get_type(type);
			printf(", %s", type_name);
			printf(", IN, ");
			printf("%u", ttl);

			if (type == 2 && auth[i].rdata != NULL)
			{
				printf(", %s", auth[i].rdata);
			}
		}
		printf("\n");
	}

	// ------------------------------------------------------------------------------------------------------
	/* Read Additional, store them in addit[]  */
	for (i = 0; i < ntohs(dns->add_count); i++)
	{
		addit[i].name = read_name(reader, buf, &stop);
		reader += stop;

		addit[i].resource = (struct R_DATA *)(reader);
		reader += sizeof(struct R_DATA);

		if (ntohs(addit[i].resource->type) == 1 || ntohs(addit[i].resource->type) == 28)
		{
			addit[i].rdata = (unsigned char *)malloc(ntohs(addit[i].resource->data_len) + 1); // Allocate space for the data + null terminator
			if (addit[i].rdata != NULL)
			{
				for (j = 0; j < ntohs(addit[i].resource->data_len); j++)
				{
					addit[i].rdata[j] = reader[j];
				}
				addit[i].rdata[ntohs(addit[i].resource->data_len)] = '\0'; // Add null terminator
			}
			reader += ntohs(addit[i].resource->data_len);
		}
		else
		{
			addit[i].rdata = read_name(reader, buf, &stop);
			reader += ntohs(addit[i].resource->data_len);
		}
	}

	/* Print Additional */
	printf("Additional section (%d)\n", ntohs(dns->add_count));
	for (int i = 0; i < ntohs(dns->add_count); i++)
	{
		printf("  %s", addit[i].name);

		if (addit[i].resource != NULL)
		{
			uint16_t type = ntohs(addit[i].resource->type);
			uint16_t _class = ntohs(addit[i].resource->_class);
			uint32_t ttl = ntohl(addit[i].resource->ttl);
			uint16_t data_len = ntohs(addit[i].resource->data_len);

			if (type == 1 && data_len == 4 && addit[i].rdata != NULL)
			{
				// Handling for IPv4 (A) records
				struct in_addr a;
				memcpy(&a.s_addr, addit[i].rdata, sizeof(a.s_addr));
				printf(", A, ");
				if (_class == 1)
				{
					printf("IN, ");
				}
				else
				{
					printf("%d, ", _class);
				}
				printf("%u, %s", ttl, inet_ntoa(a));
			}
			else if (type == 28 && data_len == 16 && addit[i].rdata != NULL)
			{
				// Handling for IPv6 (AAAA) records
				struct in6_addr a6;
				memcpy(&a6, addit[i].rdata, sizeof(struct in6_addr));
				char address[INET6_ADDRSTRLEN];
				if (inet_ntop(AF_INET6, &a6, address, INET6_ADDRSTRLEN) != NULL)
				{
					printf(", AAAA, ");
					if (_class == 1)
					{
						printf("IN, ");
					}
					else
					{
						printf("%d, ", _class);
					}
					printf("%u, %s", ttl, address);
				}
				else
				{
					fprintf(stderr, "Failed to convert IPv6 address\n");
					exit(ERROR);
				}
			}
		}
		printf("\n");
	}
}

/* https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/ */
unsigned char *read_name(unsigned char *reader, unsigned char *buffer, int *count)
{
	unsigned char *name;
	unsigned int p = 0, jumped = 0, offset;
	int i, j;

	*count = 1;
	name = (unsigned char *)malloc(256);

	name[0] = '\0';

	/* 3www6google3com Format */
	while (*reader != 0)
	{
		if (*reader >= 192)
		{
			offset = (*reader) * 256 + *(reader + 1) - 49152; // 11000000 00000000
			reader = buffer + offset - 1;
			jumped = 1;
		}
		else
		{
			name[p++] = *reader;
		}

		reader = reader + 1;

		if (jumped == 0)
		{
			*count = *count + 1;
		}
	}

	name[p] = '\0';
	if (jumped == 1)
	{
		*count = *count + 1;
	}

	/* Converting 3www6google3com0 to www.google.com */
	for (i = 0; i < (int)strlen((const char *)name); i++)
	{
		p = name[i];
		for (j = 0; j < (int)p; j++)
		{
			name[i] = name[i + 1];
			i = i + 1;
		}
		name[i] = '.';
	}
	return name;
}

/* Get the DNS servers from /etc/resolv.conf file on Linux */
/* https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/ */
void get_dns_servers()
{
	FILE *fp;
	if ((fp = fopen("/etc/resolv.conf", "r")) == NULL)
	{
		printf("Failed opening /etc/resolv.conf file \n");
	}

	strcpy(dns_servers[0], "208.67.222.222");
	strcpy(dns_servers[1], "208.67.220.220");
}

/* Converting www.google.com to 3www6google3com */
/* https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/ */
void change_to_dns(unsigned char *dns, unsigned char *host)
{
	int lock = 0, i;
	strcat((char *)host, ".");
	int lenght = strlen((char *)host);
	for (i = 0; i < lenght; i++)
	{
		if (host[i] == '.')
		{
			*dns++ = i - lock;
			for (; lock < i; lock++)
			{
				*dns++ = host[lock];
			}
			lock++;
		}
	}
	*dns++ = '\0';
}

/* Returns char from int */
const char *get_type(int qtype)
{
	switch (qtype)
	{
	case 1:
		return "A";
	case 2:
		return "NS";
	case 3:
		return "MD";
	case 4:
		return "MF";
	case 5:
		return "CNAME";
	case 6:
		return "SOA";
	case 7:
		return "MB";
	case 8:
		return "MG";
	case 9:
		return "MR";
	case 10:
		return "NULL";
	case 11:
		return "WKS";
	case 12:
		return "PTR";
	case 13:
		return "HINFO";
	case 14:
		return "MINFO";
	case 15:
		return "MX";
	case 16:
		return "TXT";
	case 28:
		return "AAAA";
	}
	return "A";
}

/* 	Reverses IPv6 address, for example, from:
	2001:4860:4860::8888
	to
	8.8.8.8.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.6.8.4.0.6.8.4.1.0.0.2.ip6.arpa.
	https://stackoverflow.com/questions/58826162/how-to-reverse-ipv6-address-for-dns-lookup-c */
const char *reverse_IPv6(const char *address)
{
	int len = strlen(address);
	char reversed_ip6_dns[INET6_ADDRSTRLEN * 4];
	char *ptr_ptr = reversed_ip6_dns;
	int overall_number = 0;
	int countforzero = 0;
	int count = 0;
	int zeroInsert = -1;
	int helpcount = 0;
	/* Gets the overall characters in IPv6*/
	for (int y = len - 1; y >= 0; y--)
	{
		overall_number++;
	}

	/* Substracts 32 and 5, so we get the negative number of the missing numbers*/
	overall_number = overall_number - 32 - 5;
	/* With abs() we get the value thats missing in positive number */
	overall_number = abs(overall_number);

	/* We round the number we got to the nearest *4, we round it downwards */
	if (overall_number % 4 != 0)
	{
		overall_number = overall_number - (overall_number % 4);
	}
	for (int i = len - 1; i >= 0; i--)
	{
		if (address[i] != ':')
		{
			helpcount++;
			/* We insert the missing zeros, to get the exact IPv6 Address */
			if (zeroInsert >= 0 && helpcount != 4)
			{
				for (int j = 0; j < overall_number; j++)
				{
					*ptr_ptr = '0';
					ptr_ptr++;
					*ptr_ptr = '.';
					ptr_ptr++;
				}
				zeroInsert = -1;
			}
			*ptr_ptr = address[i];
			ptr_ptr++;
			count++;
			if (count % 1 == 0 && i != 0)
			{
				*ptr_ptr = '.';
				ptr_ptr++;
			}
			countforzero++;
		}
		else if (i > 0 && address[i - 1] == ':')
		{
			helpcount = 0;
			zeroInsert = i - 1;
		}
		else if (address[i] == ':' && countforzero < 4 && zeroInsert == -1)
		{
			helpcount = 0;
			*ptr_ptr = '0';
			ptr_ptr++;
			*ptr_ptr = '.';
			ptr_ptr++;
		}
		if (address[i] == ':')
		{
			helpcount = 0;
			countforzero = 0;
		}
		/* We have to reset helpcount everytime, so we can keep the number of the numbers in between :: */
	}

	strncpy(ptr_ptr, ".ip6.arpa", 9);
	ptr_ptr += 9;
	*ptr_ptr = '\0';

	char *reversed = malloc(strlen(reversed_ip6_dns) + 1);
	if (reversed != NULL)
	{
		strcpy(reversed, reversed_ip6_dns);
	}

	return reversed;
}

/* 	Reverses IPv4 address, for example, from:
	93.184.216.34
	to
	34.216.184.93.in-addr.arpa.
	https://stackoverflow.com/questions/16373248/convert-ip-for-reverse-ip-lookup */
char *reverse_IPv4(char *address)
{
	int len = strlen(address);
	if (len > 0 && address[len - 1] == '.')
	{
		address[len - 1] = '\0';
	}
	struct in_addr addr;
	if (!inet_pton(AF_INET, address, &addr))
	{
		fprintf(stderr, "inet_pton() error. (reverse_IPv4)");
		exit(ERROR);
	}

	unsigned char *reversing = (unsigned char *)&addr.s_addr;

	snprintf(address, INET_ADDRSTRLEN + 16, "%d.%d.%d.%d.in-addr.arpa",
			 reversing[3], reversing[2], reversing[1], reversing[0]);
	return address;
}

/* Perform a DNS Query by Sending a Packet */
/* https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/ */
void ngethostbyname(const char *questioned_server, const char *questioned_address, int query_type, struct filter_variables *filters)
{
	/* Variable for overall packet lenght */
	int packet_lenght = 0;

	/* Basic initialization */
	unsigned char buf[BUFFERSIZE], *qname, *reader;
	int s;
	struct sockaddr_in a;

	memset(buf, 0, BUFFERSIZE);

	/* Struct response record for better storage*/
	struct RES_RECORD answers[20], auth[20], addit[20];

	/* Struct DNS header */
	struct DNS_HEADER *dns = (struct DNS_HEADER *)buf;
	dns->id = (unsigned short)htons(getpid()); /* Random number for the DNS_HEADER id */
	dns->q_count = htons(1);				   /* Only One Question */

	/* Initialize almost everything on 0 in the DNS_HEADER */
	dns->qr = 0;
	dns->aa = 0;
	dns->opcode = 0;
	dns->tc = 0;
	dns->ra = 0;
	dns->z = 0;
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;

	/* If theres arg -r */
	if (filters->recurse == true)
	{
		dns->rd = 1;
	}
	else
	{
		dns->rd = 0;
	}

	struct QUESTION *qinfo = NULL;

	/* DNS_HEADER has 12 bytes */
	packet_lenght += 12;

	/* 	https://pubs.opengroup.org/onlinepubs/009619199/getad.htm
		From getaddrinfo() online manual */
	struct addrinfo hints;
	struct addrinfo *res;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo((const char *)questioned_server, NULL, &hints, &res) != 0)
	{
		fprintf(stderr, "Invalid Addres or Server. (getaddrinfo)\n");
		exit(ERROR);
	}

	struct sockaddr *addr = res->ai_addr;

	if (addr->sa_family != AF_INET && addr->sa_family != AF_INET6)
	{
		fprintf(stderr, "Invalid Addres or Server. (sa_family)\n");
		exit(ERROR);
	}

	/* Checks, if the questioned_address is IPv6 via : */
	bool ipv6 = strchr((const char *)questioned_address, ':') != NULL;

	/* If there's -x */
	if (filters->reverse == true)
	{
		/* If its ipv6 */
		if (ipv6 == true)
		{
			questioned_address = reverse_IPv6((const char *)questioned_address);
		}
		else
		{
			uint8_t addr[16];
			/* IPv4 convertion */
			if (!inet_pton(AF_INET, questioned_address, addr))
			{
				fprintf(stderr, "inet_pton() error. (IPv4 convertion)");
				exit(ERROR);
			}
			questioned_address = reverse_IPv4((char *)questioned_address);
		}
	}

	qname = (unsigned char *)&buf[sizeof(struct DNS_HEADER)];

	change_to_dns(qname, (unsigned char*)questioned_address);

	/* Adds lenght of the qname plus 1 byte of the null terminator */
	packet_lenght += strlen((const char *)qname) + 1;
	qinfo = (struct QUESTION *)(buf + packet_lenght);

	/* 4 bytes for the qinfo->qtype and qinfo->qclass*/
	packet_lenght += 4;

	/* Type of Query */
	qinfo->qtype = htons(query_type); /* Assigns corresponding query_type */
	qinfo->qclass = htons(1);		  /* IN type */

	/* Creates socket for the corresponding address format */
	s = socket(addr->sa_family, SOCK_DGRAM, IPPROTO_UDP);
	if (s < 0)
	{
		fprintf(stderr, "socket() error\n");
		exit(ERROR);
	}

	struct sockaddr_in *server = (struct sockaddr_in *)addr;

	/* If port is enabled, assigns port parsed, otherwise 53 */
	if (filters->port_enabled == true)
	{
		server->sin_port = htons(filters->port_number);
	}
	else
	{
		server->sin_port = htons(filters->port_number);
	}

	socklen_t len;
	if (addr->sa_family == AF_INET6)
	{
		len = sizeof(struct sockaddr_in6);
	}
	else
	{
		len = sizeof(struct sockaddr_in);
	}

	// ------------------------------------------------------------------------------------------------------
	/* Sendto */
	if (sendto(s, buf, packet_lenght, 0, (struct sockaddr *)server, len) < 0)
	{
		fprintf(stderr, "sendto() error.");
		exit(ERROR);
	}

	/* Revcfrom */
	if (recvfrom(s, buf, BUFFERSIZE, 0, (struct sockaddr *)server, &len) < 0)
	{
		fprintf(stderr, "recvfrom() error");
		exit(ERROR);
	}

	/* 	https://pubs.opengroup.org/onlinepubs/009619199/getad.htm
		We have to free structures returned from the getaddrinfo() */
	freeaddrinfo(res);
	// ------------------------------------------------------------------------------------------------------
	packet_lenght = 0;

	reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1) + sizeof(struct QUESTION)];

	/* Once again, DNS_HEADER requires 12 bytes */
	packet_lenght += 12;
	recieved_info(qinfo, (unsigned char*)questioned_address, dns, reader, answers, auth, addit, buf, filters, a);
	return;
}

int main(int argc, char *argv[])
{
	unsigned char buffer[100];
	char *questioned_server;
	char *questioned_address;

	struct filter_variables *filters = NULL;

	filters = (struct filter_variables *)&buffer;
	filters->recurse = false;
	filters->reverse = false;
	filters->six = false;
	filters->server = false;
	filters->address = false;
	filters->port_enabled = false;
	filters->port_number = 53;
	filters->type = A;

	/* Enabling Parsed Arguments */
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-r") == 0)
		{
			if (filters->recurse == true)
			{
				fprintf(stderr, "-r argument for the second time.\n");
				exit(ERROR);
			}
			filters->recurse = true;
		}
		if (strcmp(argv[i], "-x") == 0)
		{
			if (filters->reverse == true)
			{
				fprintf(stderr, "-x argument for the second time.\n");
				exit(ERROR);
			}
			filters->reverse = true;
		}
		if (strcmp(argv[i], "-6") == 0)
		{
			if (filters->six == true)
			{
				fprintf(stderr, "-6 argument for the second time.\n");
				exit(ERROR);
			}
			filters->six = true;
			filters->type = AAAA;
		}
		if (strcmp(argv[i], "-s") == 0)
		{
			if (filters->server == true)
			{
				fprintf(stderr, "-s argument for the second time.\n");
				exit(ERROR);
			}
			if (argv[i + 1][0] == '-')
			{
				fprintf(stderr, "Server address is wrong.\n");
				exit(ERROR);
			}
			questioned_server = argv[i + 1];
			filters->server = true;
			i++;
			i++;
		}
		if (strcmp(argv[i], "-p") == 0)
		{
			if (filters->port_enabled == true)
			{
				fprintf(stderr, "-p argument for the second time.\n");
				exit(ERROR);
			}
			i++;
			if (atoi(argv[i]) < 0 || atoi(argv[i]) > BUFFERSIZE)
			{
				fprintf(stderr, "Number has to be 0-65535.\n");
				exit(ERROR);
			}
			filters->port_number = atoi(argv[i]);
			filters->port_enabled = true;
			continue;
		}
		if (argv[i][0] != '-')
		{
			questioned_address = argv[i];
			if (filters->address == false)
			{
				filters->address = true;
			}
			else
			{
				fprintf(stderr, "Wrong arguments.\n");
				exit(ERROR);
			}
		}
	}

	if (filters->reverse == true)
	{
		filters->type = PTR;
	}

	/* -s or Address Missing */
	if (filters->address == false || filters->server == false)
	{
		fprintf(stderr, "Obligatory argument missing. (Server or Address)\n");
		exit(ERROR);
	}

	/* Get the DNS servers from the resolv.conf file */
	get_dns_servers();
	ngethostbyname(questioned_server, questioned_address, filters->type, filters);

	/* Returns with SUCCESS*/
	return 0;
}