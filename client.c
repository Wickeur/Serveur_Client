#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define MAX_MESSAGE_LENGTH 80

int main(void)
{
    int sock_fd;                      // socket fichier description
    struct sockaddr_in server_addr;   // adresse socket
    char message[MAX_MESSAGE_LENGTH]; // message
    int num_bytes_sent;               // nombre de bytes envoyés (permet l'envoi au serveur)
    int letsSpeak = 1;                // maintient la boucle ouverte
    int peutonEcrire = 0;             // pour savoir si nous pouvons ecrire
    int messageErreur = 1;            // pour savoir si nous avons une erreur
    int enNormal = 1;                 // pour savoir si nous sommes en mode default
    int enMajuscule = 0;              // pour savoir si nous sommes en majuscule
    int enMinuscule = 0;              // pour savoir si nous sommes en minuscule
    int blocOuvert = 0;               // pour savoir si nous avons ouvert un bloc
    int premiereLigneDuBloc = 0;      // pour savoir si nous sommes à la première ligne du nouveau bloc (si c'est le cas, ce int empeche l'apparrition de | sur la ligne marquant l'ouverture du bloc : +------+)
    int retourALaLigne = 0;           // pour savoir si on effectue un retour a la ligne
    int premierMessage = 1;           // pour savoir si nous sommes à la première ligne de notre fichier (sert dans certaines situations)
    int dernierMessage = 0;           // pour savoir si nous sommes à la dernière ligne de notre fichier (sert dans certaines situations)
    char leMode[10];                  // chaine de caractère permettant  de stocker le nom du mode d'ecriture en cours d'utilisation (celui activé)
    int modeIndentation = 0;          // pour savoir si nous sommes en mode indentation
    int premierMotLigne = 1;          // pour savoir si nous sommes au premier mot de la ligne (sert dans certaines situations)
    int modePuce = 0;                 // pour savoir si nous sommes en mode puce
    int taille_bloc = 78;             // pour savoir la taille du bloc
    int nb_bloc_ouvert = 0;           // pour savoir le nombre de bloc ouvert

    // Création de la socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET = IPv4, SOCK_STREAM = TCP
    if (sock_fd == -1)
    {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));       // remplir de 0
    server_addr.sin_family = AF_INET;                   // IPv4
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); // adresse IP
    server_addr.sin_port = htons(PORT);                 // port

    // Connexion serveur
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Erreur lors de la connexion serveur");
        exit(EXIT_FAILURE);
    }

    printf("Connexion serveur réussie\n");

    // Boucle de saisie et d'envoi de messages
    while (letsSpeak == 1)
    {
        messageErreur = 1;

        // Saisie du message à envoyer
        printf("Que voulez vous faire ?\n");
        printf("0 - Quitter\n1 - Ouvrir un block\n2 - Fermer un block\n3 - Indenter\n4 - Desindenter\n5 - Nouvelle_Ligne\n6 - Pucer\n7 - Ecrire\n");
        printf("Votre choix : \n");
        fgets(message, MAX_MESSAGE_LENGTH, stdin); // ecrire dans message
        char *token = strtok(message, "\n");       // on coupe le message à la fin de la ligne

        // 1 - Ouverture Bloc
        if (strcmp(message, "1") == 0)
        {
            messageErreur = 0;
            peutonEcrire = 1;
            blocOuvert = 1;
            premiereLigneDuBloc = 1;

            // Définir la taille du bloc
            if (nb_bloc_ouvert != 0)
            {
                taille_bloc -= 4;
            }

            // Si on a écrit qqch avant d'ouvrir un bloc
            if (premierMessage != 1)
            {
                if (nb_bloc_ouvert != 0)
                {
                    strcpy(message, "\n|");
                    for (int i = 0; i < nb_bloc_ouvert; i++)
                    {
                        strcat(message, "|");
                    }
                    strcat(message, "+");
                }
                if (nb_bloc_ouvert == 0)
                {
                    strcpy(message, "\n+");
                }
                for (int i = 0; i < taille_bloc; i++)
                {
                    strcat(message, "-");
                }
                strcat(message, "+");
            }
            // Sinon
            if (premierMessage == 1)
            {
                strcpy(message, "+");
                for (int i = 0; i < taille_bloc; i++)
                {
                    strcat(message, "-");
                }
                strcat(message, "+");
            }
            premierMessage = 0;
            printf("Vous avez ouvert un nouveau bloc \n\n");
            nb_bloc_ouvert++;
        }

        // 2 - Fermeture Bloc
        if (strcmp(message, "2") == 0 && nb_bloc_ouvert > 0)
        {
            messageErreur = 0;
            blocOuvert = 0;
            peutonEcrire = 1;

            if (nb_bloc_ouvert > 1)
            {
                strcpy(message, "\n");
                for (int i = 0; i < nb_bloc_ouvert; i++)
                {
                    strcat(message, "|");
                }
                strcat(message, "+");
            }
            if (nb_bloc_ouvert == 0 || nb_bloc_ouvert == 1)
            {
                strcpy(message, "\n+");
            }
            for (int i = 0; i < taille_bloc; i++)
            {
                strcat(message, "-");
            }
            strcat(message, "+\n");

            for (int i = 1; i < nb_bloc_ouvert; i++)
            {
                strcat(message, "|");
            }

            printf("Vous avez ferme un bloc\n\n");

            // Définir la taille du bloc
            if (nb_bloc_ouvert != 1)
            {
                taille_bloc += 4;
            }

            premierMotLigne = 1;
            nb_bloc_ouvert--;
        }
        if (strcmp(message, "2") == 0 && nb_bloc_ouvert == 0)
        {
            printf("Impossible, vous n'avez pas de bloc ouvert \n\n");
        }

        // 3 - Indenter
        if (strcmp(message, "3") == 0 && premierMotLigne == 1)
        {
            modeIndentation = 1;
            peutonEcrire = 1;
            messageErreur = 0;

            strcpy(message, "\t");

            printf("Vous avez indente \n");
        }
        if (strcmp(message, "3") == 0 && premierMotLigne == 0)
        {
            printf("Impossible, veuillez retourner à la ligne \n\n");
        }

        // 4 - Desindenter
        if (strcmp(message, "4") == 0 && modeIndentation == 0)
        {
            printf("Impossible, vous n'etes pas en mode indentation \n\n");
        }
        if (strcmp(message, "4") == 0 && modeIndentation == 1)
        {
            messageErreur = 0;
            modeIndentation = 0;
            printf("Vous avez desindente \n");
        }

        // 5 - Nouvelle Ligne

        if (strcmp(message, "5") == 0)
        {
            retourALaLigne = 1;
            premierMotLigne = 1;
            modePuce = 0;
        }

        // 6 - Pucer
        if (strcmp(message, "6") == 0 && premierMotLigne == 1)
        {
            modePuce = 1;
            messageErreur = 0;
            peutonEcrire = 1;
            strcpy(message, "  - ");
            printf("Vous avez puce \n");
        }
        if (strcmp(message, "6") == 0 && premierMotLigne == 0)
        {
            printf("Impossible, veuillez retourner à la ligne \n\n");
        }

        // Si on ecrit 7, on demande le texte à ecrire

        if (strcmp(message, "7") == 0)
        {
            messageErreur = 0;
            peutonEcrire = 1;
            int modeEcriture = 0;
            if (enNormal == 1)
            {
                strcpy(leMode, "Normal");
            }
            if (enMajuscule == 1)
            {
                strcpy(leMode, "Majuscule");
            }
            if (enMinuscule == 1)
            {
                strcpy(leMode, "Minuscule");
            }

            printf("\nQuel texte voulez vous ecrire ?\n");
            printf("Vous écrivez par défaut en mode %s\n(8 - Majuscule, 9 - Minuscule, 10 - Normal)\n", leMode);
            fgets(message, MAX_MESSAGE_LENGTH, stdin); // ecrire dans message
            // strcat("|", message);
            char *token = strtok(message, "\n"); // on coupe le message à la fin de la ligne

            if (strcmp(message, "8") == 0)
            {
                // Tout ce qu'on écrit va être en majuscule dans le fichier
                printf("\nVous écrivez en mode majuscule :\n");
                fgets(message, MAX_MESSAGE_LENGTH, stdin); // ecrire dans message
                char *token = strtok(message, "\n");       // on coupe le message à la fin de la ligne
                enMajuscule = 1;
                enMinuscule = 0;
                enNormal = 0;
            }
            if (strcmp(message, "9") == 0)
            {
                // Tout ce qu'on écrit va être en minuscule dans le fichier
                printf("\nVous écrivez en mode minuscule :\n");
                fgets(message, MAX_MESSAGE_LENGTH, stdin); // ecrire dans message
                char *token = strtok(message, "\n");       // on coupe le message à la fin de la ligne
                enMajuscule = 0;
                enMinuscule = 1;
                enNormal = 0;
            }
            if (strcmp(message, "10") == 0)
            {
                // Tout ce qu'on écrit va être en minuscule dans le fichier
                printf("\nVous écrivez en mode normal :\n");
                fgets(message, MAX_MESSAGE_LENGTH, stdin); // ecrire dans message
                char *token = strtok(message, "\n");       // on coupe le message à la fin de la ligne
                enMajuscule = 0;
                enMinuscule = 0;
                enNormal = 1;
            }
            printf("\n");
            premierMotLigne = 0;
        }

        // Pour quiter le programme
        if (strcmp(message, "0") == 0)
        {
            messageErreur = 0;
            letsSpeak = 0;
        }

        // Envoi du message au serveur

        if (letsSpeak == 1 && peutonEcrire == 1)
        {
            peutonEcrire = 0;
            if (enMajuscule == 1)
            {
                num_bytes_sent = send(sock_fd, strupr(message), strlen(message), 0); // num_bytes_sent = nombre de bytes envoyés
                if (num_bytes_sent == -1)
                {
                    perror("Erreur lors de l'envoi du message au serveur");
                    break;
                }
                printf("Message envoyé au serveur\n\n");
            }
            if (enMinuscule == 1)
            {
                num_bytes_sent = send(sock_fd, strlwr(message), strlen(message), 0); // num_bytes_sent = nombre de bytes envoyés
                if (num_bytes_sent == -1)
                {
                    perror("Erreur lors de l'envoi du message au serveur");
                    break;
                }
                printf("Message envoyé au serveur\n\n");
            }
            if (enNormal == 1)
            {
                num_bytes_sent = send(sock_fd, message, strlen(message), 0); // num_bytes_sent = nombre de bytes envoyés
                if (num_bytes_sent == -1)
                {
                    perror("Erreur lors de l'envoi du message au serveur");
                    break;
                }
                printf("Message envoyé au serveur\n\n");
            }

            // Envoi d'un espace si on est au début d'une ligne
            if (premierMotLigne == 0)
            {
                char espace[5] = " ";
                num_bytes_sent = send(sock_fd, espace, strlen(espace), 0); // num_bytes_sent = nombre de bytes envoyéss
            }
        }

        // Permet de retourner à la ligne et qui le | du début la ligne
        if (retourALaLigne == 1 || premiereLigneDuBloc == 1)
        {
            retourALaLigne = 0;
            premiereLigneDuBloc = 0;

            char backslashN[5] = "\n";
            num_bytes_sent = send(sock_fd, backslashN, strlen(backslashN), 0);
            if (blocOuvert == 1)
            {
                char barreDu6[100] = "|";
                if (nb_bloc_ouvert > 1)
                {
                    for (int i = 0; i < nb_bloc_ouvert; i++)
                    {
                        strcat(barreDu6, "|");
                    }
                }
                num_bytes_sent = send(sock_fd, barreDu6, strlen(barreDu6), 0);
            }
        }

        else
        {
            if (messageErreur == 1)
            {
                printf("Commande inconnu \n\n");
            }
        }
        premierMessage = 0;
    }

    // Fermeture de la socket
    close(sock_fd);

    return 0;
}