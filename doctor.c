#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <limits.h>

#include "sqlite3.h"

typedef struct DoctorInfo {
    int doc_id;
    char name[30];
    int phno;
    char email[50];
    char dep[20];
    char license[20];
    int stime;
    int etime;
    int num_slots;
    int *occupied_slots;
    char** occupied_events;
} DoctorInfo;


typedef struct doctor_record {
    DoctorInfo info;
    struct doctor_record* next;
} doctor_record;

sqlite3* db_doc;

void loadDoctorData(doctor_record** head_doc);
void removeFromDatabase(int doc_id);
void addDoctorRecord(doctor_record* head_doc, int n, sqlite3* db_doc);
void deleteDoctorRecord(doctor_record** head_doc);
doctor_record *assignDoc(char dep[], doctor_record *head_doc);


int createDoctorTable() {
    char *sql = "CREATE TABLE IF NOT EXISTS doctor_record ("
                "doc_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "name TEXT NOT NULL,"
                "phno INTEGER NOT NULL,"
                "email TEXT NOT NULL,"
                "dep TEXT NOT NULL,"
                "license TEXT NOT NULL,"
                "stime INTEGER NOT NULL,"
                "etime INTEGER NOT NULL"
                ");";

    int rc = sqlite3_exec(db_doc, sql, 0, 0, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s \n", sqlite3_errmsg(db_doc));
        return rc;
    }
    return SQLITE_OK;
}
static int openDatabaseD() {
    int rc = sqlite3_open("doctor_records.db_doc", &db_doc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db_doc));
        sqlite3_close(db_doc);
        return rc;
    }
    return SQLITE_OK;
}

DoctorInfo* createDoctorInfo(doctor_record* head_doc) 
{
    DoctorInfo* newDoctorInfo = malloc(sizeof(DoctorInfo));
    if (newDoctorInfo == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }

    // Initialize doctor's information
    printf("Enter doctor ID: ");
    scanf("%d", &newDoctorInfo->doc_id);

    doctor_record* current = head_doc;
    while (current != NULL) {
        if (current->info.doc_id == newDoctorInfo->doc_id) {
            printf("Doctor with ID %d already exists.\n", newDoctorInfo->doc_id);
            return NULL; // Return NULL to indicate failure
        }
        current = current->next;
    
    }
    printf("Enter doctor name: ");
    scanf("%s", newDoctorInfo->name);

    printf("Enter doctor phone number: ");
    scanf("%d", &newDoctorInfo->phno);

    printf("Enter doctor email: ");
    scanf("%s", newDoctorInfo->email);

    printf("Enter doctor department: ");
    scanf("%s", newDoctorInfo->dep);

    printf("Enter doctor license: ");
    scanf("%s", newDoctorInfo->license);

    printf("Enter doctor's start time: ");
    scanf("%d", &newDoctorInfo->stime);

    printf("Enter doctor's end time: ");
    scanf("%d", &newDoctorInfo->etime);

    newDoctorInfo->num_slots = newDoctorInfo->etime - newDoctorInfo->stime;
   
    newDoctorInfo->occupied_slots = malloc(newDoctorInfo->num_slots * sizeof(int));
    if (newDoctorInfo->occupied_slots == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }

    newDoctorInfo->occupied_events = malloc(newDoctorInfo->num_slots * sizeof(char*));
    if (newDoctorInfo->occupied_events == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < newDoctorInfo->num_slots; ++i) {
        newDoctorInfo->occupied_events[i] = malloc(100 * sizeof(char)); // Adjust size as needed
        if (newDoctorInfo->occupied_events[i] == NULL) {
            // Handle memory allocation failure
            exit(EXIT_FAILURE);
        }
        strcpy(newDoctorInfo->occupied_events[i], "NIL"); // Initialize events as empty strings
        newDoctorInfo->occupied_slots[i] = 0;
    }

    return newDoctorInfo;
}


void loadDoctorData(doctor_record** head_doc) {
    // Open the database connection
    sqlite3* db_doc;
    if (sqlite3_open("doctor_records.db_doc", &db_doc) != SQLITE_OK) {
        fprintf(stderr, "Cannot open doctor database: %s\n", sqlite3_errmsg(db_doc));
        sqlite3_close(db_doc);
        return;
    }

    // Create the doctor_record table if it doesn't exist
    createDoctorTable();

    // SQL query to retrieve all records from the doctor_record table
    const char* sql = "SELECT * FROM doctor_record;";

    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_doc, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_doc));
        sqlite3_close(db_doc);
        return;
    }

    // Fetch records and add them to the linked list
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        DoctorInfo doctorInfo;

        // Extract values from the result set
            doctorInfo.doc_id = sqlite3_column_int(stmt, 0);

        strcpy(doctorInfo.name, (const char*)sqlite3_column_text(stmt, 1));
        doctorInfo.phno = sqlite3_column_int(stmt, 2);
        strcpy(doctorInfo.email, (const char*)sqlite3_column_text(stmt, 3));
        strcpy(doctorInfo.dep, (const char*)sqlite3_column_text(stmt, 4));
        strcpy(doctorInfo.license, (const char*)sqlite3_column_text(stmt, 5));
        doctorInfo.stime = sqlite3_column_int(stmt, 6);
        doctorInfo.etime = sqlite3_column_int(stmt, 7);

    doctorInfo.num_slots = doctorInfo.etime - doctorInfo.stime;
   
    doctorInfo.occupied_slots = malloc(doctorInfo.num_slots * sizeof(int));
    if (doctorInfo.occupied_slots == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }

    doctorInfo.occupied_events = malloc(doctorInfo.num_slots * sizeof(char*));
    if (doctorInfo.occupied_events == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < doctorInfo.num_slots; ++i) {
        doctorInfo.occupied_events[i] = malloc(100 * sizeof(char)); // Adjust size as needed
        if (doctorInfo.occupied_events[i] == NULL) {
            // Handle memory allocation failure
            exit(EXIT_FAILURE);
        }
        strcpy(doctorInfo.occupied_events[i], "NIL"); // Initialize events as empty strings
        doctorInfo.occupied_slots[i] = 0;
    }

        // Create a new node with doctor information and add it to the linked list
        doctor_record* newNode = (doctor_record*)malloc(sizeof(doctor_record));
        if (newNode == NULL) {
            // Handle memory allocation failure
            exit(EXIT_FAILURE);
        }
        newNode->info = doctorInfo;
        newNode->next = *head_doc;
        *head_doc = newNode;
    }

    // Finalize the statement and close the database connection
    sqlite3_finalize(stmt);
    sqlite3_close(db_doc);
}


