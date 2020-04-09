/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#define max_argv 2   //nombre max d'arguments tapeé par le Client
#define MAX 90  // la taille du paquet qu'on va utiliser en octets


// une fonction qui renvoie la taille du fichier en octet
int size_file(int fd){  
    long int size=0 ;
    struct stat info;
    fstat(fd, &info);
    size=(int)info.st_size;
    return size;
}

// La commande get pour faire le transfert du fichier depuis notre serveur
void get_file(int clientfd,char* cmd,rio_t* rio,char buf[MAXLINE]){    
    int size_fichier, nb_pq,fd,size_actu=0, size_rest,x,n;
    char mem[MAX], size[MAX];
	double sec;
	// debut et fin se sont des variables qu'on a utiliser pour calculer le temps du transfert
	clock_t debut,fin;
   
// debut de calcul du temps du transfert 
	debut=clock();
      // ouvrir le fichier
         fd = Open(cmd, O_CREAT | O_APPEND | O_RDWR , 0666);     
        if(fd<0){
            printf("Erreur d'ouverture du fichier \n");
            exit(0);
        }

        size_actu = size_file(fd);
         char offset[4];
         char fichier[9];
         char buf1[MAXLINE];
         sprintf(offset, " %d", size_actu);               //enregistrer l'Offset dans le buffer offset
         sprintf(fichier, " %s", cmd);                    //meme chose pour le nom du fichier

        strcat(buf1,"get");
        strcat(buf1,fichier);
        strcat(buf1,offset);
        strcpy(buf,buf1);              //Concatener la commande get + nom fichier +  Offset dans un buffer pour les envoyees au serveur
	 
	 
	
      
        // recuperer la taille du fichier envoyer par le serveur
        if (Rio_readlineb(rio, size, MAX) > 0) {
            size_fichier = atoi(size);             //Convertir la taille du fichier recus depuis le serveur convertir en size_t
        } 

         printf("télechargement du fichier %s de taille %d / %d\n",cmd ,size_actu, size_fichier);
        size_actu = size_file(fd);     //Recuperer size actuel du fichier (L'Offset)

        if(size_fichier == size_actu){
            printf("fichier deja existant\n");
            Close(fd);
            exit(0);
        }
        else { 
            printf("Debut de telechargement d'un fichier de taille: %d\n", size_fichier);
            if (size_actu !=0) {//Si le fichier  n'est pas vide
              
// on deplace à l'offset dans  le fichier
  lseek(fd, size_actu, SEEK_SET);    
            }

	// nombre d'octets restés à recuperer
           size_rest = size_fichier - size_actu;    
	// on divise le reste d'octets en paquets d'octets
           nb_pq = size_rest/MAX;   
            // taille qui reste
           int reste = size_rest - (nb_pq*MAX); 
          
  memset(mem, 0, MAX);           


            for (int i = 0; i<nb_pq; i++){         
                if((x=Rio_readn(clientfd, mem, MAX))>0){
     // ecriture des paquets recus en fichier dans client
                    write(fd , mem, x);         
                    size_actu = size_file(fd);
                    printf("transfers en cours de %d paquets, ( WAITING..) (%d / %d)\n", nb_pq,size_actu, size_fichier );
                }
            }
            if(reste != 0){     //recuperer le reste des paquets inferieur au MAX
                memset(mem, 0, MAX);
                if(( n =Rio_readn(clientfd, mem, reste))>0){
                    write(fd , mem, n);
                    size_actu = size_file(fd);
                    printf("transfers en cours du reste des %d Octets. (%d / %d)\n", n ,size_actu, size_fichier );
                }
            }    
            close(fd);  //fermer le descripteur du fichier
        }

        printf(" tranfer successfully complete\n");
	fin=clock();
	sec =((double)(fin-debut))/((double)CLOCKS_PER_SEC)*1000;

        printf("%d bytes received in %f seconds (%f kbytes/s)\n", size_actu , sec, ((size_actu/1000)/sec));


}

int main(int argc, char **argv)
{
    int clientfd,port;
    char *host, buf[MAXLINE],input[MAXLINE], *cmd;
    rio_t rio;
  // int b=1;
    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> \n", argv[0]);
        exit(0);
    }

    host = argv[1]; 
    port = 2121; 

    clientfd = Open_clientfd(host, port);

    printf("client connected to server OS\n");
    printf("ftp>"); 
    
    Rio_readinitb(&rio, clientfd);
  
