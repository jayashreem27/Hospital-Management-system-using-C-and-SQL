
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <limits.h>
#include "sqlite3.h"
typedef struct info {
    char name[30];
    int pat_id;
    char dob[10];
    int phno;
    int emgphno;
    char email[50];
    char bloodt[4];
    char chronic_illness[30];
} info;

typedef struct patient_record {
    info i;
    struct patient_record *next;
} patient_record;

sqlite3 *db_pat;

void loadPatientData(patient_record** head);
void addPatientRecord(patient_record *head, int n, sqlite3 *db_pat) ;
void updatePatientRecord(patient_record *head, sqlite3 *db_pat);
void displayPatientRecord(sqlite3 *db_pat) ;
void deletePatientRecord(patient_record *head, sqlite3 *db_pat) ;

static int openDataBase() {
    int rc = sqlite3_open("patient_records.db_pat", &db_pat);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db_pat));
        sqlite3_close(db_pat);
        return rc;
    }
    return SQLITE_OK;
}

int createPatientTable() {
    char *sql = "CREATE TABLE IF NOT EXISTS patients ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "pat_id INTEGER NOT NULL,"
                "name TEXT NOT NULL,"
                "dob TEXT NOT NULL,"
                "phno INTEGER NOT NULL,"
                "emgphno INTEGER NOT NULL,"
                "email TEXT NOT NULL,"
                "bloodt TEXT NOT NULL,"
                "chronic_illness TEXT NOT NULL);";

    int rc = sqlite3_exec(db_pat, sql, 0, 0, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_pat));
        return rc;
    }
    return SQLITE_OK;
}

void loadPatientData(patient_record** head) {
    // Open the database connection
    sqlite3* db_pat;
    if (sqlite3_open("patient_records.db_pat", &db_pat) != SQLITE_OK) {
        fprintf(stderr, "Cannot open patient database: %s\n", sqlite3_errmsg(db_pat));
        sqlite3_close(db_pat);
        return;
    }

    // Create the patients table if it doesn't exist
    createPatientTable();

    // SQL query to retrieve all records from the patients table
    const char* sql = "SELECT * FROM patients;";

    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_pat, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_pat));
        sqlite3_close(db_pat);
        return;
    }

    // Fetch records and add them to the linked list
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        info patientInfo;

        // Extract values from the result set
        strcpy(patientInfo.name, (const char*)sqlite3_column_text(stmt, 2));
        patientInfo.pat_id = sqlite3_column_int(stmt, 1);
        strcpy(patientInfo.dob, (const char*)sqlite3_column_text(stmt, 3));
        patientInfo.phno = sqlite3_column_int(stmt, 4);
        patientInfo.emgphno = sqlite3_column_int(stmt, 5);
        strcpy(patientInfo.email, (const char*)sqlite3_column_text(stmt, 6));
        strcpy(patientInfo.bloodt, (const char*)sqlite3_column_text(stmt, 7));
        strcpy(patientInfo.chronic_illness, (const char*)sqlite3_column_text(stmt, 8));

        // Create a new node with patient information and add it to the linked list
        patient_record* newNode = (patient_record*)malloc(sizeof(patient_record));
        if (newNode == NULL) {
            // Handle memory allocation failure
            exit(EXIT_FAILURE);
        }
        newNode->i = patientInfo;
        newNode->next = *head;
        *head = newNode;
    }

    // Finalize the statement and close the database connection
    sqlite3_finalize(stmt);
    sqlite3_close(db_pat);
}


patient_record *createNodeP(info *i) {
    patient_record *nN = malloc(sizeof(patient_record));
    if (nN == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }
    nN->i = *i;
    nN->next = NULL;
    return nN;
}

void addPatientRecord(patient_record *head, int n, sqlite3 *db_pat) {
    for (int i = 0; i < n; ++i) {

        // Read patient information from user input...
        info patient_info;

        printf("Enter patient name: ");
        scanf("%s", patient_info.name);
        
        printf("Enter patient ID: ");
        scanf("%d", &patient_info.pat_id);
        patient_record *current=head;

        while (current != NULL) {

            if (patient_info.pat_id == current->i.pat_id) {
                printf("Patient with this ID exists\n");
                return;
            }

            current = current->next;
        }

        printf("Enter patient date of birth (DD-MM-YYYY): ");
        scanf("%s", patient_info.dob);
        if (strlen(patient_info.dob) > 10) {
            patient_info.dob[10] = '\0';
        }
        printf("Enter patient phone number: ");
scanf("%d", &patient_info.phno);

int count = 0;
int num = patient_info.phno;

while (num != 0) {
    num /= 10;
    ++count;
}

if (count != 10) {
    printf("Phone number must have exactly 10 digits.\n");
    // You might want to handle this situation accordingly, like returning from the function or asking the user to enter the phone number again.
    return;
}

printf("Enter patient emergency phone number: ");
scanf("%d", &patient_info.emgphno);

// Reset count for the second phone number
count = 0;
num = patient_info.emgphno;

while (num != 0) {
    num /= 10;
    ++count;
}

if (count != 10) {
    printf("Emergency phone number must have exactly 10 digits.\n");
    // You might want to handle this situation accordingly, like returning from the function or asking the user to enter the phone number again.
    return;
}

        printf("Enter patient email: ");
        scanf("%s", patient_info.email);

        printf("Enter patient blood type: ");
        scanf("%s", patient_info.bloodt);

        printf("Enter patient chronic illness: ");
        scanf("%s", patient_info.chronic_illness);

        // Create a new node with the patient information
        patient_record *newNode = createNodeP(&patient_info);

        // Add the new node to the list
        newNode->next = head->next;
        head->next = newNode;

        // Insert the patient record into the database
        char sql[256];
        sprintf(sql, "INSERT INTO patients (name, pat_id, dob, phno, emgphno, email, bloodt, chronic_illness) "
                     "VALUES ('%s', %d, '%s', %d, %d, '%s', '%s', '%s');",
                     patient_info.name, patient_info.pat_id, patient_info.dob, patient_info.phno,
                     patient_info.emgphno, patient_info.email, patient_info.bloodt, patient_info.chronic_illness);

        int rc = sqlite3_exec(db_pat, sql, 0, 0, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_pat));
        } else {
            printf("Patient record added successfully!\n");
        }
    }
}

