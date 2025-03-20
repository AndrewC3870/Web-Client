#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include "helpers.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

//defines for type of requests
#define LOGIN 2
#define REGISTER 1
#define ENTER_LIBRARY 3
#define GET_ALL_BOOKS 4
#define ADD_BOOK 5
#define GET_BOOK 6
#define DELETE_BOOK 7
#define LOGOUT 8

//function for sending the request to the server
int open_connection_to_server(string request) {
	char *payload_to_send = (char*) request.c_str();
	string ip = "34.246.184.49";
	int server = open_connection((char*) ip.c_str(), 8080, AF_INET, SOCK_STREAM, 0);
	send_to_server(server, payload_to_send);
	return server;
}

//function for generating the json payload according to the type of request
json generate_payload(int type, string username, string password, string id, string title, string author,
	string publisher, string genre, string page_count) {
	json payload;
	switch (type) {
        //login and register payload type
		case REGISTER:
			payload["username"] = (char*) username.c_str();
			payload["password"] = (char*) password.c_str();
			break;
        //add book payload type
		case ADD_BOOK:
			payload["title"] = title;
			payload["author"] = author;
			payload["genre"] = genre;
			payload["page_count"] = page_count;
			payload["publisher"] = publisher;
			break;
        //get book and delete payload
		case GET_BOOK:
			payload["id"] = id;
			break;
		default:
			break;
	}

	return payload;
}

//function to verify if an username or password input contains spaces
int verify_input(string user_input) {
	int string_length = user_input.length();
	if (string_length==0) {
		return -1;
	} 
	for (int i = 0; i < string_length; i++) {
		if (isspace(user_input[i])) {
			return -1;
		}
	}


	return 0;
}

//function to get and generate json for username and password and verify integrity
json get_username_password() {
	string username;
	string password;
	json payload;

    //get inputs
	cout << "username=";
	getline(cin, username);

	cout << "password=";
	getline(cin, password);

    //verify if inputs are valid 
	int username_good = verify_input(username);
	if (username_good == -1) {
		cout << "ERROR You should enter a valid username!\n";
		return payload;
	}

	int password_good = verify_input(password);
	if (password_good == -1) {
		cout << "ERROR You should enter a valid password!\n";
		return payload;
	}

	payload = generate_payload(REGISTER, username, password, "", "", "", "", "", "");
	return payload;
}

//function to generate request accordingly
string generate_request(int type, json payload, string login_cookie, string library_cookie, string id) {
	string to_send;
	switch (type) {
        //generate request for register
		case REGISTER:
			to_send = "POST /api/v1/tema/auth/register HTTP/1.1\r\nHost: 34.246.184.49\r\n";
			to_send += "Content-Type: application/json\r\nContent-Length: ";
			to_send += to_string(strlen((char*) payload.dump().c_str()));
			to_send += "\r\n\r\n";
			to_send += payload.dump();
			break;
        //generate request for login
		case LOGIN:
			to_send = "POST /api/v1/tema/auth/login HTTP/1.1\r\nHost: 34.246.184.49\r\n";
			to_send += "Content-Type: application/json\r\nContent-Length: ";
			to_send += to_string(strlen((char*) payload.dump().c_str()));
			to_send += "\r\n\r\n";
			to_send += payload.dump();
			break;
        //generate request for enter_library
		case ENTER_LIBRARY:
			to_send = "GET /api/v1/tema/library/access HTTP/1.1\r\nHost: 34.246.184.49\r\n";
			to_send += "Cookie: " + login_cookie + "\r\n\r\n";
			break;
        //generate request for get_all_books
		case GET_ALL_BOOKS:
			to_send = "GET /api/v1/tema/library/books HTTP/1.1\r\nHost: 34.246.184.49\r\n";
			to_send += "Authorization: Bearer " + library_cookie + "\r\n\r\n";
			break;
        //generate requests for add_book
		case ADD_BOOK:
			to_send = "POST /api/v1/tema/library/books HTTP/1.1\r\nHost: 34.246.184.49\r\n";
			to_send += "Authorization: Bearer ";
			to_send += library_cookie;
			to_send += "\r\nContent-Type: application/json\r\nContent-Length: ";
			to_send += to_string(strlen((char*) payload.dump().c_str()));
			to_send += "\r\n\r\n";
			to_send += payload.dump();
			break;
        //generate request for get_book
		case GET_BOOK:
			to_send = "GET /api/v1/tema/library/books/";
			to_send += id;
			to_send += " HTTP/1.1\r\nHost: 34.246.184.49\r\n";
			to_send += "Authorization: Bearer ";
			to_send += library_cookie + "\r\n\r\n";
			break;
        //generate request for delete_book
		case DELETE_BOOK:
			to_send = "DELETE /api/v1/tema/library/books/";
			to_send += id;
			to_send += " HTTP/1.1\r\nHost: 34.246.184.49\r\n";
			to_send += "Authorization: Bearer ";
			to_send += library_cookie + "\r\n\r\n";
			break;
        //generate_request for logout
		case LOGOUT:
			to_send = "GET /api/v1/tema/auth/logout HTTP/1.1\r\nHost: 34.246.184.49\r\n";
			to_send += "Cookie: " + login_cookie + "\r\n\r\n";
			break;
		default:
			break;
	}

	return to_send;
}

