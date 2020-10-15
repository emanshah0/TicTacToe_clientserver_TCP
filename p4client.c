/*
	Name: Eman Shah
	Course: ECE416 | Project 4
	Date: 4/8/19
	Usage: ./p4client -s hostname -p portnumber
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>

#define h_addr h_addr_list[0]
#define MAXSIZE 1024
#define rows 3
#define cols 3

char board[3][3];

// Initialize board
void initializeBoard()
{
	for (int x = 0; x < rows; x++)
		for (int y = 0; y < cols; y++)
			board[x][y] = ' ';
}

// scan for player moves
bool humanPlayerMove()
{
	//Get Value from player
	int row, col;
	printf("Enter Row (0 to 2): ");
	scanf("%d", &row);
	printf("Enter Column (0 to 2): ");
	scanf("%d", &col);

	if (board[row][col] == ' ')
	{
		board[row][col] = 'X'; //make move
		return true;
	}
	return false;
}

//Displays the Game Board
void displayBoard()
{
	for (int x = 0; x < rows; x++)
	{
		for (int y = 0; y < cols; y++)
		{
			printf("%c|", board[x][y]);
		}
		printf("\n");
	}
	printf("-----------\n");
}

// random generated computer move
void computerPlayerMove()
{
	srand(time(NULL)); //random seed
	int row, col;
	do
	{
		row = rand() % 3;
		col = rand() % 3;
	} while (board[row][col] != ' ');

	board[row][col] = 'O';
}

int whoWon()
{
	// check for straight row win
	for (int i = 0; i < rows; i++)
	{
		if (board[i][0] == board[i][1] &&
			board[i][1] == board[i][2])
		{
			if (board[i][0] == 'X')
			{
				return 1;
			}
			else
			{
				return 2;
			}
		}
	}

	// check for straight column win
	for (int i = 0; i < cols; i++)
	{
		// determine if 'somebody' won
		if (board[0][i] == board[1][i] &&
			board[1][i] == board[2][i])
		{
			// determine 'who' won
			if (board[0][i] == 'X')
			{
				return 1;
			}
			else
			{
				return 2;
			}
		}
	}

	// check for forward diagonal win
	if (board[0][0] == board[1][1] &&
		board[1][1] == board[2][2])
	{
		if (board[0][0] == 'X')
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
	// check for backwards diagonal win
	if (board[2][0] == board[1][1] &&
		board[1][1] == board[0][2])
	{
		if (board[2][0] == 'X')
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
	return 0;
}

// ===================================
//	TCP connection code below this
// ===================================

int main(int argc, char *argv[])
{
	struct sockaddr_in server_info;
	struct hostent *host;
	int socket_fd, num, port;
	char buffer[1024];
	char buff[1024];
	if (argc == 2)
	{
		char help[3];
		strcpy(help, argv[1]);
		help[2] = '\0';
		switch (help[1])
		{
		case ('h'):
		{
			printf("\nGet ready to play a game of Tic Tac Toe against a server AI!\n");
			printf("Rules of the game are simple! Play as X by entering 'X' when\n");
			printf("promted and the server will play O once you are finished with your turn\n");
			printf("Connect three characters in a row, column or diagonal to win the game.\n");
			printf("Board Dimensions: 3x3\nTo start game, use: ./p4client -s <host name> -p <port number>\n\n");
			exit(1);
		}
		}
	}
	if (argc != 5)
	{
		fprintf(stderr, "Incorrect arguments! use '-h' flag for help!\n");
		exit(1);
	}
	port = atoi(argv[4]);
	if ((host = gethostbyname(argv[2])) == NULL)
	{
		fprintf(stderr, "Cannot get host name\n");
		exit(1);
	}

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Socket Failure!!\n");
		exit(1);
	}
	// allocate memory for sockaddr_in structure
	memset(&server_info, 0, sizeof(server_info));
	// address family always set to syymbolic constant AF_INET
	server_info.sin_family = AF_INET;
	// copy port into sin_port field - from host byte order to network byte order
	server_info.sin_port = htons(port);
	// host ip address - set to server ip address: INADDR_ANY = IP address of the running machine
	server_info.sin_addr = *((struct in_addr *)host->h_addr);
	if (connect(socket_fd, (struct sockaddr *)&server_info, sizeof(struct sockaddr)) < 0)
	{
		//fprintf(stderr, "Connection Failure\n");
		perror("connect");
		exit(1);
	}
	//memset(buffer, 0 , sizeof(buffer));
	int spacesLeft = 9;
	initializeBoard();
	char turn = 'X';
	int win = 0;

	while (spacesLeft > 0)
	{
		if (spacesLeft < 7)
		{
			win = whoWon();
			if (win == 1)
			{
				printf("\nX Won!\n");
				displayBoard();
				exit(0);
			}
			else if (win == 2)
			{
				printf("\nO Won!\n");
				displayBoard();
				exit(0);
			}
			else if (win == 0 && spacesLeft < 4)
			{
				printf("\nDRAW!\n");
				displayBoard();
				exit(0);
			}
		}

		if (turn == 'X')
		{
			bool valid = humanPlayerMove();
			if (valid)
			{
				turn = 'O';
				spacesLeft--;
			}
			displayBoard();
		}

		if ((send(socket_fd, board, sizeof(board), 0)) == -1)
		{
			fprintf(stderr, "Failure Sending Message\n");
			close(socket_fd);
			exit(1);
		}
		else
		{
			//printf("Client:Message being sent: %s\n", buffer);
			num = recv(socket_fd, board, sizeof(board), 0);
			turn = 'X';
			displayBoard();
			if (num <= 0)
			{
				printf("Either Connection Closed or Error\n");
				//Break from the While
				break;
			}
			//buff[num] = '\0';
			//printf("Client:Message Received From Server -  %s\n", buffer);
			//continue;
		}
	}
	close(socket_fd);

} //End of main