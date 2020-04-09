/*
 * echo - read and echo text lines until client closes connection
 */
#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> 
#include <unistd.h> 
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ftw.h>
#include "csapp.h"
#define max_argv 3   //nombre max d'arguments envoyee par le Client
#define MAX 90    //taille d'un paquet d'octets



int size(int fd){  // renvoyer la taille du fichier en octet dans le Serveur
    long int size=0 ;
    struct stat info;
    fstat(fd, &info);
    size=(int)info.st_size;
    return size;
}

void echo(int connfd)
{   
    int  size_fichier,i=0, nb_pq,x,fd, size_rcp, size_rest=0,n , nomfich;
    char buf[MAXLINE], mem[MAX],argv01[max_argv][MAXLINE],recup[MAX], *mot, sizerecupclt[MAX];
    rio_t rio;


    rio_readinitb(&rio, connfd);
    memset(buf, 0 , MAXLINE);
    rio_readlineb(&rio, buf, MAXLINE);          //Recevoire un buffer dans lequel y a les commandes a executees
    // if (strcmp(buf, "ls")!=0||strcmp(buf, "pwd")!=0){   //tester si on a qu'une seul commande dans le buffer
            mot = strtok(buf, " ");          
    
             while( mot != NULL && i<max_argv )  //Boucle pour diviser le buffer et extraire la commande apres les maitre dans un matrice 
             {
              strcpy(argv01[i], mot);
              i++;
              mot = strtok(NULL, " ");
             }
               nomfich = strlen(argv01[1]);
                argv01[1][nomfich-1]='\0';

 if(strcmp(argv01[0], "put")==0){ 
 //  rio_t rio;
    int fd;
    ssize_t n, nw;
    char sizef[MAXLINE];
    // sizef est la taille Totale du fichier reçu et sizeactu est la taille actuelle du fichier dans le serveurFTP
    int sizefint, sizeactu = 0; 
  
       // creation et ouverture du fichier en lecture ecriture
        fd = open( argv01[1] , O_CREAT | O_APPEND | O_RDWR, 0666 );
        if (fd < 0){
            printf( "ERREUR  fichier description:\n");
            exit(0);
        }
        // on recupère la taille du fichier envoyer par le client
         if (Rio_readlineb(&rio, sizerecupclt, MAX) > 0) {
            sizefint = atoi(sizerecupclt); 
        } 
         printf("télechargement de  %s de taille %d / %d\n",argv01[1], sizeactu ,sizefint);
         // on envoie la taille du fichier sizeactu au client
        memset(sizef, 0, MAXLINE);
        sizeactu = size(fd);
        sprintf(sizef,"%d\n",sizeactu);
        nw = rio_writen(connfd, sizef, strlen(sizef));
        if (nw < 0) 
            exit(1);

        printf("Début du telechargement de %s de taille %d / %d \n", argv01[1], sizeactu, sizefint);

        assert(sizeactu<=sizefint);

        if (sizeactu == sizefint) {
            printf("Le fichier existe déjà \n");
            close(fd);
        } 
        else {

            //Lecture de ce qui est envoyé par le client à PARTIR de sizeactu   
            lseek(fd, sizeactu , SEEK_SET);
            char *bufcmd = (char*)malloc(MAXLINE*sizeof(char));


            //Quelques calculs avant : 
            int nb_paquet = (sizefint-sizeactu)/MAXLINE;
            int rest_sizef = (sizefint-sizeactu) - nb_paquet*MAXLINE; //la taille restante à DL
            printf(" %d paquets de taille MAXLINE + un paquet de taille %d \n", nb_paquet, rest_sizef);

            //réception des nb_paquets
            int i;
            for (i = 1; i <= nb_paquet; i++){

                memset (bufcmd, 0, MAXLINE);

                if ( (n = rio_readn(connfd, bufcmd, MAXLINE)) > 0){
                    write(fd, bufcmd, strlen(bufcmd));
                    sizeactu = size(fd);
                    printf("Transfert en cours : %zu octets reçus. (%d sur %d)\n", strlen(bufcmd), sizeactu, sizefint);

                }
                else if (n == 0){
                    printf("0 octet lu... \n");
                }
                else {
                    printf( "ERREUR : \n");
                    Close(connfd);
                    exit(1);
                }

            }
            //réception du dernier paquet
            if (rest_sizef != 0){
                memset (bufcmd, 0, MAXLINE);
                if ( (n = rio_readn(connfd, bufcmd, rest_sizef)) > 0){
                    write(fd, bufcmd, n);
                    sizeactu = size(fd);
                    printf("Transfert en cours : %zu octets reçus. (%d sur %d)\n", n, sizeactu, sizefint);

                }
                else if (n == 0){
                    printf("Aucun octet lu... \n");
                }
                else {
                    printf("ERREUR : \n");
                    Close(connfd);
                    exit(1);
                }
            }
            printf("Transfert terminé. \n");
            free(bufcmd);
            close(fd);
        }   

        printf("**************************************\n");

    }


   // tester si c'st get
    if(strcmp(argv01[0], "get")==0){  
       // on ouvre le fichier et on teste
                fd = Open(argv01[1], O_RDONLY, 0);      
                if (fd < 0)
                {
                	printf("erreur ouvertur fichier \n");
                }
       

             size_rcp = atoi(argv01[2]);   


		// avoir la taille du fichier 
             size_fichier = size(fd);                 
           
             sprintf(recup, " %d", size_fichier);     
		// on fais envoyer envoyer la taile au client
             rio_writen(connfd, recup, strlen(recup)); 
             if(size_rcp){
                printf("downloads du fichier %s  a partir du %d eme octet\n", argv01[1], size_rcp );


		// on deplace à l'offset
                lseek(fd, size_rcp-1, SEEK_SET);       
             }                                        

        // on gere les pannes


             printf("télechargement de  %d octets de %s\n", size_rest, argv01[1] );
          size_rest = size_fichier -  size_rcp ;
            
         
	//    le nombres de paquets qu'on va envoyé
            nb_pq=  size_rest/MAX;       
            int restpaq  = size_rest - (nb_pq*MAX);
              // envoyer le 1er paquet
	 rio_writen(connfd, mem, MAX);     
           

	// envoyer le reste des paquets
	 for (i = 0; i<nb_pq; i++){  
	// on fais un sleep pour bien les remarquer et les observer
                sleep(1);       
                memset(mem, 0, MAX);
                n=rio_readn(fd, mem, MAX);        
			// envoyer ce qu'on a lis au client
                rio_writen(connfd, mem, n);           
            }


            

	// on fais envoyer le reste qui n'etais pas envoyées;
            if(restpaq !=0){
               memset(mem, 0, MAX);
               if(( x =rio_readn(fd, mem, restpaq))>0){
                    rio_writen(connfd, mem, x);
               }
            }
     // estster si c'est la commande ls;
  }else if(strcmp(buf, "ls")==0){    

       printf(" Serveur:  comannde ls \n");
            struct dirent *lecture;
            DIR *rep;
            char list_rep[MAXLINE]; 

            char recup1[MAXLINE];
            memset(list_rep, 0 , MAXLINE);
	// ouverture du tube pr qu'on liste le repertoire ou on est;
            rep = opendir("." );  
	
   // avoir les noms des fichiers qui sont dans le repertoire
          while ((lecture = readdir(rep))) {  

                printf("%s\n", lecture->d_name);
                sprintf(recup1, " %s", lecture->d_name);         
                strcat(list_rep , recup1);     

            }

            list_rep[strlen(list_rep)]='\n';
          // on va envoyer  list_rep au notre client;
            rio_writen(connfd, list_rep,  strlen(list_rep));        

            closedir(rep);  

	// tester si c'est pwd qui est tapé
  } else if(strcmp(argv01[0], "pwd")==0){  

  
	printf(" Serveur: commande pwd \n");
        char tampon[MAXLINE];

// on sauvegarde le resultat de fct getcwd dans le tampon;
        getcwd(tampon, MAXLINE);     
 // supp le \n
        tampon[strlen(tampon)] = '\n';    
 // on envoie le tampon  au client;
        rio_writen(connfd, tampon , MAXLINE); 

// commande cd;
  }else if ( strcmp( argv01[0] , "cd" ) == 0 ){
    
	int retour=chdir(argv01[1]);     
     
  if (retour < 0)
         
  printf("\nDirectory change failed : %s\n",strerror(errno));
 

		printf(" on est bien deplacé");
     
       
}   // commande bye pour sortir;
 else if( strcmp( argv01[0],"bye")== 0 ){

    	  exit(0);
}
  // commande mkdir ;
  if(strcmp(argv01[0], "mkdir")==0){

     char* nomD = argv01[1]; 
  
                   mkdir(nomD, 0777 ); 
   
     }


}