//verify if server returned an error
int verify_response(char *response) {
	int error = 0;
	json json_response;
	char *extracted_json = basic_extract_json_response(response);
    //if server response is null there is no error
	if (extracted_json == NULL) {
		return error;
	}
    //if server response returns error return error
	json_response = json::parse((string) extracted_json);
	if (json_response.contains("error")) {
		error = 1;
	}

	return error;
}

//function to verify if id and page_count is a number
int invalid_page_count(string str) {
	for (char c: str) {
		if (!isdigit(c)) {
			return 1;
		}
	}

	return 0;
}

int invalid_name(string str) {
	for (char c: str) {
		if (isdigit(c) ) {
			return 1;
		}
	}
	return 0;
}

//function to parse input for add_book , get_book and delete_book commands
json parse_book_input(int type) {
	string title;
	string author;
	string publisher;
	string genre;
	string page_count;
	json payload;
	string id;
	int wrong_inputs = 0;
	switch (type) {
        //get inputs for add_book
		case ADD_BOOK:
			cout << "title=";
			getline(cin, title);
			if (title == "") {
				wrong_inputs = 1;
			}

			cout << "author=";
			getline(cin, author);
			if (author == "") {
				wrong_inputs = 1;
			}

			cout << "publisher=";
			getline(cin, publisher);
			if (publisher == "") {
				wrong_inputs = 1;
			}

			cout << "genre=";
			getline(cin, genre);
			// genre can't contain a number also verified
			if (genre == "" || invalid_name(genre) == 1){
				wrong_inputs = 1;
			}

			cout << "page_count=";
			getline(cin, page_count);
			if (page_count == "" || invalid_page_count(page_count) == 1) {
				wrong_inputs = 1;
			}

            //if invalid inputs raise error
			if (wrong_inputs == 1) {
				cout << "ERROR Please consider valid book descriptions\n";
				return payload;
			}

            //generate json accordingly
			payload = generate_payload(5, "", "", "", title, author, publisher, genre, page_count);
			return payload;
			break;
        //get delete_book and get_book inputs
		case GET_BOOK:
			cout << "id=";
			getline(cin, id);

			if ((invalid_page_count(id) == 1) || (id == ""))
			{
				cout << "ERROR Please enter a number for id!\n";
				return payload;
			}
            //generate json accordingly
			payload = generate_payload(6, "", "", id, "", "", "", "", "");
			return payload;
			break;
		default:
			break;
	}

	return payload;
}

//utility function for parsing servers response into json
json convert_chars_to_json(char *response) {
	string char_to_string = response;
	json json_from_string;
	json_from_string = json::parse(char_to_string);
	return json_from_string;
}

//register function
void get_rs_register_db(string login_token) {
    if (login_token != "" && login_token != "no access") {
        cout<<"ERROR  you are already logged in\n";
        return;
    }
	json payload;
	payload = get_username_password();

    //verify integrity of payload
	if (strlen((char*) payload.dump().c_str()) <= 5) {
		return;
	}

    //send request to server
	string request = generate_request(REGISTER, payload, "", "", "");
	int server = open_connection_to_server(request);

    //recieve response from server
	char *response_server;
	response_server = receive_from_server(server);

    //verify response 
	int error = verify_response(response_server);
	if (error == 0) {
		cout << "SUCCESS - register\n";
		free(response_server);
		return;
	}
	else {
		cout << "ERROR - register - the username is taken\n";
		free(response_server);
		return;
	}
}