// une boucle infini pour l'etape 5
   // while(b){

    // lecture entrée standard et copier dans buf
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
      // Exraire  la commande 
    	//Rio_writen(clientfd, buf, strlen(buf)); // envoie de la commande

        memcpy(input,buf,strlen(buf)-1);
        cmd = strtok(input, " ");         //Extraire la commande tapee par le client


        if (strcmp(cmd,"get")==0){
	Rio_writen(clientfd, buf, strlen(buf)); 
            // Exraire le nom du fichier
            cmd = strtok(NULL, " ");   //Extraire le nom du fichier tapee par le client
            get_file(clientfd,cmd,&rio,buf);   // chercher un fichier dans le serveur pour le telecharager
exit(0);
        } 

     else if (strcmp(cmd,"put")==0){
            // Exraire le nom du fichier
            cmd = strtok(NULL, " ");   //Extraire le nom du fichier tapee par le client
            //get_fileserveur(clientfd,cmd,&rio,buf);   // chercher un fichier dans le serveur pour le telecharager
            size_t n;
    ssize_t nw;
   // rio_t rio;
    int fd;
    int size_int = 0;
    char csize_int[MAXLINE];
    int tmp_size_int;
    
        //COMMANDE = linecmd[0] (ici get), NOM_FICHIER = linecmd[1], TAILLE_FICHIER = linecmd[2]
        fd = open(cmd, O_RDONLY,0);
       // Rio_readinitb(&rio, clientfd);
            
        //GESTION D'ERREUR
        if (fd < 0){
            printf("ERREUR : Fichier inexistant. \n");
        }                       
        else {                  
            //ENVOI de la commande + la taille du fichier
            size_int = size_file(fd);
            sprintf(csize_int," %d\n",size_int);
            strcat(buf, csize_int);
            nw = rio_writen(clientfd, buf, strlen(buf)); // Envoi de toute la commande
            if (nw < 0) return -1;
            printf("Envoi du fichier %s de taille %d. \n", cmd, size_int);
            
            //RECEPTION TAILLE du fichier du serveur
            memset (csize_int, 0, MAXLINE);
            if ( (n = rio_readlineb(&rio, csize_int, MAXLINE)) > 0){
                tmp_size_int = atoi(csize_int);
            }
                        
            if (tmp_size_int == size_int){
                printf("Le serveur possède déjà le fichier. \n");
            }
            else {
                printf("Lecture à partir de %d / %d... \n", tmp_size_int, size_int);
                lseek(fd, tmp_size_int , SEEK_SET);
                
                //Quelques calculs avant : 
                int nbpaquet = (size_int-tmp_size_int)/MAXLINE;
                int restsize_int = (size_int-tmp_size_int) - nbpaquet*MAXLINE; //la taille restante à DL
                printf(" %d paquets de taille MAXLINE + un paquet de taille %d \n", nbpaquet+1, restsize_int);
                char *buf1 = (char*)malloc(MAXLINE*sizeof(char));
                int i;
                
                //envoi des nbpaquets
                for (i = 1; i <= nbpaquet; i++){                            
                    sleep(1);
                    memset (buf1, 0, MAXLINE);
                    
                    n = rio_readn(fd, buf1, MAXLINE);
                    nw = rio_writen(clientfd, buf1, MAXLINE);
                    
                    if  ( nw == -1 ){
                        printf("ALERTE : La connexion du serveur semble interrompue. \n");
                        return -1;
                    }
                    else {                                      
                        printf("Envoi : (%zu) %zu octets \n", nw, n);
                    }
                }
                //envoi du dernier paquet de taille restsize_int
                if (restsize_int != 0){
                
                    sleep(1);
                    memset (buf, 0, MAXLINE);
                    n = rio_readn(fd, buf, MAXLINE);
                    nw = rio_writen(clientfd, buf, restsize_int);
                    
                    if  ( nw > 0 ){
                        printf("Envoi : (%zu) %zu octets \n", nw, n);
                    }
                    else {
                        printf("ALERTE : La connexion du serveur semble interrompue. \n");
                        return -1;
                    }
                }                           
                
                printf("Fin de lecture. \n");
                free(buf1);
                close(fd);
            }
        }

    return 0;

        } 

        else if(strcmp(cmd, "pwd")==0||strcmp(cmd, "ls")==0){    
          //  Rio_writen(clientfd, buf, strlen(buf)); // envoie de la commande
      
           Rio_writen(clientfd, cmd, MAXLINE);   //l'envoie du buffer ou  il y a la commande soit "ls" ou "pwd".
            char tampon[MAXLINE];
            int n;
            if ( (n = Rio_readlineb(&rio, tampon, MAXLINE)) > 0){ //attendre  le resultat de la commande "ls" ou "pwd"
                    tampon[n-1]='\0';   //enlever le caractere "\n"
                    printf("%s\n", tampon);    //affichage du resultat
            }
              exit(0); 
        } 
        else if ( strcmp( cmd , "cd") == 0 ){
                buf[strlen(buf)] = '\n';
                Rio_writen(clientfd, buf, strlen(buf));
               exit(0);        
        }
        else if(strcmp(cmd, "mkdir")==0){
        buf[strlen(buf)] = '\n';
                Rio_writen(clientfd, buf, strlen(buf));
       } 
       else if ( strcmp( cmd , "bye") == 0 ){
            
                cmd[strlen(cmd)] = '\n';
               Rio_writen(clientfd, cmd,  MAXLINE);
                Close(clientfd);
               
                return 0;
                
        }
       else if ( strcmp( cmd , "rm" ) == 0 ){                
                    
                  buf[strlen(buf)] = '\n';
                Rio_writen(clientfd, buf, strlen(buf));        
       }
       else{
            printf("Cette commande n'existe pas\n");
        }
  
        printf("ftp>");
    }
}

