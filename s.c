#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <mysql.h>

//gcc -o s $(mysql_config --cflags) s.c $(mysql_config --libs) -pthread
// portul folosit 
#define PORT 2000

// codul de eroare returnat de anumite apeluri 
extern int errno;

typedef struct thData{
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); // functia executata de fiecare thread ce realizeaza comunicarea cu clientii 
int vreme_da(int x);       
int sport_da(int x);
int pretul_combustibilului_da(int x);
int vreme_nu(int x);
int sport_nu(int x);
int pretul_combustibilului_nu(int x);
int ia_id(char x[100]);
int verif_viteza_60sec(int x);
int viteza_regulamentara(int x);
int acces_la_vreme(int x);
int acces_la_sport(int x);
int acces_la_combustibil(int x);
char* vreme();
char* combustibil();
char* sport();
int viteza_scazuta(int x);
int adauga_viteza_in_baza_de_date(int id, int viteza);
int caut_in_baza_de_date_login(char x[100]);
int caut_in_baza_de_date_autentificare(char x[100]);
int adauga_in_baza_de_date(char x[100]); 
int adauga_accident_in_baza_de_date(int x);
int trimite(char msg[100], int x, int bytes);
int verif_accident(int x);
int sterge_accident(int x);
char* mesaj_accident();

char vr[150], co[150];

