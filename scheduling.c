#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>
#include <limits.h>

#include "doctor.c"
#include "patient.c"
#include "inventory.c"
#include "sqlite3.h"

// Forward declarations
typedef struct appointment appointment;
typedef struct appointmentHashTableNode appointmentHashTableNode;

void insertAppointment(appointmentHashTableNode* hashTable[], int key, appointment* newAppointment) ;
void cancelAppointment(appointmentHashTableNode* hashTable[], int key) ;
appointment* findAppointment(appointmentHashTableNode* hashTable[], int key);
void scheduleappointment(appointmentHashTableNode* hashTable[], doctor_record* head_doc, patient_record* head_pat, InventoryItem* head_inventory) ;
int availableslots(doctor_record* doctor, InventoryItem* inventory) ;


// Define the hash table size
#define HASH_TABLE_SIZE 100

// Define the structure for the hash table node
struct appointmentHashTableNode {
    int key;
    appointment* info;
    struct appointmentHashTableNode* next;
};

// Define the structure for an appointment
struct appointment {
    int type;
    info* patient;
    doctor_record* doctor;
    int slotnumber;
};

// Hash function
int hashFunction(int key) {
    return key % HASH_TABLE_SIZE;
}

// Function to insert an appointment into the hash table
void insertAppointment(appointmentHashTableNode* hashTable[], int key, appointment* newAppointment) {
    int index = hashFunction(key);

    // Create a new node for the appointment
    appointmentHashTableNode* newNode = (appointmentHashTableNode*)malloc(sizeof(appointmentHashTableNode));
    if (newNode == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }

    newNode->key = key;
    newNode->info = newAppointment;
    newNode->next = NULL;

    // Handle collision using chaining
    if (hashTable[index] == NULL) {
        hashTable[index] = newNode;
    } else {
        newNode->next = hashTable[index];
        hashTable[index] = newNode;
    }
}

// Function to find an appointment in the hash table

appointment* findAppointment(appointmentHashTableNode* hashTable[], int key) {
    int index = hashFunction(key);

    // Traverse the linked list at the calculated index
    appointmentHashTableNode* current = hashTable[index];
    while (current != NULL) {
        if (current->key == key) {
            return current->info;
        }
        current = current->next;
    }

    // Appointment not found
    return NULL;
}void cancelAppointment(appointmentHashTableNode* hashTable[], int key) {
    int index = hashFunction(key);

    // Handle collision using chaining
    appointmentHashTableNode* current = hashTable[index];
    appointmentHashTableNode* prev = NULL;

    while (current != NULL) {
        if (current->key == key) {
            // Found the appointment to cancel
            if (prev == NULL) {
                hashTable[index] = current->next;
            } else {
                prev->next = current->next;
            }

            // Free memory
            free(current->info); // Free the appointment information
            free(current);

            // Update the doctor's occupied slot to 0 for the canceled slot
            int slotNumber = current->info->slotnumber;
            current->info->doctor->info.occupied_slots[slotNumber - current->info->doctor->info.stime] = 0;

            printf("Appointment for patient ID %d has been canceled.\n", key);
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("No appointment found for patient ID %d.\n", key);
}

// Function to view appointments in the hash table


void viewAppointments(appointmentHashTableNode* hashTable[], int key) {
    appointment* appointment = findAppointment(hashTable, key);
    if (appointment != NULL) {
        printf("Doctor: %s\n", appointment->doctor->info.name);
        printf("Time Slot: %d\n", appointment->slotnumber);
        printf("Query: %s\n", appointment->doctor->info.occupied_events[appointment->slotnumber-appointment->doctor->info.stime]);
        printf("-----------\n");
    } else {
        printf("No appointments found for patient ID %d.\n", key);
    }
}
int availableslots(doctor_record* doctor, InventoryItem* inventory) {
    printf("\nAvailable Slots (time in hours):");
    int availableSlotCount = 0;

for (int i = doctor->info.stime; i < doctor->info.etime; i++) {
        if (doctor->info.occupied_slots[i-doctor->info.stime] == 0) {
            printf(" %d,", i);
            availableSlotCount++;
        }
    }

    if (availableSlotCount == 0) {
        printf("\nNo available slots for consultation.\n");
        return -1; // Return -1 to indicate no available slots
    }

    int slot;
    printf("\nEnter your preferred time slot (time in hours): ");
    scanf("%d", &slot);

    if (slot < doctor->info.stime || slot >=doctor->info.etime || doctor->info.occupied_slots[slot - doctor->info.stime] != 0) {
        printf("Invalid time slot.\n");
        return -1; // Return -1 to indicate failure
    }

    doctor->info.occupied_slots[slot - doctor->info.stime] = 1;

    // Assuming that "query" is a property of the doctor_record structure
    printf("Enter your query: ");
    scanf("%s", doctor->info.occupied_events[slot - doctor->info.stime]);

    printf("Appointment scheduled successfully at slot %d\n", slot);

    return slot;
}


void scheduleappointment(appointmentHashTableNode* hashTable[], doctor_record* head_doc, patient_record* head_pat, InventoryItem* head_inventory) {
    // Allocate memory for the new appointment
    appointment* newnode = (appointment*)malloc(sizeof(appointment));
    if (newnode == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }

    // Initialize new appointment
    newnode->type = 0;
    newnode->doctor = NULL;
    newnode->patient = NULL;
    newnode->slotnumber = -1;

    // Get patient ID and assign patient information
    int pat_id;
    printf("Enter patient ID: ");
    scanf("%d", &pat_id);
    newnode->patient =searchPatientRecord(head_pat, pat_id);

    if (newnode->patient == NULL) {
        printf("Patient not found with ID %d.\n", pat_id);
        free(newnode);
        return;
    }

    // Get doctor department and assign doctor information
    char dep[20];
    printf("Enter doctor department: ");
    scanf("%s", dep);
    newnode->doctor = assignDoc(dep, head_doc);

    if (newnode->doctor == NULL) {
        printf("No available doctor in department %s.\n", dep);
        free(newnode);
        return;
    }

    // Get appointment type
    printf("Enter type of appointment (1: consultation, 2: operation): ");
    scanf("%d", &newnode->type);

    // Logic for different appointment types
    switch (newnode->type) {
        case 1: // Consultation
            newnode->slotnumber = availableslots(newnode->doctor, head_inventory);
            break;
        case 2: // Operation
            if (isStockAvailableDep(head_inventory, dep)==1) {
                // Assign available time slot using FCFS
                newnode->slotnumber = availableslots(newnode->doctor, head_inventory);
            }else{
                printf("Inventory Stock not available for the operation.\n");
                free(newnode);
                return;
            }
            break;
        default:
            printf("Invalid appointment type.\n");
            free(newnode);
            return;
        }// Add the new appointment to the hash table
        int key = newnode->patient->pat_id;
        insertAppointment(hashTable, key, newnode);

}

void cleanup(appointmentHashTableNode* hashTable[]) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        appointmentHashTableNode* current = hashTable[i];
        while (current != NULL) {
            appointmentHashTableNode* next = current->next;
            free(current->info);
            free(current);
            current = next;
        }
    }
}
