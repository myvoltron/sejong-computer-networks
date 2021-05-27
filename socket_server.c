#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_message(char *str)    //문자열로 받은 에러메시지 str을 받고 프로그램 종료
{
	printf("%s\n", str);
	exit(1);
}

int main(int argc, char* argv[]) 
{
	char send_message[100], sent_message[100];
	char clnt_name[100];
	char *my_ip_addr = "192.168.200.201";

	//각 소켓 디스크립터 값을 저장할 변수 
	int serv_sock;
	int clnt_sock;
	
	//각 소켓의 주소를 저장할 구조체 변수
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	int str_len = 0;

	if(argc != 3) //argv에 경로, 프로토콜, 포트넘버 3가지 값이 들어와야만 함. 
	{
		printf("올바른 사용법! : %s which_protocol?  PORT_NUM\n", argv[0]);
		return 1;
	}
	
	printf("starting your chatting!!------------------------\n\n");
	//tcp case
	if(!strcmp(argv[1], "tcp") || !strcmp(argv[1], "TCP"))
	{
		//serv_sock 생성 
		serv_sock = socket(PF_INET, SOCK_STREAM, 0); //PF_INET -> IPv4 SOCK_STREAM -> tcp protocol 
		if(serv_sock == -1)
			error_message("socket() error");

		//주소 초기화
		memset(&serv_addr, 0, sizeof(serv_addr));               //초기화 
		serv_addr.sin_family = AF_INET;			             	//주소체계 IPv4 지정
		serv_addr.sin_addr.s_addr = inet_addr(my_ip_addr);      //ip주소 저장
		serv_addr.sin_port = htons(atoi(argv[2])); 				//port num 저장

		//소켓에 주소 할당 
		if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
			error_message("bind() error");
		printf("Server : create tcp socket(%s : %s) done\n", my_ip_addr, argv[2]);

		//연결요청 대기상태 
		if(listen(serv_sock, 1) == -1)
			error_message("listen() error");
		printf("Server : waiting connection request\n");

		//연결 허용
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);  //accept를 통해 연결요청을 한 클라이언트측의 소켓과 연결될 수 있다.
		if(clnt_sock == -1)
			error_message("accept() error");
		printf("Server : %s(%d) client connect\n", inet_ntoa(clnt_addr.sin_addr), clnt_addr.sin_port);

		//클라이언트의 name 받기 
		str_len = read(clnt_sock, clnt_name, sizeof(clnt_name)-1);
		if(str_len == -1)
			error_message("read() error");
		clnt_name[str_len] = '\0';
		printf("%s(%d) %s님이 입장하셨습니다. (나가기 q or quit)\n\n", inet_ntoa(clnt_addr.sin_addr), clnt_addr.sin_port, clnt_name); //클라이언트의 name

		//채팅 시작 
		while(1)
		{
			//message 수신
			str_len = 0;
			while(str_len <= 0) //read를 받아서 str_len에 변화가 생길 때 까지 대기 
			{
				str_len = read(clnt_sock, sent_message, 100);			 	   
				if(str_len == -1)
					error_message("read() error");
			}
			sent_message[str_len] = '\0';
			if(!strcmp(sent_message, "q") || !strcmp(sent_message, "quit"))
			{
				printf("클라이언트로부터 종료메시지를 받았습니다. 채팅프로그램이 종료됩니다.\n");
				break;
			}
			printf("\n[%s] : %s\n\n", clnt_name, sent_message);

			//message 송신
			printf("input the message : ");
			fgets(send_message, 100, stdin);
			send_message[strlen(send_message) - 1] = '\0';
			if(!strcmp(send_message, "q") || !strcmp(send_message, "quit"))
			{
				write(clnt_sock, send_message, sizeof(send_message));
				printf("채팅 프로그램이 종료됩니다.\n");
				break;
			}
			write(clnt_sock, send_message, sizeof(send_message));
		}

		//종료 
		close(serv_sock);
		close(clnt_sock);
	}
	//udp case
	else if(!strcmp(argv[1], "udp") || !strcmp(argv[1], "UDP"))
	{
		//소켓생성 
		serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
		if(serv_sock == -1)
			error_message("socket() error");

		//주소 초기화 
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(my_ip_addr);
		serv_addr.sin_port = htons(atoi(argv[2]));

		//소켓에 주소 할당
		if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
			error_message("bind() error");
		printf("Server : create udp socket(%s : %s) done\n", my_ip_addr, argv[2]);
		
		//클라이언트의 이름을 받음
		clnt_addr_size = sizeof(clnt_addr);
		str_len = recvfrom(serv_sock, clnt_name, 100, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if(str_len == -1)
					error_message("recvfrom() error");
		clnt_name[str_len] = '\0';
		printf("%s(%d) %s님이 입장하셨습니다. (나가기 q or quit)\n", inet_ntoa(clnt_addr.sin_addr), clnt_addr.sin_port, clnt_name); //클라이언트의 ipaddress, port number, name

		//채팅시작
		while(1)
		{
			//수신
			str_len = 0;
			while(str_len <= 0)
			{
				str_len = recvfrom(serv_sock, sent_message, 100, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
				if(str_len == -1)
					error_message("recvfrom() error");
			}
			sent_message[str_len] = '\0';
			if(!strcmp(sent_message, "q") || !strcmp(sent_message, "quit"))
			{
				printf("클라이언트로부터 종료메시지를 받았습니다. 채팅프로그램이 종료됩니다.\n");
				break;
			}
			printf("\n[%s] : %s\n\n", clnt_name, sent_message);
		
			//송신
			printf("input the message : ");
			fgets(send_message, 100, stdin);
			send_message[strlen(send_message) - 1] = '\0';
			if(!strcmp(send_message, "q") || !strcmp(send_message, "quit"))
			{
				sendto(serv_sock, send_message, strlen(send_message), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
				printf("채팅 프로그램이 종료됩니다.\n");
				break;
			}
			sendto(serv_sock, send_message, strlen(send_message), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
		}
		close(serv_sock);
	}
	
	//tcp와 udp가 아닌 것을 입력했을 때 
	else 
		error_message("please input tcp or udp protocol!!");
	//final
	printf("이용해 주셔서 감사합니다.\n");
	return 0;
}
