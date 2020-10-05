#include <iostream>

#include <stdio>
#include <stdlib>
#include <unistd>
#include <string>
#include <sys/types>
#include <sys/socket>
#include <netinet/in>
#include <netdb>
#include <thread>
#include <async>

#define PORT 4000

using namespace std;

void clientSetup(struct hostent *server) {

	int client_socket;
	struct sockaddr_in serv_addr;

	// Server name check
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    // Socket Opening
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");

}

class CommunicationChannel {
	public:
		string m_username;
		string m_groupname;
		string m_server_ip_address;
		int m_port;

		CommunicationChannel(string username, string)
}

class Interface {
	public:
		string 				m_username;
		string 				m_groupname;
		int 				m_client_socket;
		int 				m_operation_status;
		struct sockaddr_in 	m_serv_addr;
		struct hostent 		*m_server;


		Interface(string username, string groupname, struct hostent *server, int port) {

			setUsername(username);
			setGroupname(groupname);

			// Socket Opening
			if ((m_client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        		printf("ERROR opening socket\n");
        		exit(0);
			}

        	// Setting Parameters
        	m_serv_addr.sin_family = AF_INET;
			m_serv_addr.sin_port = htons(PORT);
			m_serv_addr.sin_addr = *((struct in_addr *)m_server->h_addr);
			bzero(&(m_serv_addr.sin_zero), 8);

			// Connection
			if (connect(m_client_socket,(struct sockaddr *) &m_serv_addr,sizeof(m_serv_addr)) < 0) {
        		printf("ERROR connecting\n");
        		exit(0);
			}
			showMessage("Voce entrou no grupo.");

        	m_operation_status = write()

		}

		showMessage(string message) {
			cout << message << endl;
		}

		// Getters:
		getUsername() {
			return m_username;
		}

		getGroupname() {
			return m_groupname;
		}

		// Setters:
		setUsername(string username) {
			m_username = username;
		}

		setGroupname(string groupname) {
			m_groupame = groupname;
		}

		setChannel(string username, string groupname, string server_ip_address, int port) {
			CommunicationChannel
		}
}

int main(int argc, char *argv[]) {


	if(argc < 5) {
		cout << "Run the application with the proper arguments:" << endl;
		cout << "./app_client <username> <groupname> <server_ip_address> <port>" << endl;
		exit(0);
	}

	string username = argv[1];
	string groupname = argv[2];
	struct hostent *server = gethostbyname(argv[3]);
	int port = argv[4];

	// Server name check
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

	thread thr1(Interface(), username, groupname, server, port);

	thr1.join();

	return 0;
}