void updatePatientRecord(patient_record *head, sqlite3 *db_pat) {
    int id;
    printf("Enter the patient ID you want to update: ");
    scanf("%d", &id);

    patient_record *current = head->next;
    while (current != NULL) {
        if (current->i.pat_id == id) {
            printf("Enter updated patient name: ");
            scanf("%s", current->i.name);

            printf("Enter updated patient date of birth (DD-MM-YYYY): ");
            scanf("%s", current->i.dob);

            printf("Enter updated patient phone number: ");
            scanf("%d", &current->i.phno);

            printf("Enter updated patient emergency phone number: ");
            scanf("%d", &current->i.emgphno);

            printf("Enter updated patient email: ");
            scanf("%s", current->i.email);

            printf("Enter updated patient blood type: ");
            scanf("%s", current->i.bloodt);

            printf("Enter updated patient chronic illness: ");
            scanf("%s", current->i.chronic_illness);

            // Update the patient record in the database
            char sql[256];
            sprintf(sql, "UPDATE patients SET name='%s', dob='%s', phno=%d, emgphno=%d, "
                         "email='%s', bloodt='%s', chronic_illness='%s' WHERE pat_id=%d;",
                         current->i.name, current->i.dob, current->i.phno, current->i.emgphno,
                         current->i.email, current->i.bloodt, current->i.chronic_illness, current->i.pat_id);

            int rc = sqlite3_exec(db_pat, sql, 0, 0, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_pat));
            } else {
                printf("Patient record updated successfully!\n");
            }
            return;
        }
        current = current->next;
    }
    printf("Patient with ID %d not found.\n", id);
}

void displayPatientRecord(sqlite3 *db_pat) {
    int id;
    printf("Enter the patient ID you want to display: ");
    scanf("%d", &id);

    char sql[128];
    sprintf(sql, "SELECT * FROM patients WHERE pat_id=%d;", id);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_pat, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            printf("Patient ID: %d\n", sqlite3_column_int(stmt, 1));
            printf("Name: %s\n", sqlite3_column_text(stmt, 2));
            printf("Date of Birth: %s\n", sqlite3_column_text(stmt, 3));
            printf("Phone Number: %d\n", sqlite3_column_int(stmt, 4));
            printf("Emergency Phone Number: %d\n", sqlite3_column_int(stmt, 5));
            printf("Email: %s\n", sqlite3_column_text(stmt, 6));
            printf("Blood Type: %s\n", sqlite3_column_text(stmt, 7));
            printf("Chronic Illness: %s\n", sqlite3_column_text(stmt, 8));
        } else {
            printf("Patient with ID %d not found.\n", id);
        }
    } else {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_pat));
    }
    sqlite3_finalize(stmt);
}

void deletePatientRecord(patient_record *head, sqlite3 *db_pat) {
    int id;
    printf("Enter the patient ID you want to delete: ");
    scanf("%d", &id);

    patient_record *current = head->next;
    patient_record *prev = head;

    while (current != NULL) {
        if (current->i.pat_id == id) {
            // Delete the record from the database
            char sql[128];
            sprintf(sql, "DELETE FROM patients WHERE pat_id=%d;", id);

            int rc = sqlite3_exec(db_pat, sql, 0, 0, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_pat));
            }

            // Delete the record from the linked list
            prev->next = current->next;
            free(current);
            printf("Patient with ID %d deleted successfully.\n", id);
            return;
        }
        prev = current;
        current = current->next;
    }

    printf("Patient with ID %d not found.\n", id);
}

info* searchPatientRecord(patient_record* head, int id) {
    patient_record* current = head;
    while (current != NULL) {

        if (current->i.pat_id == id) {
            return &(current->i); // Return the info struct if patient ID matches
        }
        current = current->next;
    }
    return NULL; // Return NULL if patient with given ID is not found
}