#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <limits.h>
#include "sqlite3.c"
#include "scheduling.c"



int main() {
    printf("Welcome !\n");
    patient_record *head_pat = malloc(sizeof(patient_record));
    int pdatabase =openDataBase(); // Call your openDatabase function or any other functions here
    int pat_table= createPatientTable();

    doctor_record *head_doc = malloc(sizeof(doctor_record));
    int ddatabase =openDatabaseD();
    int doc_table=createDoctorTable();

    InventoryItem *head_inventory =malloc(sizeof(head_inventory));
    if (sqlite3_open("inventory.db", &dbI) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(dbI));
        return EXIT_FAILURE;
    }
    // Create the inventory table if it doesn't exist
    if (createInventoryTable() != SQLITE_OK) {
        fprintf(stderr, "Failed to create inventory table.\n");
        return EXIT_FAILURE;
    }
    appointmentHashTableNode* appointmentHashTable[HASH_TABLE_SIZE];
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        appointmentHashTable[i] = NULL;
    }


    loadDoctorData(&head_doc);
    loadPatientData(&head_pat);
    loadInventoryData(&head_inventory);
    int ch1,ch2;
    do{
    printf("\nManage:\n1.Patients\n2.Doctors\n3.Schedules\n4.Inventory\n0.Exit\n\n");
    scanf("%d",&ch1);
    switch(ch1)
    {
        case 1:
        printf("1.Add New Patient\n2.Update Patient Information\n3.Display Patient Information\n4.Delete Patient Information\n");
        scanf("%d",&ch2);
        switch(ch2){
            case 1:
            addPatientRecord(head_pat,1,db_pat);
            break;
            case 2:
            updatePatientRecord(head_pat,db_pat);
            break;
            case 3:
            displayPatientRecord(db_pat);
            break;
            case 4:
            deletePatientRecord(head_pat, db_pat);
            break;
        }
        break;


        case 2:
        printf("1.Add New Doctor\n2.Delete Doctor Record\n3.Display Doctor Information\nEnter Your Choice: ");
        scanf("%d",&ch2);
        switch(ch2){
            case 1:
             addDoctorRecord( head_doc, 1,db_doc);
            break;
            case 2:
              deleteDoctorRecord(&head_doc);
            break;
            case 3:
           displayDoctorRecordByID(head_doc,db_doc);
            break;
        }

        break;

        case 3:
        printf("1. Book an Appointment\n");
        printf("2. Cancel an Appointment\n");
        printf("3. View Appointments\n");
        printf("Enter your choice: ");
        scanf("%d", &ch2);

        switch (ch2) {
            case 1:
                scheduleappointment(appointmentHashTable, head_doc, head_pat, head_inventory);
                break;

            case 2:

                    int pat_id;
                    printf("Enter patient ID to cancel appointment: ");
                    scanf("%d", &pat_id);
                    cancelAppointment(appointmentHashTable, pat_id);
                break;

            case 3:
                // View Appointments
                {
                    int pat_id;
                    printf("Enter patient ID to view appointments: ");
                    scanf("%d", &pat_id);
                    viewAppointments(appointmentHashTable, pat_id);
                }
                break;
        }

        break;
        case 4:
        printf("1.Add New Inventory Item\n2.Print All Inventory Items\n3.Check Stock Availability\n4.Update Stock\n5.Update Stock- Custom\nEnter Your Choice: ");
        scanf("%d",&ch2);
        switch(ch2){    
        case 1:

            char itemName[100];
            int quantity;
            char dep[20];

            printf("Enter item name: ");
            scanf("%s", itemName);

            printf("Enter quantity: ");
            scanf("%d", &quantity);

            printf("Enter department: ");
            scanf("%s", dep);

            // Insert the new inventory item
            head_inventory = insertInventoryItem(head_inventory, itemName, quantity, dep);
        break;

        case 2:
            // Option to print all inventory items
            printInventory(head_inventory);
            break;

        case 3:
            // Option to check stock availability of a particular item
                char itemname[20];
                printf("Enter item name to check stock availability: ");
                scanf("%s", itemname);

                if (isStockAvailable(head_inventory, itemname))
                    printf("\nStock is available for this item.\n");
                else 
                    printf("\nStock is not available for this item.\n");
                
            break;
        case 4:
        char item[20];
        printf("Enter Inventory Item\n");
        scanf("%s",&item);
        updateStock(head_inventory,item);
        break;case 5:
    char itemn[50];
    int quan;
    printf("Enter item name : ");
    scanf("%s", itemn);
    printf("Enter quantity to be added: ");
    scanf("%d", &quan);  // Use %d for integers, and don't forget the address-of operator (&)

    addQuantityToItem(head_inventory, itemn, quan);
    break;  // Don't forget to break the case

    }}}while(ch1!=0);
    printf("\n Thank You");
cleanup(appointmentHashTable);

    sqlite3_close(dbI);
    sqlite3_close(db_doc);
    sqlite3_close(db_pat);

    return 0; // Indicates successful execution
}