//login function that returns login token in case of succes
string get_rs_login_db(string login_token) {
    if (login_token!=""&&login_token != "no access") {
        cout<<"ERROR  you are already logged in\n";
        return login_token;
    }
	json payload;
	payload = get_username_password();

    //verify integrity
	if (strlen((char*) payload.dump().c_str()) <= 5) {
		return "no access";
	}

    //generate request and send to server
	string request = generate_request(LOGIN, payload, "", "", "");
	int server = open_connection_to_server(request);

    //recieve response from server
	char *response_server;
	response_server = receive_from_server(server);

	int error = verify_response(response_server);

	if (error == 0) {
        //if response is success parse the login cookie
		char *cookie = strstr(response_server, "Set-Cookie");
		string cookie_session = cookie;
		string cookie_session_login =
			cookie_session.substr(0, cookie_session.find("\r\n"))
			.substr(cookie_session.find(" ") + 1);
		cout << "SUCCESS - login\n";
		free(response_server);
		return cookie_session_login;
	}
	else {
		cout << "ERROR - login - wrong username or password\n";
		free(response_server);
		return "no access";
	}
}

//enter_library function that returns token if successfull
string get_rs_enter_library(string login_cookie, string library_token) {
    //verify if user is logged in
    if(library_token!=""&&library_token!="no access library"&&login_cookie=="") {
        cout<<"ERROR you have already entered the library\n";
        return library_token;
    }
	if (login_cookie == "no access"||login_cookie=="") {
		cout << "ERROR - enter library not permitted\n";
		return "no access library";
	}

    //generate request and send to server
	json payload;
	string request = generate_request(ENTER_LIBRARY, payload, login_cookie, "", "");
	int server = open_connection_to_server(request);

    //recieve response
	char *response_server;
	response_server = receive_from_server(server);
	int error = verify_response(response_server);
	if (error == 0) {
        //if successfull get the token from json
		char *json_server = basic_extract_json_response(response_server);
		json json_enter_l = convert_chars_to_json(json_server);
		free(response_server);
		cout << "SUCCES library\n";
		return json_enter_l["token"];
	}
	else {
		cout << "ERROR - library\n";
		free(response_server);
		return "no access library";
	}
}

//get_all_books_function
void get_rs_all_books(string login_token, string library_token) {
    //verify if user is logged in and has access to library
	if (login_token == "no access"||login_token=="") {
		cout << "ERROR - you have to login in order to perform this command\n";
		return;
	}
	else if (library_token == "no access library"||library_token=="") {
		cout << "ERROR - you did not enter library\n";
        return;
	}

    //generate and send request to server
	json payload;
	string request = generate_request(GET_ALL_BOOKS, payload, "", library_token, "");
	int server = open_connection_to_server(request);

    //recieve response 
	char *response_server;
	response_server = receive_from_server(server);
	char *find_array = strchr(response_server, '[');
	if (find_array != NULL) {
        //if successfull display all books
		string json_parse = response_server;
		json all_books = convert_chars_to_json(find_array);
		cout << all_books.dump() << "\n";
		return;
	}
}


//add_book function 
void get_rs_add_book_db(string login_token, string library_token) {
    //verify if user is logged in and has access to library
	if(login_token == "no access"||login_token=="") {
		cout << "ERROR - you have to login in order to perform this command\n";
		return;
	}
	else if (library_token == "no access library"||library_token=="") {
		cout << "ERROR - you did not enter library\n";
        return;
	}

    //verify input integrity
	json payload = parse_book_input(ADD_BOOK);
	if (strlen((char*) payload.dump().c_str()) <= 5) {
		return;
	}

    //generate and send request to server
	string request = generate_request(ADD_BOOK, payload, "", library_token, "");
	int server = open_connection_to_server(request);

    //recieve response 
	char *response_server;
	response_server = receive_from_server(server);
	int error = verify_response(response_server);
	if (error == 0) {
		cout << "SUCCES - added_book\n";
	}
	else {
		cout << "ERROR - add book\n";
	}

	free(response_server);
	return;
}

