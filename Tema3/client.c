#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson/parson.h"

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    char **cookies = calloc(1,sizeof(char*));
    cookies[0] = NULL;
    int cookies_count = 0;
    char *JWT = NULL;
    int sockfd;
    char *line = calloc(LINELEN,sizeof(char));

    char *json_data;
    JSON_Value *root_val;
    JSON_Object *root_obj;

    while(1) {
        fgets(line,LINELEN,stdin);
        char *command = strtok(line," \n");

        if(command == NULL) {
            continue;
        }

        // Register comand
        if(!strcmp(command,"register")) {
            // Open the connection to server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // Read the username & pasword for the user
            char *username = calloc(150,sizeof(char));
            char *password = calloc(150,sizeof(char));

            printf("username=");
            fgets(username,150,stdin);
            if (username[strlen(username) - 1] == '\n'){
                username[strlen(username) - 1] = '\0';
            }

            printf("password=");
            fgets(password,150,stdin);
            if (password[strlen(password) - 1] == '\n')
                password[strlen(password) - 1] = '\0';

            // Initialize json objects
            root_val = json_value_init_object();
            root_obj = json_value_get_object(root_val);

            json_object_set_string(root_obj, "username", username);
            json_object_set_string(root_obj, "password", password);
            json_data = json_serialize_to_string(root_val);

            // Send the request to the server:
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", "application/json", &json_data, 1, NULL, 0, NULL);
            send_to_server(sockfd,message);

            // Receive the response from the server
            response = receive_from_server(sockfd);

            if(response == NULL) {
                printf("No response from the server\n");
                continue;
            }

            // Parse the response to verify if the account has been created
            char *buff = strtok(response, "\n");

            if(!strncmp(buff,"HTTP/1.1 201 Created",18)) {
                printf("201 - OK - The account has been created.\n");
            }
            else
            {
                printf("400 - The username %s is taken\n",username);   
            }

            json_free_serialized_string(json_data);
            json_value_free(root_val);
            free(username);
            free(password);
            close(sockfd);
            printf("\n");
        }

        if (!strcmp(command,"login")) {
            // Open the connection to server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // Read the username & pasword for the user
            char *username = calloc(150,sizeof(char));
            char *password = calloc(150,sizeof(char));

            printf("username=");
            fgets(username,150,stdin);
            if (username[strlen(username) - 1] == '\n'){
                username[strlen(username) - 1] = '\0';
            }

            printf("password=");
            fgets(password,150,stdin);
            if (password[strlen(password) - 1] == '\n')
                password[strlen(password) - 1] = '\0';

            // Initialize json objects
            root_val = json_value_init_object();
            root_val = json_value_init_object();
            root_obj = json_value_get_object(root_val);

            json_object_set_string(root_obj, "username", username);
            json_object_set_string(root_obj, "password", password);
            json_data = json_serialize_to_string(root_val);

            // Send the request to the server:
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", "application/json", &json_data, 1, NULL, 0, NULL);
            send_to_server(sockfd,message);

            // Receive the response from the server
            response = receive_from_server(sockfd);

            if(response == NULL) {
                printf("No response from the server\n");
                continue;
            }

            // Parse the response to verify if the credentials are good
            char *buff = strtok(response, "\n");

            cookies_count = 0;
            while (buff != NULL && buff[0] != '{')
            {
                if(!strncmp(buff,"Set-Cookie:",11)) {
                    cookies[cookies_count] = calloc(LINELEN,sizeof(char));

                    char *cookie = strtok(buff,";");
                    strcpy(cookies[cookies_count],cookie + 12);
                    cookies_count ++;
                }
                buff = strtok(NULL,"\n");
            }

            // Error: wrong credentials or username doesn't exists
            if (buff != NULL) {
                buff[strlen(buff) - 2] = '\0';
                printf("400 - %s\n",buff + 10);
            }
            /* Reset the JWT, so the new user doesn't have the JWT from last user
               and prints login successful*/
            else
            {
                if(JWT != NULL){
                    free(JWT);
                    JWT = NULL;
                }
                printf("200 - OK - User logged in successfully\n");
            }

            json_free_serialized_string(json_data);
            json_value_free(root_val);
            free(username);
            free(password);
            close(sockfd);
            printf("\n");
        }

        if (!strcmp (command, "enter_library")) {
            // Open the connection to server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // Send the request to the server:
            message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", NULL, cookies, cookies_count, JWT);
            send_to_server(sockfd,message);

            // Receive the response from the server
            response = receive_from_server(sockfd);

            if(response == NULL) {
                printf("No response from the server\n");
                continue;
            }

            // Parse the response to verify if the user is logged in
            char *buff = strtok(response, "\n");

            // Not logged
            if(!strncmp(buff,"HTTP/1.1 401",11)) {
                printf("401 - The user is not logged in.\n");
            }
            else
            {
                // Get JWT and prints successful message
                while (buff != NULL && buff[0] != '{') {
                    buff = strtok(NULL,"\n");
                }
                if (JWT == NULL) {
                    JWT = calloc(LINELEN,sizeof(char));
                }
                buff[strlen(buff) - 2] = '\0';
                strcpy(JWT,buff + 10);
                printf("200 - OK - The user have access now to library\n");
            }
            

            close(sockfd);
            printf("\n");
        }

        if (!strcmp(command,"get_books")) {
            // Open the connection to server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // Verify if the user have access
            if( JWT != NULL ) {
                // Send the request to the server:
                message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", NULL, cookies, cookies_count, JWT);
                send_to_server(sockfd,message);

                // Receive the response from the server
                response = receive_from_server(sockfd);

                // Parse the response until we get to the list of the books
                char *buff = strtok(response, "\n");
                while (buff != NULL && buff[0] != '[') {
                    buff = strtok(NULL,"\n");
                }
                printf("%s\n",buff);
                
            } else
            {
                printf("400 - The user does not have access to library\n");
            }
            
            close(sockfd);
            printf("\n");
        }

        if (!strcmp(command,"get_book")) {
            // Open the connection to server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            if (JWT != NULL){
                char *id = calloc(20,sizeof(char));
                char *url = calloc(LINELEN,sizeof(char));

                printf("id=");
                fgets(id,150,stdin);
                if (id[strlen(id) - 1] == '\n'){
                    id[strlen(id) - 1] = '\0';
                }
                strcpy(url,"/api/v1/tema/library/books/");
                strcat(url,id);

                // Send the request to the server:
                message = compute_get_request("34.241.4.235", url, NULL, cookies, cookies_count, JWT);
                send_to_server(sockfd,message);

                // Receive the response from the server
                response = receive_from_server(sockfd);

                char *buff = strtok(response, "\n");
                // Wrong id
                if(!strncmp(buff,"HTTP/1.1 404",11)) {
                    printf("404 - The id is is not valid.\n");
                }
                else
                {
                    while (buff != NULL && buff[0] != '[') {
                        buff = strtok(NULL, "\n");
                    }
                    printf("%s\n",buff);
                }
                free(id);
                free(url);
            }
            else
            {
                printf("400 - The user does not have access to library\n");
            }
            close(sockfd);
            printf("\n");
        }

        if (!strcmp(command,"add_book")) {
            // Open the connection to server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // Verify if the user have access
            if( JWT != NULL ) {
                char *title = calloc(150,sizeof(char));
                char *author = calloc(150,sizeof(char));
                char *genre = calloc(150,sizeof(char));
                char *page_count = calloc(150,sizeof(char));
                char *publisher = calloc(150,sizeof(char));

                printf("title=");
                fgets(title,150,stdin);
                if (title[strlen(title) - 1] == '\n'){
                    title[strlen(title) - 1] = '\0';
                }
                
                printf("author=");
                fgets(author,150,stdin);
                if (author[strlen(author) - 1] == '\n'){
                    author[strlen(author) - 1] = '\0';
                }

                printf("genre=");
                fgets(genre,150,stdin);
                if (genre[strlen(genre) - 1] == '\n'){
                    genre[strlen(genre) - 1] = '\0';
                }

                printf("page_count=");
                fgets(page_count,150,stdin);
                if (page_count[strlen(page_count) - 1] == '\n'){
                    page_count[strlen(page_count) - 1] = '\0';
                }

                printf("publisher=");
                fgets(publisher,150,stdin);
                if (publisher[strlen(publisher) - 1] == '\n'){
                    publisher[strlen(publisher) - 1] = '\0';
                }

                if (!strcmp(title,"") || !strcmp(author,"") || !strcmp(genre,"") || !strcmp(publisher,"") || atoi(page_count) == 0) {
                    printf("The given information is either incomplete or wrong.\n");
                    continue;
                }

                // Initialize json objects
                root_val = json_value_init_object();
                root_obj = json_value_get_object(root_val);

                json_object_set_string(root_obj, "title", title);
                json_object_set_string(root_obj, "author", author);
                json_object_set_string(root_obj, "genre", genre);
                json_object_set_number(root_obj,"page_count",atoi(page_count));
                json_object_set_string(root_obj, "publisher", publisher);
                json_data = json_serialize_to_string(root_val);


                // Send the request to the server:
                message = compute_post_request("34.241.4.235", " /api/v1/tema/library/books", "application/json", &json_data, 1, cookies, cookies_count, JWT);
                send_to_server(sockfd,message);

                // Receive the response from the server
                response = receive_from_server(sockfd);
                printf("The book has been added successfully.\n");

                free(title);
                free(author);
                free(genre);
                free(page_count);
                free(publisher);
            } else
            {
                printf("400 - The user does not have access to library\n");
            }

            close(sockfd);
            printf("\n");
        }

        if(!strcmp(command,"delete_book")) {
            // Open the connection to server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            if (JWT != NULL){
                char *id = calloc(20,sizeof(char));
                char *url = calloc(LINELEN,sizeof(char));

                printf("id=");
                fgets(id,150,stdin);
                if (id[strlen(id) - 1] == '\n'){
                    id[strlen(id) - 1] = '\0';
                }
                strcpy(url,"/api/v1/tema/library/books/");
                strcat(url,id);
                // Send the request to the server:
                message = compute_delete_request("34.241.4.235", url, cookies, cookies_count, JWT);
                send_to_server(sockfd,message);

                // Receive the response from the server
                response = receive_from_server(sockfd);

                char *buff = strtok(response, "\n");                

                if(!strncmp(buff,"HTTP/1.1 404",11)) {
                    printf("404 - The id is is not valid.\n");
                }else {
                    printf("200 - OK - The book has been deleted\n");
                }

            } else
            {
                printf("400 - The user does not have access to library\n");
            }

            close(sockfd);
            printf("\n");
        }

        if (!strcmp(command,"logout")) {
            // Open the connection to server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // Send the request to the server:
            message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", NULL, cookies, cookies_count, JWT);
            send_to_server(sockfd,message);

            // Receive the response from the server
            response = receive_from_server(sockfd);

            char *buff = strtok(response, "\n");

            if(!strncmp(buff,"HTTP/1.1 400",11)) {
                printf("400 - You are not logged in.\n");
            }
            else
            {
                for (int i = 0; i < cookies_count; i++) {
                    free(cookies[i]);
                }
                cookies_count = 0;
                
                if(JWT != NULL) {
                    free(JWT);
                    JWT = NULL;
                }
                printf("200 - OK - You have been logged out.\n");
            }

            close(sockfd);
            printf("\n");
        }

        if (!strcmp(command,"exit")) {
            break;
        }
    }

    free(cookies);
    free(message);
    free(response);
    free(line);

    return 0;
}