int main ()
{
    struct sockaddr_in server;    // structura folosita de server
    struct sockaddr_in from;  
    int nr;       //mesajul primit de trimis la client 
    int sd;       //descriptorul de socket 
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;
    strcpy(vr, vreme());
    strcpy(co, combustibil());
    // crearea unui socket 
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server]Eroare la socket().\n");
        return errno;
    }
    // utilizarea optiunii SO_REUSEADDR 
    int on=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
    // pregatirea structurilor de date 
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    // umplem structura folosita de server 
    // stabilirea familiei de socket-uri 
    server.sin_family = AF_INET;    
    // acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    // utilizam un port utilizator 
    server.sin_port = htons (PORT);
  
    // atasam socketul 
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server]Eroare la bind().\n");
        return errno;
    }

    // punem serverul sa asculte daca vin clienti sa se conecteze 
    if (listen (sd, 2) == -1)
    {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }
    // servim in mod concurent clientii...folosind thread-uri 
    while (1)
    {
        int client;
        thData * td; //parametru functia executata de thread     
        int length = sizeof (from);

        printf ("[server]Asteptam la portul %d...\n",PORT);
        fflush (stdout);

        // acceptam un client (stare blocanta pina la realizarea conexiunii) 
        if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
        {
          perror ("[server]Eroare la accept().\n");
          return 0;
        }  
        td=(struct thData*)malloc(sizeof(struct thData));   
        td->idThread=i++;
        td->cl=client;
        pthread_create(&th[i], NULL, &treat, td);         
    }   
}              
static void *treat(void * arg)
{      
    int acc_vreme=0, acc_comb=0, avr, aco, asp; 
    int id;
    struct thData tdL; 
    tdL= *((struct thData*)arg);    
    printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush (stdout);         
    pthread_detach(pthread_self());
    int ok=0;
    while(ok==0)        
    {
        int bytes;      /* numarul de octeti cititi/scrisi */
        char msg[100];    //mesajul primit de la client 
        bytes = read (tdL.cl, msg, 100);
        if (bytes < 0)
        {
            perror ("Eroare la read() de la client.\n");
            return 0;
        }
            if(strncmp(msg, "exit", 4)==0)
            {
                printf ("[server]S-a deconectat clientul cu descriptorul %d.\n",tdL.cl);
                close (tdL.cl); 
                pthread_exit(NULL);
            }
            else
                if(strncmp(msg, "login", 5)==0)
                {
                    printf ("[server]Mesajul a fost receptionat...%s\n", msg);
                    char nume[100];
                    strcpy(nume, msg+6);
                    int r=caut_in_baza_de_date_login(nume);
                    if(r==1)
                    {
                        id=ia_id(nume);
                        strcpy(msg, "Doriti sa schimbati optiunile referitoare la informatiile afisate alese?");
                        if(trimite(msg, tdL.cl, bytes)==0)
                        {
                            perror ("[server]Eroare la write() catre client.\n");
                            return 0;
                        }
                        bzero (msg, 100);
                        bytes = read (tdL.cl, msg, 100);
                        if (bytes < 0)
                        {
                            perror ("Eroare la read() de la client.\n");
                            return 0;
                        }
                        if(strncmp(msg, "da", 2)==0)
                        {
                            strcpy(msg, "Doriti informatii despre vreme?");
                            if(trimite(msg, tdL.cl, bytes)==0)
                            {
                                perror ("[server]Eroare la write() catre client.\n");
                                return 0;
                            }
                            bzero (msg, 100);
                            bytes = read (tdL.cl, msg, 100);
                            if (bytes < 0)
                            {
                                perror ("Eroare la read() de la client.\n");
                                return 0;
                            }
                            if(strncmp(msg, "da", 2)==0)
                            {
                                if(vreme_da(id)==-1)
                                {
                                    perror ("Eroare la baza de date la vreme.\n");
                                    return 0;
                                }
                            }
                            else
                                if(strncmp(msg, "nu", 2)==0)
                                {
                                    if(vreme_nu(id)==-1)
                                    {
                                        perror ("Eroare la baza de date la vreme.\n");
                                        return 0;
                                    }
                                }
                            strcpy(msg, "Doriti informatii despre sport?");
                            if(trimite(msg, tdL.cl, bytes)==0)
                            {
                                perror ("[server]Eroare la write() catre client.\n");
                                return 0;
                            }
                            bzero (msg, 100);
                            bytes = read (tdL.cl, msg, 100);
                            if (bytes < 0)
                            {
                                perror ("Eroare la read() de la client.\n");
                                return 0;
                            }
                            if(strncmp(msg, "da", 2)==0)
                            {
                                if(sport_da(id)==-1)
                                {
                                    perror ("Eroare la baza de date la vreme.\n");
                                    return 0;
                                }
                            }
                            else
                                if(strncmp(msg, "nu", 2)==0)
                                {
                                    if(sport_nu(id)==-1)
                                    {
                                        perror ("Eroare la baza de date la vreme.\n");
                                        return 0;
                                    }
                                }
                            strcpy(msg, "Doriti informatii despre pretul combustibilului?");
                            if(trimite(msg, tdL.cl, bytes)==0)
                            {
                                perror ("[server]Eroare la write() catre client.\n");
                                return 0;
                            }
                            bzero (msg, 100);
                            bytes = read (tdL.cl, msg, 100);
                            if (bytes < 0)
                            {
                                perror ("Eroare la read() de la client.\n");
                                return 0;
                            }
                            if(strncmp(msg, "da", 2)==0)
                            {
                                if(pretul_combustibilului_da(id)==-1)
                                {
                                    perror ("Eroare la baza de date la vreme.\n");
                                    return 0;
                                }
                            }
                            else
                                if(strncmp(msg, "nu", 2)==0)
                                {
                                    if(pretul_combustibilului_nu(id)==-1)
                                    {
                                        perror ("Eroare la baza de date la vreme.\n");
                                        return 0;
                                    }
                                }
                        }
                        bzero (msg, 100);
                        strcpy(msg, "Login efectuat cu succes!");
                        ok=1;
                    }
                    else
                        if(r==0)
                        {
                            strcpy(msg, "Nu aveti un cont!");
                        }
                        else
                            if(r==-1)
                            {
                                strcpy(msg, "Sintaxa nu este corecta!");
                            }
                    if(trimite(msg, tdL.cl, bytes)==0)
                    {
                        perror ("[server] Eroare la write() catre client.\n");
                        return 0;
                    }
                }
                else
                    if(strncmp(msg, "autentificare", 13)==0)
                    {
                        printf ("[server]Mesajul a fost receptionat...%s\n", msg);
                        char nume[100];
                        strcpy(nume, msg+14);
                        int r=caut_in_baza_de_date_autentificare(nume);
                        if(r==1)
                        {
                            strcpy(msg, "Exista deja un cont cu acest id!");
                        }
                        else
                            if(r==-1)
                            {
                                    strcpy(msg, "Sintaxa nu este corecta!");  
                            }
                            else
                            {
                                int rr=adauga_in_baza_de_date(nume);
                                if(rr==-1)
                                {
                                    strcpy(msg, "A aparut o eroare, mai incearca o data!");
                                }
                                else
                                    if(rr==1)
                                    {
                                        id=ia_id(nume);
                                        strcpy(msg, "Doriti informatii despre vreme?");
                                        if(trimite(msg, tdL.cl, bytes)==0)
                                        {
                                            perror ("[server]Eroare la write() catre client.\n");
                                            return 0;
                                        }
                                        bzero (msg, 100);
                                        bytes = read (tdL.cl, msg, 100);
                                        if (bytes < 0)
                                        {
                                            perror ("Eroare la read() de la client.\n");
                                            return 0;
                                        }
                                        if(strncmp(msg, "da", 2)==0)
                                        {
                                            if(vreme_da(id)==-1)
                                            {
                                                perror ("Eroare la baza de date la vreme.\n");
                                                return 0;
                                            }
                                        }
                                        else
                                            if(strncmp(msg, "nu", 2)==0)
                                            {
                                                if(vreme_nu(id)==-1)
                                                {
                                                    perror ("Eroare la baza de date la vreme.\n");
                                                    return 0;
                                                }
                                            }
                                        strcpy(msg, "Doriti informatii despre sport?");
                                        if(trimite(msg, tdL.cl, bytes)==0)
                                        {
                                            perror ("[server]Eroare la write() catre client.\n");
                                            return 0;
                                        }
                                        bzero (msg, 100);
                                        bytes = read (tdL.cl, msg, 100);
                                        if (bytes < 0)
                                        {
                                            perror ("Eroare la read() de la client.\n");
                                            return 0;
                                        }
                                        if(strncmp(msg, "da", 2)==0)
                                        {
                                            if(sport_da(id)==-1)
                                            {
                                                perror ("Eroare la baza de date la sport.\n");
                                                return 0;
                                            }
                                        }
                                        else
                                            if(strncmp(msg, "nu", 2)==0)
                                            {
                                                if(sport_nu(id)==-1)
                                                {
                                                    perror ("Eroare la baza de date la vreme.\n");
                                                    return 0;
                                                }
                                            }
                                        strcpy(msg, "Doriti informatii despre pretul combustibilului?");
                                        if(trimite(msg, tdL.cl, bytes)==0)
                                        {
                                            perror ("[server]Eroare la write() catre client.\n");
                                            return 0;
                                        }
                                        bzero (msg, 100);
                                        bytes = read (tdL.cl, msg, 100);
                                        if (bytes < 0)
                                        {
                                            perror ("Eroare la read() de la client.\n");
                                            return 0;
                                        }
                                        if(strncmp(msg, "da", 2)==0)
                                        {
                                            if(pretul_combustibilului_da(id)==-1)
                                            {
                                                perror ("Eroare la baza de date la combustibil.\n");
                                                return 0;
                                            }
                                        }
                                        else
                                            if(strncmp(msg, "nu", 2)==0)
                                            {
                                                if(pretul_combustibilului_nu(id)==-1)
                                                {
                                                    perror ("Eroare la baza de date la vreme.\n");
                                                    return 0;
                                                }
                                            }
                                        strcpy(msg, "Login efectuat cu succes!");
                                        ok=1;
                                    }
                              }
                        if(trimite(msg, tdL.cl, bytes)==0)
                        {
                            perror ("[server]Eroare la write() catre client.\n");
                            return 0;
                        }
                    }
    }
    avr=acces_la_vreme(id);
    aco=acces_la_combustibil(id);
    asp=acces_la_sport(id);
    sleep(1);
    while(1)
    {
            srand(time(0));
            int viteza=rand()%70;
            printf("[server]Trimitem viteza catre baza de date...\n");
            int r=adauga_viteza_in_baza_de_date(id, viteza);
            if(r==-1)
               printf("[server]A aparut o eroare in baza de date!\n");
            else
                printf("[server]S-a adaugat viteza in baza de date!\n");
            int bytes;      // numarul de octeti cititi 
            char msg[200];    //mesajul primit de la client 
            int verif=verif_accident(id);
            if(verif==1) //exista accidente si trebuie sa le afisez caci nu eu l-am declarat
            {
                printf("Exista accident\n");
                strcpy(msg, mesaj_accident());
                if(trimite(msg, tdL.cl, strlen(msg))==0)
                {
                    perror ("[server] Eroare la write() catre client.\n");
                    return 0;
                }  
            }
            else
            {
                if(verif==-3)    // -3 eu am raportat accidentul
                {
                    printf("Eu am raportat accidentul asa ca il sterg\n");
                    sterge_accident(id);
                }
                int v=viteza_regulamentara(id);
                if(v>1)
                {
                    sprintf(msg, "Nu mergeti cu viteza regulamentara de %d km/h", v);
                    if(trimite(msg, tdL.cl, strlen(msg))==0)
                    {
                        perror ("[server] Eroare la write() catre client.\n");
                        return 0;
                    }
                }
                else
                {
                    int d=viteza_scazuta(id);
                    if(d>0)
                    {
                        sprintf(msg, "Aveti viteza mica de %d km/h. Raportati un accident?", d);
                        if(trimite(msg, tdL.cl, strlen(msg))==0)
                        {
                            perror ("[server] Eroare la write() catre client.\n");
                            return 0;
                        }
                        bytes = read (tdL.cl, msg, 200);
                        if (bytes < 0)
                        {
                            perror ("Eroare la read() de la client.\n");
                            return 0;
                        }
                        if(strncmp(msg, "exit", 4)==0)
                        {
                            printf ("[server]S-a deconectat clientul cu descriptorul %d.\n",tdL.cl);
                            close (tdL.cl); 
                            pthread_exit(NULL);
                        }
                        else
                            if(strncmp(msg, "da", 2)==0)
                            {
                                strcpy(msg, "Vom anunta ceilalti participanti la trafic! Multumim pentru informatie!");
                                if(trimite(msg, tdL.cl, strlen(msg))==0)
                                {
                                    perror ("[server] Eroare la write() catre client.\n");
                                    return 0;
                                }
                                if(adauga_accident_in_baza_de_date(id)==-1)
                                {
                                    perror ("[server] Eroare la adauga_accident_in_baza_de_date().\n");
                                    return 0;
                                }
                            }
                            else
                            {
                                strcpy(msg, "Ne bucuram sa auzim asta! Drum bun in continuare!");
                                if(trimite(msg, tdL.cl, strlen(msg))==0)
                                {
                                    perror ("[server] Eroare la write() catre client.\n");
                                    return 0;
                                }
                            }
                    }
                    else
                    {
                        if(acc_vreme==0)
                        {
                            if(avr==1)
                            {
                                sprintf(msg, "vreme %s", vr);
                                if(trimite(msg, tdL.cl, strlen(msg))==0)
                                {
                                    perror ("[server] Eroare la write() catre client.\n");
                                    return 0;
                                }
                            }
                            else
                                if(acc_comb==0)
                                {
                                    if(aco==1)
                                    {
                                        sprintf(msg, "combustibil %s", co);
                                        if(trimite(msg, tdL.cl, strlen(msg))==0)
                                        {
                                            perror ("[server] Eroare la write() catre client.\n");
                                            return 0;
                                        }
                                    }
                                    else
                                    {
                                        if(asp==1)
                                        {
                                            char msgg[200];
                                            sprintf(msgg, "sport %s", sport());
                                            if(trimite(msgg, tdL.cl, strlen(msgg))==0)
                                            {
                                                perror ("[server] Eroare la write() catre client.\n");
                                                return 0;
                                            }
                                        } 
                                    }
                                    acc_comb=1;
                                }
                            acc_vreme=1;
                        }
                        else
                            if(acc_comb==0)
                            {
                                if(aco==1)
                                {
                                    sprintf(msg, "combustibil %s", co);
                                    if(trimite(msg, tdL.cl, strlen(msg))==0)
                                    {
                                        perror ("[server] Eroare la write() catre client.\n");
                                        return 0;
                                    }
                                }
                                else
                                {
                                    if(asp==1)
                                    {
                                        char msgg[200];
                                        sprintf(msgg, "sport %s", sport());
                                        if(trimite(msgg, tdL.cl, strlen(msgg))==0)
                                        {
                                            perror ("[server] Eroare la write() catre client.\n");
                                            return 0;
                                        }
                                    }
                                }
                                acc_comb=1;
                            }
                            else
                            {
                                if(asp==1)
                                {
                                    char msgg[200];
                                    sprintf(msgg, "sport %s", sport());
                                    if(trimite(msgg, tdL.cl, strlen(msgg))==0)
                                    {
                                        perror ("[server] Eroare la write() catre client.\n");
                                        return 0;
                                    }
                                }
                            }
                    }
                }
            }
        sleep(60);
    }
    return(NULL);         
}
int ia_id(char x[100])
{
   char nume[50], prenume[50], nr[10];
    int id;
    int i=0;
    while(x[i]!='.'&&i<strlen(x))
    {
      if(x[i]>'0'&&x[i]<'9')
          return -1;
      nume[i]=x[i];
      i++;
    }
    nume[i]='\0';
    i++;
    int c=0;
    while(x[i]!='.'&&i<strlen(x))
    {
      prenume[c]=x[i];
      i++;
      c++;
    }
    prenume[c]='\0';
    i++;
    c=0;
    while(i<strlen(x))
    {
        if((x[i]>'a'&&x[i]<'z')||(x[i]>'A'&&x[i]<'Z')||x[i]==' ')
            return -1;
        nr[c]=x[i];
        i++;
        c++;
    }
    nr[c]='\0';
    id=atoi(nr);
   return id;
}
int trimite(char msg[200], int x, int bytes)
{
    printf("[server]Trimitem mesajul inapoi...%s\n", msg);
    char msgrasp[200];
    strcpy(msgrasp, msg);
    if (bytes&&write(x, msgrasp, bytes)<0)
       return 0;
   return 1;
}
int caut_in_baza_de_date_login(char x[100])
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    char nume[50], prenume[50], nr[10];
    int id;
    int i=0;
    while(x[i]!='.'&&i<strlen(x))
    {
        if(x[i]>'0'&&x[i]<'9')
            return -1;
        nume[i]=x[i];
        i++;
    }
    nume[i]='\0';
    if(strlen(nume)==0)
        return -1;
    i++;
    int c=0;
    while(x[i]!='.'&&i<strlen(x))
    {
        if(x[i]>'0'&&x[i]<'9')
            return -1;
        prenume[c]=x[i];
        i++;
        c++;
    }
    prenume[c]='\0';
    if(strlen(prenume)==0)
        return -1;
    i++;
    c=0;
    while(i<strlen(x))
    {
        if((x[i]>'a'&&x[i]<'z')||(x[i]>'A'&&x[i]<'Z')||x[i]==' ')
            return -1;
        nr[c]=x[i];
        i++;
        c++;
    }
    nr[c]='\0';
    if(strlen(nr)==0)
        return -1;
    id=atoi(nr);
    char sqls[200];

    sprintf(sqls, "select * from client where id=%d and trim(nume)='%s' and trim(prenume)='%s'", id, nume, prenume);
    if (mysql_query(conn, sqls))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    int columns=mysql_num_fields(res);
    row=mysql_fetch_row(res);
    if(row==NULL)
        return 0;
    mysql_free_result(res);
    mysql_close(conn);
    return 1;
}
int caut_in_baza_de_date_autentificare(char x[100])
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    char nume[50], prenume[50], nr[10];
    int id;
    int i=0;
    while(x[i]!='.'&&i<strlen(x))
    {
        if(x[i]>'0'&&x[i]<'9')
            return -1;
        nume[i]=x[i];
        i++;
    }
    nume[i]='\0';
    if(strlen(nume)==0)
        return -1;
    i++;
    int c=0;
    while(x[i]!='.'&&i<strlen(x))
    {
        if(x[i]>'0'&&x[i]<'9')
            return -1;
        prenume[c]=x[i];
        i++;
        c++;
    }
    prenume[c]='\0';
    if(strlen(prenume)==0)
        return -1;
    i++;
    c=0;
    while(i<strlen(x))
    {
        if((x[i]>'a'&&x[i]<'z')||(x[i]>'A'&&x[i]<'Z')||x[i]==' ')
            return -1;
        nr[c]=x[i];
        i++;
        c++;
    }
    nr[c]='\0';
    if(strlen(nr)==0)
        return -1;
    id=atoi(nr);
    char sqls[200];

    sprintf(sqls, "select * from client where id=%d", id);
    if (mysql_query(conn, sqls))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    int columns=mysql_num_fields(res);
    row=mysql_fetch_row(res);
    if(row==NULL)
        return 0;
    mysql_free_result(res);
    mysql_close(conn);
    return 1;
}
int adauga_in_baza_de_date(char x[100]) 
{
    MYSQL *conn;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    char nume[50], prenume[50], nr[10];
    int id;
    int i=0;
    while(x[i]!='.'&&i<strlen(x))
    {
        nume[i]=x[i];
        i++;
    }
    nume[i]='\0';
    i++;
    int c=0;
    while(x[i]!='.'&&i<strlen(x))
    {
        prenume[c]=x[i];
        i++;
        c++;
    }
    prenume[c]='\0';
    i++;
    c=0;
    while(i<strlen(x))
    {
        nr[c]=x[i];
        i++;
        c++;
    }
    nr[c]='\0';
    id=atoi(nr);
    char sqls[250];

    sprintf(sqls, "insert into client(nume, prenume, id, vreme, sport, combustibil) values ('%s','%s',%d, 0, 0, 0)", nume, prenume, id);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    mysql_close(conn);
    return 1;
}
int adauga_viteza_in_baza_de_date(int id, int viteza)
{
   MYSQL *conn;

   char *server = "localhost";
   char *user = "isabela";
   char *password = "isabela";
   char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }
    char sqls[200];

    sprintf(sqls, "update client set viteza=%d where id=%d", viteza, id);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    mysql_close(conn);
    return 1;
}
int viteza_regulamentara(int x) 
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    int vit, vit_leg;
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("Failed to connect MySQL Server %s. Error: %s\n", server, mysql_error(conn));
        return 0;
    }

    char sql[300];

    sprintf(sql, "select viteza from client where id=%d", x);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }

    row = mysql_fetch_row(res);
    vit=atoi(row[0]);

    if (mysql_query(conn, "select count(*) from viteza"))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    int nr=atoi(row[0]); //nr de linii din tabel;
    srand(time(0));
    int id=1+rand()%nr;

    sprintf(sql, "select strada from viteza where id=%d", id);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    char st[100];
    strcpy(st, row[0]);

    sprintf(sql, "update client set strada='%s' where id=%d", st, x);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    sprintf(sql, "select * from viteza where id=%d", id);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    vit_leg=atoi(row[0]);

    mysql_free_result(res);
    mysql_close(conn);
    if(vit<=vit_leg)
        return 1;
    else
        return vit_leg;
}
int vreme_da(int x)
{
    MYSQL *conn;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    char sqls[200];

    sprintf(sqls, "update client set vreme=1 where id=%d", x);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    mysql_close(conn);
    return 1;
}
int sport_da(int x)
{
    MYSQL *conn;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }
    char sqls[200];

    sprintf(sqls, "update client set sport=1 where id=%d", x);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    mysql_close(conn);
    return 1;
}
int pretul_combustibilului_da(int x)
{
    MYSQL *conn;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    char sqls[200];

    sprintf(sqls, "update client set combustibil=1 where id=%d", x);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    mysql_close(conn);
    return 1;
}
int vreme_nu(int x)
{
    MYSQL *conn;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    char sqls[200];

    sprintf(sqls, "update client set vreme=0 where id=%d", x);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    mysql_close(conn);
    return 1;
}
int sport_nu(int x)
{
    MYSQL *conn;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    char sqls[200];

    sprintf(sqls, "update client set sport=0 where id=%d", x);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    mysql_close(conn);
    return 1;
}
int pretul_combustibilului_nu(int x)
{
    MYSQL *conn;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }

    char sqls[200];

    sprintf(sqls, "update client set combustibil=0 where id=%d", x);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    mysql_close(conn);
    return 1;
}
int acces_la_vreme(int x)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("Failed to connect MySQL Server %s. Error: %s\n", server, mysql_error(conn));
        return 0;
    }

    char sql[100];

    sprintf(sql, "select vreme from client where id=%d", x);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    int acc=atoi(row[0]);
    mysql_free_result(res);
    mysql_close(conn);
    return acc;
}
int acces_la_sport(int x)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("Failed to connect MySQL Server %s. Error: %s\n", server, mysql_error(conn));
        return 0;
    }

    char sql[100];

    sprintf(sql, "select sport from client where id=%d", x);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
       return 0;
    }
    row = mysql_fetch_row(res);
    int acc=atoi(row[0]);
    mysql_free_result(res);
    mysql_close(conn);
    return acc;
}
int acces_la_combustibil(int x)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("Failed to connect MySQL Server %s. Error: %s\n", server, mysql_error(conn));
        return 0;
    }

    char sql[100];

    sprintf(sql, "select combustibil from client where id=%d", x);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    int acc=atoi(row[0]);
    mysql_free_result(res);
    mysql_close(conn);
    return acc;
}
char* vreme()
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return 0;
    }

    if (mysql_query(conn, "select count(*) from vreme"))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    int columns = mysql_num_fields(res);
    row = mysql_fetch_row(res);
    int nr=atoi(row[0]); //nr de linii din tabel;
    srand(time(0));
    int id=1+rand()%nr;
    char sql[200];

    sprintf(sql, "select descriere from vreme where id=%d", id);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    mysql_close(conn);
    return row[0];
}
char* combustibil()
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return 0;
    }

    if (mysql_query(conn, "select count(*) from combustibil"))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
       return 0;
    }
    int columns = mysql_num_fields(res);
    row = mysql_fetch_row(res);
    int nr=atoi(row[0]); //nr de linii din tabel;
    srand(time(0));
    int id=1+rand()%nr;
    char sql[200];

    sprintf(sql, "select descriere from combustibil where id=%d", id);
    if (mysql_query(conn, sql))
    {
       printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
       return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
       return 0;
    }
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    mysql_close(conn);
    return row[0];
}
char* sport()
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
     
    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("Failed to connect MySQL Server %s. Error: %s\n", server, mysql_error(conn));
        return 0;
    }

    char sql[100];
     
    if (mysql_query(conn, "select count(*) from sport"))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    int nr=atoi(row[0]); //nr de linii din tabel;
    srand(time(0));
    int id=1+rand()%nr;

    sprintf(sql, "select descriere from sport where id=%d", id);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    mysql_close(conn);
    return row[0];
}
int viteza_scazuta(int x)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    int vit;
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("Failed to connect MySQL Server %s. Error: %s\n", server, mysql_error(conn));
        return 0;
    }

    char sql[100];

    sprintf(sql, "select viteza from client where id=%d", x);
    if (mysql_query(conn, sql))
    {
        printf("Failed to execute quesry. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    vit=atoi(row[0]);
    mysql_free_result(res);
    mysql_close(conn);
    if(vit<=20)
        return vit;
    return 0;
}
int adauga_accident_in_baza_de_date(int x)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }
   char sqls[200];

    sprintf(sqls, "select strada from client where id=%d", x);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    char s[100];
    strcpy(s, row[0]);

    sprintf(sqls, "update client set accident='ATENTIE!!! Accident pe %s!!!' where id=%d", s, x);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    mysql_free_result(res);
    mysql_close(conn);
    return 1;
}
int verif_accident(int x)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }
    char sqls[200];

    sprintf(sqls, "select count(id) from client where accident is not null");
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }
    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    int cnt=atoi(row[0]);
    if(cnt==0)  //nu exista accidente
        return -2;

    sprintf(sqls, "select id from client where accident is not null");
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    int nr=atoi(row[0]);
    if(nr==x)  //verif_accident afiseaza informatii tuturor clientilor, mai putin cel care a transmis accidentul
        return -3;
    mysql_free_result(res);
    mysql_close(conn);
    return 1;
}
int sterge_accident(int x)
{
    MYSQL *conn;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return -1;
    }
    char sqls[200];

    sprintf(sqls, "update client set accident=null where id=%d", x);
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return -1;
    }
    mysql_close(conn);
    return 1;
}
char* mesaj_accident()
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isabela";
    char *password = "isabela";
    char *database = "isabela";

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("[server]Eroare la conectarea cu serverul %s din MySQL. Error: %s\n", server, mysql_error(conn));
        return 0;
    }
    char sqls[200];

    sprintf(sqls, "select accident from client where accident is not null");
    if (mysql_query(conn, sqls))
    {
        printf("Failed to execute query. Error: %s\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        return 0;
    }
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    mysql_close(conn);
    return row[0];
}
