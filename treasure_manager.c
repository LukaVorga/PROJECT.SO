#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_STRING_LENGTH 256

// Structura pentru o comoară
typedef struct {
    int treasure_id;
    char username[MAX_STRING_LENGTH];
    float latitude;
    float longitude;
    char clue[MAX_STRING_LENGTH];
    int value;
} Treasure;

// Funcția pentru a loga acțiunile utilizatorilor în fișierul de log
void log_action(const char *hunt_id, const char *action) {
    char log_file[MAX_STRING_LENGTH];
    snprintf(log_file, sizeof(log_file), "%s/logged_hunt.txt", hunt_id);

    FILE *log = fopen(log_file, "a");
    if (log == NULL) {
        perror("Eroare la deschiderea fișierului de log");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n", 
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, 
            t->tm_hour, t->tm_min, t->tm_sec, action);

    fclose(log);
}

// Funcția pentru a adăuga o comoară într-o vânătoare specifică
void add_treasure(const char *hunt_id, Treasure *treasure) {
    char hunt_dir[MAX_STRING_LENGTH];
    snprintf(hunt_dir, sizeof(hunt_dir), "%s", hunt_id);

    // Crearea directorului pentru vânătoare, dacă nu există deja
    if (mkdir(hunt_dir, 0700) && errno != EEXIST) {
        perror("Eroare la crearea directorului de vânătoare");
        return;
    }

    // Fișierul care va stoca comorile
    char treasure_file[MAX_STRING_LENGTH];
    snprintf(treasure_file, sizeof(treasure_file), "%s/treasures.dat", hunt_dir);

    FILE *file = fopen(treasure_file, "ab");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului de comori");
        return;
    }

    // Scrierea comorii în fișier
    fwrite(treasure, sizeof(Treasure), 1, file);
    fclose(file);

    // Logarea acțiunii
    char log_msg[MAX_STRING_LENGTH];
    snprintf(log_msg, sizeof(log_msg), "Adăugată comoară ID %d pentru utilizatorul %s", treasure->treasure_id, treasure->username);
    log_action(hunt_id, log_msg);
}

// Funcția pentru a lista toate comorile dintr-o vânătoare
void list_treasures(const char *hunt_id) {
    char hunt_dir[MAX_STRING_LENGTH];
    snprintf(hunt_dir, sizeof(hunt_dir), "%s", hunt_id);

    // Verificăm dacă directorul vânătoarei există
    struct stat statbuf;
    if (stat(hunt_dir, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        printf("Directorul vânătoarei nu există!\n");
        return;
    }

    // Deschiderea fișierului cu comori
    char treasure_file[MAX_STRING_LENGTH];
    snprintf(treasure_file, sizeof(treasure_file), "%s/treasures.dat", hunt_dir);

    FILE *file = fopen(treasure_file, "rb");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului de comori");
        return;
    }

    // Informații despre fișier
    struct stat file_stat;
    stat(treasure_file, &file_stat);
    printf("Vânătoare: %s\n", hunt_id);
    printf("Dimensiune totală: %ld bytes\n", file_stat.st_size);
    printf("Ultima modificare: %s", ctime(&file_stat.st_mtime));

    // Listarea comorilor
    Treasure treasure;
    while (fread(&treasure, sizeof(Treasure), 1, file) > 0) {
        printf("Comoară ID: %d, Utilizator: %s, Valoare: %d\n", treasure.treasure_id, treasure.username, treasure.value);
    }

    fclose(file);
}

// Funcția pentru a vizualiza o comoară specifică după ID
void view_treasure(const char *hunt_id, int treasure_id) {
    char hunt_dir[MAX_STRING_LENGTH];
    snprintf(hunt_dir, sizeof(hunt_dir), "%s", hunt_id);

    // Deschiderea fișierului cu comori
    char treasure_file[MAX_STRING_LENGTH];
    snprintf(treasure_file, sizeof(treasure_file), "%s/treasures.dat", hunt_dir);

    FILE *file = fopen(treasure_file, "rb");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului de comori");
        return;
    }

    Treasure treasure;
    while (fread(&treasure, sizeof(Treasure), 1, file) > 0) {
        if (treasure.treasure_id == treasure_id) {
            printf("Comoară ID: %d\n", treasure.treasure_id);
            printf("Utilizator: %s\n", treasure.username);
            printf("Coordonate: %.2f, %.2f\n", treasure.latitude, treasure.longitude);
            printf("Indiciu: %s\n", treasure.clue);
            printf("Valoare: %d\n", treasure.value);
            fclose(file);
            return;
        }
    }

    printf("Comoară cu ID %d nu a fost găsită!\n", treasure_id);
    fclose(file);
}