void insertDoctorRecordIntoDatabase(sqlite3* db_doc, DoctorInfo* doctorInfo) {
    char sql[500];
    sprintf(sql, "INSERT INTO doctor_record (doc_id, name, phno, email, dep, license, stime, etime) VALUES (%d, '%s', %d, '%s', '%s', '%s', %d, %d)",
            doctorInfo->doc_id, doctorInfo->name, doctorInfo->phno, doctorInfo->email, doctorInfo->dep, doctorInfo->license, doctorInfo->stime, doctorInfo->etime);

    if (sqlite3_exec(db_doc, sql, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_doc));
    } else {
        printf("Doctor record inserted successfully!\n");
    }
}



// Function to add a doctor record to the linked list and database
void addDoctorRecord(doctor_record* head_doc, int n, sqlite3* db_doc) {
    for (int i = 0; i < n; ++i) {
        DoctorInfo* doctorInfo = createDoctorInfo(head_doc);
        if(doctorInfo ==NULL)
        return;
        // Create a new node with the doctor information
        doctor_record* newNode = malloc(sizeof(doctor_record));
        if (newNode == NULL) {
            // Handle memory allocation failure
            exit(EXIT_FAILURE);
        }
        newNode->info = *doctorInfo;
        newNode->next = NULL;

        // Add the new node to the list
        newNode->next = head_doc->next;
        head_doc->next = newNode;

        // Add the new doctor record to the database
        insertDoctorRecordIntoDatabase(db_doc, &newNode->info);
    }
}
void displayDoctorRecordByID(doctor_record *head_doc, sqlite3 *db_doc) {
    int id;
    printf("Enter Doctor ID: ");
    scanf("%d", &id);

    char sql[128];
    sprintf(sql, "SELECT * FROM doctor_record WHERE doc_id=%d;", id);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_doc, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            printf("Doctor ID: %d\n", sqlite3_column_int(stmt, 0));
            printf("Name: %s\n", sqlite3_column_text(stmt, 1));
            printf("Phone Number: %d\n", sqlite3_column_int(stmt, 2));
            printf("Email: %s\n", sqlite3_column_text(stmt, 3));
            printf("Department: %s\n", sqlite3_column_text(stmt, 4));
            printf("License: %s\n", sqlite3_column_text(stmt, 5));
            printf("Start Time: %d\n", sqlite3_column_int(stmt, 6));
            printf("End Time: %d\n", sqlite3_column_int(stmt, 7));
            
        } else {
            printf("Doctor with ID %d not found.\n", id);
        }
    } else {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_doc));
    }

    sqlite3_finalize(stmt);
}


doctor_record *assignDoc(char dep[], doctor_record *head_doc) {
    doctor_record *temp = head_doc;
    while (temp != NULL) {
        if (strcmp(temp->info.dep, dep) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// Function to remove a doctor record from the linked list and database
void deleteDoctorRecord(doctor_record** head_doc) {
    doctor_record *current = *head_doc, *prev = NULL;
    int doc_id;
    printf("Enter Doctor ID\n");
    scanf("%d", &doc_id);

    while (current != NULL) {
        current = current->next;
    }
    printf("\n");

    // Reset current to the beginning of the list
    current = *head_doc;

    // Traverse the list to find the node with the given doctor ID
    while (current != NULL && current->info.doc_id != doc_id) {
        prev = current;
        current = current->next;
    }

    // If the doctor ID is not found
    if (current == NULL) {
        printf("Doctor with ID %d not found.\n", doc_id);
        return;
    }

    // Remove the node from the linked list
    if (prev == NULL) {
        // If the node to be deleted is the head
        *head_doc = current->next;
    } else {
        prev->next = current->next;
    }

    // Remove the doctor record from the database
    removeFromDatabase(doc_id);

    // Free the memory occupied by the node
    free(current);
    printf("Doctor with ID %d deleted successfully!\n", doc_id);
}

// Function to remove a doctor record from the database
void removeFromDatabase(int doc_id) {
    char sql[100];
    sprintf(sql, "DELETE FROM doctor_record WHERE doc_id=%d", doc_id);

    if (sqlite3_exec(db_doc, sql, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db_doc));
    }
}



