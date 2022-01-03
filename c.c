#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

// codul de eroare returnat de anumite apeluri 
extern int errno;

// portul de conectare la server
int port=2000;

int main (int argc, char *argv[])
{
    int id;
    int optval=1;       // optiune folosita pentru setsockopt()
    int sd;     // descriptorul de socket
    struct sockaddr_in server;  // structura folosita pentru conectare 
    char msg[200]="", msgg[100]="";    // mesajul trimis

  // cream socketul */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[client] Eroare la socket().\n");
        return errno;
    }
  
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));
    // umplem structura folosita pentru realizarea conexiunii cu serverul 
    // familia socket-ului 
    server.sin_family = AF_INET;
    // adresa IP a serverului 
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    // portul de conectare 
    server.sin_port = htons (port);
  
    // ne conectam la server 
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }
    while(strcmp(msg, "Login efectuat cu succes!")!=0)
    {
        bzero (msgg, 100);
        bzero (msg, 100);
        printf("[client]Buna! Aveti un cont? Raspunde cu da sau nu:\n");
        fflush (stdout);
        read (0, msgg, 100);
        if(strncmp(msgg, "da", 2)==0)
        {
            printf ("[client]Logati-va dupa sintaxa: <nume>.<prenume>.<id>: ");
            fflush (stdout);
            read (0, msgg, 100);
            printf ("\n");
            strcpy(msg, "login ");
            strcat(msg, msgg);
            // trimiterea mesajului la server 
            if (write (sd, msg, 100) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }
            // citirea raspunsului dat de server (apel blocant pina cind serverul raspunde) 
            if (read (sd, msg, 100) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if(strncmp(msgg, "exit", 4)==0)
            {
                if (write (sd, msgg, 100) <= 0)
                {
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                close(sd);
                exit(1);
            }
            else
                if(strncmp(msg, "Doriti sa schimbati optiunile referitoare", 30)==0)
                {
                    printf ("[client]Mesajul primit este: %s\n", msg);
                    bzero (msg, 100);
                    read (0, msg, 100);
                    if (write (sd, msg, 100) <= 0)
                    {
                        perror ("[client]Eroare la write() spre server.\n");
                        return errno;
                    }
                    if(strncmp(msg, "da", 2)==0)
                    {
                        bzero (msg, 100);
                        if (read (sd, msg, 100) < 0)
                        {
                            perror ("[client]Eroare la read() de la server.\n");
                            return errno;
                        }
                        printf ("[client]Mesajul primit este: %s\n", msg);
                        bzero (msg, 100);
                        read (0, msg, 100);
                        if (write (sd, msg, 100) <= 0)
                        {
                            perror ("[client]Eroare la write() spre server.\n");
                            return errno;
                        }
                        bzero (msg, 100);
                        if (read (sd, msg, 100) < 0)
                        {
                            perror ("[client]Eroare la read() de la server.\n");
                            return errno;
                        }
                        printf ("[client]Mesajul primit este: %s\n", msg);
                        bzero (msg, 100);
                        read (0, msg, 100);
                        if (write (sd, msg, 100) <= 0)
                        {
                            perror ("[client]Eroare la write() spre server.\n");
                            return errno;
                        }
                        bzero (msg, 100);
                        if (read (sd, msg, 100) < 0)
                        {
                            perror ("[client]Eroare la read() de la server.\n");
                            return errno;
                        }
                        printf ("[client]Mesajul primit este: %s\n", msg);
                        bzero (msg, 100);;
                        read (0, msg, 100);
                        if (write (sd, msg, 100) <= 0)
                        {
                            perror ("[client]Eroare la write() spre server.\n");
                            return errno;
                        }

                    }
                    bzero (msg, 100);
                    if (read (sd, msg, 100) < 0)
                    {
                        perror ("[client]Eroare la read() de la server.\n");
                        return errno;
                    }
                    printf ("[client]Mesajul primit este: %s\n", msg);
                }
                else
                    if(strncmp(msg, "Nu aveti un cont!", 14)==0)
                    {
                        printf ("[client]Mesajul primit este: %s\n\n", msg);
                    }
                    else
                        if(strncmp(msg, "Sintaxa nu este corecta!", 14)==0)
                        {
                            printf ("[client]Mesajul primit este: %s\n\n", msg);
                        }
        }
        else
            if(strncmp(msgg, "nu", 2)==0)
            {
                printf ("[client]Adauga un cont dupa sintaxa: <nume>.<prenume>.<id>: \nUnde:\n<nume> este numele dumneavoastra\n<prenume> este primul prenume, in cazul in care exista mai multe \n<id> este un numar natural pe care il adaugati si pe care il veti retine pentru a va loga din nou ulterior:\n" );
                fflush (stdout);
                read (0, msgg, 100);
                strcpy(msg, "autentificare ");
                strcat(msg, msgg);
                // trimiterea mesajului la server
                if (write (sd, msg, 100) <= 0)
                {
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                if (read (sd, msg, 100) < 0)
                {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                if(strncmp(msgg, "exit", 4)==0)
                {
                    if (write (sd, msgg, 100) <= 0)
                    {
                        perror ("[client]Eroare la write() spre server.\n");
                        return errno;
                    }
                    close(sd);
                    exit(1);
                }
                else
                    if(strncmp(msg, "Doriti informatii despre vreme", 20)==0)
                    {
                        printf ("[client]Mesajul primit este: %s\n", msg);
                        bzero (msg, 100);
                        read (0, msg, 100);
                        if (write (sd, msg, 100) <= 0)
                        {
                            perror ("[client]Eroare la write() spre server.\n");
                            return errno;
                        }
                        bzero (msg, 100);
                        if (read (sd, msg, 100) < 0)
                        {
                            perror ("[client]Eroare la read() de la server.\n");
                            return errno;
                        }
                        printf ("[client]Mesajul primit este: %s\n", msg);
                        bzero (msg, 100);
                        read (0, msg, 100);
                        if (write (sd, msg, 100) <= 0)
                        {
                            perror ("[client]Eroare la write() spre server.\n");
                            return errno;
                        }
                        bzero (msg, 100);
                        if (read (sd, msg, 100) < 0)
                        {
                            perror ("[client]Eroare la read() de la server.\n");
                            return errno;
                        }
                        printf ("[client]Mesajul primit este: %s\n", msg);
                        bzero (msg, 100);;
                        read (0, msg, 100);
                        if (write (sd, msg, 100) <= 0)
                        {
                            perror ("[client]Eroare la write() spre server.\n");
                            return errno;
                        }
                        bzero (msg, 100);
                        if (read (sd, msg, 100) < 0)
                        {
                            perror ("[client]Eroare la read() de la server.\n");
                            return errno;
                        }
                        printf ("[client]Mesajul primit este: %s\n", msg);
                    }
                    else
                        if(strncmp(msg, "Exista deja un cont cu acest id", 14)==0)
                        {
                            printf ("[client]Mesajul primit este: %s\n\n", msg);
                        }
                        else
                            if(strncmp(msg, "Sintaxa nu este corecta!", 14)==0)
                            {
                                printf ("[client]Mesajul primit este: %s\n\n", msg);
                            }
             }
            else
                if(strncmp(msgg, "exit", 4)==0)
                {
                   if (write (sd, msgg, 100) <= 0)
                   {
                       perror ("[client]Eroare la write() spre server.\n");
                       return errno;
                   }
                   close(sd);
                   exit(1);
                }
    }
    while(1)
    {
        bzero (msg, 200);
        if (read (sd, msg, 200) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        } 
        if(strncmp(msg, "Nu mergeti cu viteza regulamentara", 30)==0)
        {
            printf ("\n[client]Mesajul primit este: %s\n", msg);
        }
        else
            if(strncmp(msg, "vreme", 5)==0)
            {
                printf ("\n[client]Mesajul primit este: %s\n", msg+6);
            }
            else
                if(strncmp(msg, "combustibil", 11)==0)
                {
                    printf ("\n[client]Mesajul primit este: %s\n", msg+12);
                }
                else
                    if(strncmp(msg, "sport", 5)==0)
                    {
                        printf ("\n[client]Mesajul primit este: %s\n", msg+6);
                    }
                    else
                        if(strncmp(msg, "ATENTIE", 7)==0)
                        {
                            printf ("\n[client]Mesajul primit este: %s\n", msg);
                        }
                        else
                            if(strncmp(msg, "Aveti viteza mica de", 20)==0)
                            {
                                printf ("\n[client]Mesajul primit este: %s\n", msg);
                                bzero (msg, 200);
                                read (0, msg, 200);
                                if(strncmp(msg, "exit", 4)==0)
                                {
                                    if (write (sd, msg, 200) <= 0)
                                    {
                                        perror ("[client]Eroare la write() spre server.\n");
                                        return errno;
                                    }
                                    close(sd);
                                    exit(1);
                                }
                                else
                                {
                                    if (write (sd, msg, 200) <= 0)
                                    {
                                       perror ("[client]Eroare la write() spre server.\n");
                                       return errno;
                                    }
                                    bzero (msg, 200);
                                    if (read (sd, msg, 200) < 0)
                                    {
                                        perror ("[client]Eroare la read() de la server.\n");
                                        return errno;
                                    } 
                                    // afisam mesajul primit 
                                    printf ("[client]Mesajul primit este: %s\n", msg);
                                }
                            }
    } 
}