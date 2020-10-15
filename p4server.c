/*
	Name: Eman Shah
	Course: ECE416 | Project 4
	Date: 4/8/19
	Usage: ./p4server -p portnumber
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
#define BACKLOG 5
#define rows 3
#define cols 3

char board[3][3]; 

// Initialize board
void initializeBoard()
{
    for(int x = 0; x < rows; x++)
        for(int y = 0; y < cols;y++)
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

    if(board[row][col] == ' ')
    {
        board[row][col] = 'X'; //make move
        return true;
    }
    return false;

}

//Displays the Game Board
void displayBoard()
{
    for(int x = 0; x < rows; x++){
        for(int y = 0; y < cols; y++) {
            printf("%c|",board[x][y]);   
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
    }while(board[row][col] != ' ');

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
				if(board[i][0] == 'X')
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
				if(board[0][i] == 'X')
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
			if(board[0][0] == 'X')
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
			if(board[2][0] == 'X')
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

int main(int argc, char** argv[])
{
    struct sockaddr_in server;
    struct sockaddr_in target;
    int status, socket_fd, client_fd, num, port;
    socklen_t size;
    char buffer[1024];
    char *buff;

	if (argc == 2) {
		char help[3];
		strcpy(help, argv[1]);
		help[2] = '\0';
		switch (help[1])
		{
			case ('h') :
			{
				printf("\nGet ready to play a game of Tic Tac Toe against a server AI!\n");
				printf("Rules of the game are simple! Play as X by entering 'X' when\n");
				printf("promted and the server will play O once you are finished with your turn\n");
				printf("Connect three characters in a row, column or diagonal to win the game.\n");
				printf("Board Dimensions: 3x3\nTo start game, use: ./p4server -p <port number>\n\n");
				exit(1);
			}
		}
	}
	// Check for correct number of arguments
	if (argc != 3) {
		fprintf(stderr, "Incorrect arguments! Use '-h' flag for help!");
		exit(0);
	}
	// System call socket() creates new socket, socket_type = SOCK_STREAM for TCP
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }
	port = atoi(argv[2]);
    memset(&server, 0, sizeof(server));
    memset(&target,0,sizeof(target));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY; 
    if ((bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr )))== -1)    { //sizeof(struct sockaddr) 
        fprintf(stderr, "Binding Failure\n");
        exit(1);
    }
    if ((listen(socket_fd, BACKLOG))== -1){
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }
	int spacesLeft = 9;
	initializeBoard();
	int win = 0;
    while(1) {
        size = sizeof(struct sockaddr_in);
        if ((client_fd = accept(socket_fd, (struct sockaddr *)&target, &size))== -1 ) {
            perror("accept");
            exit(1);
        }
        printf("Server got connection from client %s\n", inet_ntoa(target.sin_addr));
        while(spacesLeft > 0) {
				if (spacesLeft < 7)
				{
					win = whoWon();
					if (win == 1)
					{
						printf("\nX Won!\n");
						displayBoard();
						exit(0);
					}
					else if (win == 2) {
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
                if ((num = recv(client_fd, board, sizeof(board), 0))== -1) {
                        perror("recv");
                        exit(1);
                }
                else if (num == 0) {
                        printf("Connection closed\n");
                        break;
                }
                //buffer[num] = '\0';
                computerPlayerMove();
				displayBoard();
				spacesLeft--;
                if ((send(client_fd,board, sizeof(board),0))== -1) 
                {
                     fprintf(stderr, "Failure Sending Message\n");
                     close(client_fd);
                     break;
                }
        } //End of Inner While
        close(client_fd);
    } //Outer While
    close(socket_fd);
    return 0;
} //End of main