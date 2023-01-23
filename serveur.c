// cd c:/users/Dell/Documents/'L3 SDN'/systeme_et_reseau/projet
// cd /cygdrive/d/Cours_SdN/L3_Cours_S5/Systeme_Reseaux/Projet

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345
#define BACKLOG 5
#define MAX_MESSAGE_LENGTH 1024

int NumeroDuFichier = 0;

void *handle_connection(void *arg)
{
  // Récupération de la socket de communication avec le client
  int client_fd = *((int *)arg);
  char message[MAX_MESSAGE_LENGTH];       // message envoyé par le client
  int num_bytes_received;                 // nombre d'octets reçus
  char nomFic[20];                        // nom du fichier
  sprintf(nomFic, "%d", NumeroDuFichier); // conversion du numéro du fichier en chaîne de caractères
  NumeroDuFichier++;                      // incrémentation du numéro du fichier
  // Écriture du message dans un fichier sur le serveur
  FILE *file = fopen(strcat(nomFic, ".txt"), "a");
  // Boucle de communication avec le client
  while (1)
  {
    // Réception du message envoyé par le client
    num_bytes_received = recv(client_fd, message, MAX_MESSAGE_LENGTH, 0);
    if (num_bytes_received == -1)
    {
      printf("Le client a fermé la connexion\n");
      break;
    }
    else if (num_bytes_received == 0)
    {
      printf("Le client a fermé la connexion\n");
      break;
    }

    // Ajout du caractère de fin de chaîne au message
    message[num_bytes_received] = '\0';

    printf("Message reçu du client %d : %s\n", ntohs(client_fd), message);

    // Écriture du message dans le fichier
    fputs(message, file);

    // Envoi d'un message de confirmation au client
    if (send(client_fd, "Message reçu avec succès\n", 26, 0) == -1)
    {
      perror("Erreur lors de l'envoi d'un message de confirmation au client");
      break;
    }
  }
  fclose(file);
  // Fermeture de la socket de communication avec le client
  close(client_fd);

  return NULL;
}

int main(void)
{
  int listen_fd;                               // socket d'écoute
  struct sockaddr_in server_addr, client_addr; // adresses du serveur et du client
  socklen_t client_addr_len;                   // taille de l'adresse du client
  pthread_t thread;                            // identifiant du thread

  // Création de la socket d'écoute
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == -1)
  {
    perror("Erreur lors de la création de la socket");
    exit(EXIT_FAILURE);
  }

  // Configuration de l'adresse du serveur
  memset(&server_addr, 0, sizeof(server_addr)); // initialisation de la structure à 0
  server_addr.sin_family = AF_INET;             // famille d'adresses
  server_addr.sin_addr.s_addr = INADDR_ANY;     // adresse IP automatique
  server_addr.sin_port = htons(PORT);           // numéro de port

  // Liaison de la socket à l'adresse du serveur
  if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
  {
    perror("Erreur lors de la liaison de la socket à l'adresse du serveur");
    exit(EXIT_FAILURE);
  }

  // Mise en écoute de la socket
  if (listen(listen_fd, BACKLOG) == -1)
  {
    perror("Erreur lors de la mise en écoute de la socket");
    exit(EXIT_FAILURE);
  }

  printf("Le serveur est en écoute sur le port %d\n", PORT);

  // Boucle principale du serveur
  while (1)
  {
    // Acceptation d'une connexion entrante
    client_addr_len = sizeof(client_addr);                                             // initialisation de la taille de l'adresse du client
    int *client_fd = malloc(sizeof(int));                                              // socket de communication avec le client
    *client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len); // acceptation de la connexion
    if (*client_fd == -1)
    {
      perror("Erreur lors de l'acceptation d'une connexion entrante");
      free(client_fd);
      continue;
    }

    printf("Connexion acceptée de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); // affichage de l'adresse IP et du numéro de port du client

    // Création d'un thread pour gérer la connexion avec le client
    if (pthread_create(&thread, NULL, handle_connection, client_fd) != 0)
    {
      perror("Erreur lors de la création du thread");
      close(*client_fd);
      free(client_fd);
      continue;
    }

    // Le thread principal ne doit pas terminer avant les threads créés
    pthread_detach(thread);
  }

  // Fermeture de la socket d'écoute
  close(listen_fd);

  return 0;
}