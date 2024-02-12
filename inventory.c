#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <limits.h>
#include "sqlite3.h"
#define MIN_STOCK 100

sqlite3 *dbI;

// Structure to represent an inventory item
typedef struct InventoryItem {
    char itemName[100];
    int quantity;
    char dep[20];
    struct InventoryItem* next;
} InventoryItem;

InventoryItem* insertInventoryItem(InventoryItem* head, const char* itemName, int quantity, const char* dep);
void loadInventoryData(InventoryItem** head);
void printInventory(InventoryItem* head);
int isStockAvailable(InventoryItem* head, const char* itemname) ;
void updateStock(InventoryItem* head, const char* itemName);

int createInventoryTable() {
    char *sql = "CREATE TABLE IF NOT EXISTS inventory ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "item_name TEXT NOT NULL,"
                "quantity INTEGER NOT NULL,"
                "department TEXT NOT NULL"
                ");";

    int rc = sqlite3_exec(dbI, sql, 0, 0, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(dbI));
        return rc;
    }
    return SQLITE_OK;
}

// Function to create a new inventory item
InventoryItem* createInventoryItem(const char* itemName, int quantity, const char* dep) {
    InventoryItem* newItem = (InventoryItem*)malloc(sizeof(InventoryItem));
    if (newItem == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    strcpy(newItem->itemName, itemName);
    newItem->quantity = quantity;
    strcpy(newItem->dep, dep);
    newItem->next = NULL;
    return newItem;
}

void loadInventoryData(InventoryItem** head) {
    // Open the database connection
    sqlite3* dbI;
    if (sqlite3_open("inventory.db", &dbI) != SQLITE_OK) {
        fprintf(stderr, "Cannot open inventory database: %s\n", sqlite3_errmsg(dbI));
        sqlite3_close(dbI);
        return;
    }

    // Create the inventory table if it doesn't exist
    createInventoryTable();

    // SQL query to retrieve all records from the inventory table
    const char* sql = "SELECT * FROM inventory;";

    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(dbI, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(dbI));
        sqlite3_close(dbI);
        return;
    }

    // Fetch records and add them to the linked list
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* itemName = (const char*)sqlite3_column_text(stmt, 1);
        int quantity = sqlite3_column_int(stmt, 2);
        const char* dep = (const char*)sqlite3_column_text(stmt, 3);

        // Create a new inventory item and add it to the linked list
        *head = insertInventoryItem(*head, itemName, quantity, dep);
    }

    // Finalize the statement and close the database connection
    sqlite3_finalize(stmt);
    sqlite3_close(dbI);
}

void insertInventoryItemIntoDatabase(InventoryItem* newItem) {
    char sql[500];
    sprintf(sql, "INSERT INTO inventory (item_name, quantity, department) VALUES ('%s', %d, '%s')",
            newItem->itemName, newItem->quantity, newItem->dep);

    if (sqlite3_exec(dbI, sql, 0, 0, 0) != SQLITE_OK) {
        
    } else {
        printf("Inventory item inserted successfully!\n");
    }
}


InventoryItem* insertInventoryItem(InventoryItem* head, const char* itemName, int quantity, const char* dep) {
    // Check if the item already exists in the inventory
    InventoryItem* current = head;
    while (current != NULL) {
        if (strcmp(current->itemName, itemName) == 0 &&strcmp(current->dep, dep) == 0) {
            // Item already exists, update quantity and return
            current->quantity += quantity;
            return head;
        }
        current = current->next;
    }

    // If the item doesn't exist, create a new node and insert into the database
    InventoryItem* newItem = createInventoryItem(itemName, quantity, dep);
    newItem->next = head;
    insertInventoryItemIntoDatabase(newItem);
    return newItem;
}

// Function to print all inventory items
void printInventory(InventoryItem* head) {
    printf("Inventory Items:\n");
    InventoryItem* current = head;
    while (current->next != NULL) {
        printf("Item Name: %s, Quantity: %d, Department: %s\n", current->itemName, current->quantity, current->dep);
        current = current->next;
    }
}

int isStockAvailable(InventoryItem* head, const char* itemname) {
    InventoryItem* current = head;
    while (current != NULL) {
        if (strcmp(current->itemName, itemname) == 0 && current->quantity >= (MIN_STOCK-1)) {
            return 1; // At least one item in the specified department has a quantity less than or equal to 1
        }
        current = current->next;
    }
    return 0; // All items in the specified department have a quantity greater than 1
}
int isStockAvailableDep(InventoryItem* head, const char* dep) {
    InventoryItem* current = head;
    while (current != NULL) {
        if (strcmp(current->dep, dep) == 0 && current->quantity < (MIN_STOCK)) {
            return 0; // At least one item in the specified department has a quantity less than or equal to MIN_STOCK
        }
        current = current->next;
    }
    return 1; // All items in the specified department have a quantity greater than MIN_STOCK
}

void updateStock(InventoryItem* head, const char* itemName) {
    InventoryItem* current = head;

    // Find the item with the specified itemName in the inventory
    while (current != NULL) {
        if (strcmp(current->itemName, itemName) == 0) {
            break;
        }
        current = current->next;
    }

    if (current == NULL) {
        // Item with the specified itemName not found
        printf("Item with name %s not found in the inventory.\n", itemName);
        return;
    }

    if (current->quantity < MIN_STOCK) {
        // Calculate the quantity needed to reach MIN_STOCK
        int quantityToBuy = MIN_STOCK - current->quantity;

        // Simulate buying more stock (you can replace this with your actual buying process)
        printf("Buying %d units of %s to replenish stock.\n", quantityToBuy, current->itemName);

        // Update the stock quantity in memory
        current->quantity += quantityToBuy;

        // Update the stock quantity in the database
        char sql[500];
        sprintf(sql, "UPDATE inventory SET quantity = %d WHERE item_name = '%s'", current->quantity, current->itemName);

        if (sqlite3_exec(dbI, sql, 0, 0, 0) != SQLITE_OK) {
            fprintf(stderr, "Error updating stock in the database: %s\n", sqlite3_errmsg(dbI));
        } else {
            printf("Stock updated in the database: %s - Quantity: %d\n", current->itemName, current->quantity);
        }
    } else {
        printf("Stock is sufficient for %s. No action needed.\n", current->itemName);
    }
}
void addQuantityToItem(InventoryItem* head, const char* itemName, int additionalQuantity) {
    InventoryItem* current = head;

    // Find the item with the specified itemName in the inventory
    while (current != NULL) {
        if (strcmp(current->itemName, itemName) == 0) {
            break;
        }
        current = current->next;
    }

    if (current == NULL) {
        // Item with the specified itemName not found
        printf("Item with name %s not found in the inventory.\n", itemName);
        return;
    }

    // Add the additional quantity to the item in memory
    current->quantity += additionalQuantity;

    // Update the stock quantity in the database
    char sql[500];
    sprintf(sql, "UPDATE inventory SET quantity = %d WHERE item_name = '%s'", current->quantity, current->itemName);

    if (sqlite3_exec(dbI, sql, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Error updating stock in the database: %s\n", sqlite3_errmsg(dbI));
    } else {
        printf("Stock updated in the database: %s - Quantity: %d\n", current->itemName, current->quantity);
    }
}