// Funcția pentru a elimina o comoară din vânătoare
void remove_treasure(const char *hunt_id, int treasure_id) {
    char hunt_dir[MAX_STRING_LENGTH];
    snprintf(hunt_dir, sizeof(hunt_dir), "%s", hunt_id);

    // Deschiderea fișierului pentru citire și scriere
    char treasure_file[MAX_STRING_LENGTH];
    snprintf(treasure_file, sizeof(treasure_file), "%s/treasures.dat", hunt_dir);

    FILE *file = fopen(treasure_file, "rb+");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului de comori");
        return;
    }

    FILE *temp_file = fopen("temp.dat", "wb");
    if (temp_file == NULL) {
        perror("Eroare la crearea fișierului temporar");
        fclose(file);
        return;
    }

    Treasure treasure;
    int found = 0;
    while (fread(&treasure, sizeof(Treasure), 1, file) > 0) {
        if (treasure.treasure_id != treasure_id) {
            fwrite(&treasure, sizeof(Treasure), 1, temp_file);
        } else {
            found = 1;
        }
    }

    fclose(file);
    fclose(temp_file);

    // Dacă comoara a fost găsită, înlocuim fișierul original cu fișierul temporar
    if (found) {
        remove(treasure_file);
        rename("temp.dat", treasure_file);
        printf("Comoară ID %d eliminată.\n", treasure_id);
    } else {
        printf("Comoară ID %d nu a fost găsită!\n", treasure_id);
        remove("temp.dat");
    }

    // Logarea acțiunii
    char log_msg[MAX_STRING_LENGTH];
    snprintf(log_msg, sizeof(log_msg), "Eliminată comoară ID %d", treasure_id);
    log_action(hunt_id, log_msg);
}

// Funcția pentru a elimina întreaga vânătoare și comorile asociate
void remove_hunt(const char *hunt_id) {
    char hunt_dir[MAX_STRING_LENGTH];
    snprintf(hunt_dir, sizeof(hunt_dir), "%s", hunt_id);

    // Ștergerea directorului vânătoarei
    if (rmdir(hunt_dir) == -1) {
        perror("Eroare la ștergerea directorului de vânătoare");
    } else {
        printf("Vânătoare %s eliminată.\n", hunt_id);

        // Logarea acțiunii
        char log_msg[MAX_STRING_LENGTH];
        snprintf(log_msg, sizeof(log_msg), "Eliminată întreaga vânătoare %s", hunt_id);
        log_action(hunt_id, log_msg);
    }
}

// Funcția principală pentru a gestiona comenzile de la linia de comandă
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Utilizare: %s <operațiune> <hunt_id> [argumente suplimentare]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *operation = argv[1];
    const char *hunt_id = argv[2];

    if (strcmp(operation, "add") == 0) {
        if (argc < 8) {
            fprintf(stderr, "Utilizare: %s add <hunt_id> <treasure_id> <username> <latitude> <longitude> <clue> <value>\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        Treasure treasure;
        treasure.treasure_id = atoi(argv[3]);
        strncpy(treasure.username, argv[4], MAX_STRING_LENGTH);
        treasure.latitude = atof(argv[5]);
        treasure.longitude = atof(argv[6]);
        strncpy(treasure.clue, argv[7], MAX_STRING_LENGTH);
        treasure.value = atoi(argv[8]);

        add_treasure(hunt_id, &treasure);
    } else if (strcmp(operation, "list") == 0) {
        list_treasures(hunt_id);
    } else if (strcmp(operation, "view") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Utilizare: %s view <hunt_id> <treasure_id>\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        int treasure_id = atoi(argv[3]);
        view_treasure(hunt_id, treasure_id);
    } else if (strcmp(operation, "remove_treasure") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Utilizare: %s remove_treasure <hunt_id> <treasure_id>\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        int treasure_id = atoi(argv[3]);
        remove_treasure(hunt_id, treasure_id);
    } else if (strcmp(operation, "remove_hunt") == 0) {
        remove_hunt(hunt_id);
    } else {
        fprintf(stderr, "Operațiune invalidă: %s\n", operation);
        exit(EXIT_FAILURE);
    }

    return 0;
}