void get_rs_get_book_db(string login_token, string library_token) {
    //verify if user is logged in and has access to library
	if (login_token == "no access"||login_token=="") {
		cout << "ERROR - you have to login in order to perform this command\n";
		return;
	}
	else if (library_token == "no access library"||library_token=="") {
		cout << "ERROR - you did not enter library\n";
		return;
	}

    //verify input integrity
	json payload = parse_book_input(GET_BOOK);
	if (strlen((char*) payload.dump().c_str()) <= 5) {
		return;
	}

    //generate and send request to server
	string id;
	id = payload["id"];
	string request = generate_request(GET_BOOK, payload, "", library_token, id);
	int server = open_connection_to_server(request);

    //recieve response 
	char *response_server;
	response_server = receive_from_server(server);
	int error = verify_response(response_server);
	if (error == 0) {
        //if successfull display the book 
		char *json_from_server = basic_extract_json_response(response_server);
		json book_content = convert_chars_to_json(json_from_server);
		cout << book_content.dump() << "\n";
		cout << "SUCCES - get book\n";
	}
	else {
		cout << "ERROR - get book\n";
	}

	free(response_server);
	return;
}

void get_rs_delete_book(string login_token, string library_token) {
    //verify if user is logged in and has access to library
	if (login_token == "no access"||login_token=="") {
		cout << "ERROR - you have to login in order to perform this command\n";
		return;
	}
	else if (library_token == "no access library"||library_token=="") {
		cout << "ERROR - you did not enter library\n";
		return;
	}

    //verify input integrity 
	json payload = parse_book_input(GET_BOOK);
	if (strlen((char*) payload.dump().c_str()) <= 5) {
		return;
	}

    //generate and send request to server
	string id;
	id = payload["id"];
	string request = generate_request(DELETE_BOOK, payload, "", library_token, id);
	int server = open_connection_to_server(request);

    //recieve response 
	char *response_server;
	response_server = receive_from_server(server);
	int error = verify_response(response_server);
	if (error == 0) {
		cout << "SUCCES - delete book\n";
	}
	else {
		cout << "ERROR - delete book\n";
	}

	free(response_server);
	return;
}

string get_rs_logout_db(string login_token) {
    //verify if user is logged in 
	if (login_token == "no access" || login_token == "") {
		cout << "ERROR - you have to login in order to perform this command\n";
		return login_token;
	}

    //generate and send request to server
	json payload;
	string request = generate_request(LOGOUT, payload, login_token, "", "");
	int server = open_connection_to_server(request);

    //recieve response 
	char *response_server;
	response_server = receive_from_server(server);
	int error = verify_response(response_server);
	if (error == 0) {
		cout << "SUCCES - logout\n";
        free(response_server);
	    return "";
	}
	else {
		cout << "ERROR - logout\n";
        free(response_server);
	    return login_token;
	}

	

}

//function to clear library token
string get_rs_logout_db_lib(string login_token, string library_cookie) {
    if (library_cookie!=""&&library_cookie!="no access library"&&login_token=="") {
		return "";
    }
    return library_cookie;
}

int main() {
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	string input;
	int exit = 1;
	string login_cookie="";
	string library_cookie="";

    //main logic loop 
	while (exit == 1) {
		getline(cin, input);
		if (input == "exit") {
			exit = 0;
		}
		else if (input == "register") {
			get_rs_register_db(login_cookie);
		}
		else if (input == "login") {
			login_cookie = get_rs_login_db(login_cookie);
		}
		else if (input == "enter_library") {
			library_cookie = get_rs_enter_library(login_cookie, library_cookie);
		}
		else if (input == "add_book") {
			get_rs_add_book_db(login_cookie, library_cookie);
		}
		else if (input == "get_books") {
			get_rs_all_books(login_cookie, library_cookie);
		}
		else if (input == "get_book") {
			get_rs_get_book_db(login_cookie, library_cookie);
		}
		else if (input == "delete_book") {
			get_rs_delete_book(login_cookie, library_cookie);
		}
		else if (input == "logout") {
			login_cookie = get_rs_logout_db(login_cookie);
            library_cookie=get_rs_logout_db_lib(login_cookie, library_cookie);
		}
		else {
			cout << "ERROR - Please input a valid command\n";
		}
	}

	return 0;
}