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

	int sock;
	struct sockaddr_in serv_addr; //소켓과 소켓 주소를 저장할 변수들 
	int str_len;   

	if(argc != 5)
	{
		printf("올바른 사용법! : %s which_protocol? IP PORT_NUM NAME", argv[0]);
		return 1;
	}

	printf("starting your chatting!!---------------------\n\n");
	//tcp case
	if(!strcmp(argv[1], "tcp") || !strcmp(argv[1], "TCP"))
	{
		//소켓 생성
		sock = socket(PF_INET, SOCK_STREAM, 0);
		if(sock == -1)
			error_message("socket() error");

		//주소 초기화
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
		serv_addr.sin_port = htons(atoi(argv[3]));

		//연결요청 
		if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
			error_message("connect() error");

		write(sock, argv[4], strlen(argv[4])); //서버에게 클라이언트의 이름을 보냄
		printf("채팅을 시작하세요!! (나가기 q or quit)\n\n");
		//채팅 시작 
		while(1)
		{
			//송신
			printf("input the message : ");
			fgets(send_message, 100, stdin);
			send_message[strlen(send_message) - 1] = '\0';
			if(!strcmp(send_message, "q") || !strcmp(send_message, "quit"))
			{
				write(sock, send_message, sizeof(send_message));
				printf("채팅 프로그램이 종료됩니다.\n");
				break;
			}
			write(sock, send_message, sizeof(send_message)); //서버에게 메시지 보냄	

			//수신
			str_len = 0;
			while(str_len <= 0)
			{
				str_len = read(sock, sent_message, 100); //메시지 받음
				if(str_len == -1)
					error_message("read() error");
			}	
			sent_message[str_len] = '\0';
			if(!strcmp(sent_message, "q") || !strcmp(sent_message, "quit"))
			{
				printf("서버로 부터 종료메시지를 받았습니다. 채팅프로그램이 종료됩니다.\n");
				break;
			}
			printf("\nServer : %s\n\n", sent_message); //받은 메시지 출력
		}

		close(sock);
	}

	//udp case
	else if(!strcmp(argv[1], "udp") || !strcmp(argv[1], "UDP"))
	{
		socklen_t addr_size;
		struct sockaddr_in from_addr;

		//소켓 생성 
		sock = socket(PF_INET, SOCK_DGRAM, 0);
		if(sock == -1)
			error_message("socket() error");

		//주소 초기화 
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
		serv_addr.sin_port = htons(atoi(argv[3]));	

		sendto(sock, argv[4], strlen(argv[4]), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); //서버에게 클라이언트의 이름을 보냄		
		printf("채팅을 시작하세요!! (나가기 q or quit)\n\n");
		//채팅 시작 
		while(1)
		{
			//송신
			printf("input the message : ");
			fgets(send_message, 100, stdin);
			send_message[strlen(send_message) - 1] = '\0';
			if(!strcmp(send_message, "q") || !strcmp(send_message, "quit"))
			{
				sendto(sock, send_message, strlen(send_message), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); //서버에게 메시지 보냄	
				printf("채팅 프로그램이 종료됩니다.\n");
				break;
			}
			sendto(sock, send_message, strlen(send_message), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); //서버에게 메시지 보냄	

			//수신
			str_len = 0;
			while(str_len <= 0)
			{
				str_len = recvfrom(sock, sent_message, 100, 0, (struct sockaddr*)&from_addr, &addr_size); //메시지 받음
				if(str_len == -1)
					error_message("recvfrom() error");
			}	
			sent_message[str_len] = '\0';
			if(!strcmp(sent_message, "q") || !strcmp(sent_message, "quit"))
			{
				printf("서버로 부터 종료메시지를 받았습니다. 채팅프로그램이 종료됩니다.\n");
				break;
			}
			printf("\nServer : %s\n\n", sent_message); //받은 메시지 출력
		}

		close(sock);
	}
	//tcp와 udp가 아닌 것을 입력했을 때
	else
		error_message("please input tcp or udp protocol!!");

	printf("이용해 주셔서 감사합니다.\n");
	return 0;
}
