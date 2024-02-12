#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "inventory.c"
#include "doctor.c"
#include "patient.c"
#include "sqlite3.c"

typedef struct appointment{
    int type;
    info* patient;
    doctor_record* doctor;
    int slotnumber;
}appointment;

typedef struct appointmentnode {
    appointment info;
    struct appointmentnode* next;
} appointmentNode;

int availableslots(doctor_record *doctor, InventoryItem* inventory) {
    for (int i = 0; i < doctor->info.num_slots; i++) {
        if (doctor->info.occupied_slots[i] == 0) {
            printf("%d ", (doctor->info.stime + i));
        }
    }

    int slot;
    printf("\nEnter your preferred time slot: ");
    scanf("%d", &slot);

        char dep[20];
        strcpy(dep, doctor->info.dep);
        if (doctor->info.occupied_slots[slot] == 0) {
            doctor->info.occupied_slots[slot] = 1;
            printf("Enter your query: ");
            scanf("%s", doctor->info.occupied_events[slot]);
            printf("Appointment scheduled successfully\n");
            return doctor->info.occupied_events[slot];
        } else {
            printf("Invalid time slot or insufficient inventory.\n");
        }
    
}

int getCurrentTime() {
    // Get the current time in seconds since the epoch
    time_t currentTime = time(NULL);

    // Convert the current time to a struct tm (local time)
    struct tm* localTime = localtime(&currentTime);

    // Extract the current hour in 24-hour format from the struct tm
    int currentHour = localTime->tm_hour;

    // Return the current hour
    return currentHour;
}

void assignEmergencySlot(appointmentNode **head, appointment* newnode, InventoryItem* inventory) {
    int highestPriority = INT_MAX; // Set initial highest priority to maximum value
    int targetSlot = -1;

    // Find the first available slot with the highest priority or the next higher priority if target slot is occupied
    for (int i = 0; i < newnode->doctor->info.num_slots; i++) {
        if (newnode->doctor->info.occupied_slots[i] == 0) {
            targetSlot = i;
            break;
        }
    }

    // Check if a suitable slot was found
    if (targetSlot == -1) {
        printf("No available slots for emergency appointment.\n");
        return;
    }

    // Shift occupied slots and events to the next available slot if the target slot is occupied
    if (newnode->doctor->info.occupied_slots[targetSlot] == 1) {
        for (int i = newnode->doctor->info.num_slots - 1; i > targetSlot; i--) {
            strcpy(newnode->doctor->info.occupied_events[i], newnode->doctor->info.occupied_events[i - 1]);
            newnode->doctor->info.occupied_slots[i] = newnode->doctor->info.occupied_slots[i - 1];
        }
    }

    // Move the emergency appointment to the target slot
    newnode->doctor->info.occupied_slots[targetSlot] = 1;

    // Allocate memory for the event at the target slot
    newnode->doctor->info.occupied_events[targetSlot] = (char*)malloc(50 * sizeof(char));
    if (newnode->doctor->info.occupied_events[targetSlot] == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }

    // Reduce inventory by 4 units after emergency
    inventory->quantity -= 4;

    printf("Enter your query: ");
    scanf("%s", newnode->doctor->info.occupied_events[targetSlot]);
    printf("Emergency appointment scheduled successfully at slot %d\n", targetSlot);

    // Create a new appointment node and add it to the linked list
    appointmentNode* temp = (appointmentNode*)malloc(sizeof(appointmentNode));
    if (temp == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }
    temp->info = *newnode;
    temp->next = *head;
    *head = temp;
}

void scheduleappointment(appointmentNode *head,patient_record *patients, doctor_record *doctors, InventoryItem *inventory) 
{
    // Allocate memory for the new appointment
    appointmentNode* temp = (appointmentNode*)malloc(sizeof(appointmentNode));
    appointment* newnode = (appointment*)malloc(sizeof(appointment));

    if (newnode == NULL) {
        // Handle memory allocation failure
        exit(EXIT_FAILURE);
    }

    // Initialize new appointment node
    newnode->type = 0;
    newnode->doctor = NULL;
    newnode->patient = NULL;
    newnode->slotnumber=-1;

    // Get patient ID and assign patient information
    int id;
    printf("Enter patient ID: ");
    scanf("%d", &id);
    newnode->patient = searchPatientRecord(patients, id);

    // Get doctor department and assign doctor information
    char dep[20];
    printf("Enter doctor department: ");
    scanf("%s", dep);
    newnode->doctor = assignDoc(dep, doctors);

    // Get appointment type
    printf("Enter type of appointment (1: consultation, 2: operation, 3: emergency): ");
    scanf("%d", &newnode->type);

    // Logic for different appointment types
    switch (newnode->type) {
        case 1: // Consultation
         newnode->slotnumber=availableslots(newnode->doctor, inventory);
                temp->info=*newnode;
    temp->next=head;
    head = temp;
    break;
        case 2: // Operation
         if(isStockAvailable(inventory, dep)){
            // Assign available time slot using FCFS
            newnode->slotnumber=availableslots(newnode->doctor, inventory);
                temp->info=*newnode;
    temp->next=head;
    head = temp;}
            break;
       case 3: // Emergency
            assignEmergencySlot(head,newnode,inventory) ;break;
        default:
            printf("Invalid appointment type.\n");
            break;
    }
    // Add the new appointment node to the appropriate data structure or list
    // For example, you can add it to a linked list of appointments.


}

void cancelAppointment(appointmentNode **head, int patientID) {
    appointmentNode *current = *head;
    appointmentNode *prev = NULL;
    while (current != NULL) {
if (current->info.patient->pat_id == patientID) {
            // Found the appointment to cancel
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            printf("Appointment for patient ID %d has been canceled.\n", patientID);
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("No appointment found for patient ID %d.\n", patientID);
}

void viewAppointments(appointmentNode *head, int patientID) {
    appointmentNode *current = head;
    int count = 0;
    while (current != NULL) {
        if (current->info.patient->pat_id == patientID) {
            // Found appointment for the patient
            printf("Appointment %d:\n", count + 1);
            printf("Doctor: %s\n", current->info.doctor->info.name);
            printf("Time Slot: %d\n", current->info.slotnumber);
            printf("Query: %s\n", current->info.doctor->info.occupied_events[current->info.slotnumber]);
            printf("-----------\n");
            count++;
        }
        current = current->next;
    }
    if (count == 0) {
        printf("No appointments found for patient ID %d.\n", patientID);
    }
}
