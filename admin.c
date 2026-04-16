#include <stdio.h>
#include <string.h>
#include "smart_shuttle.h"

int verifyAdmin() {
    char password[20];
    printf("Enter Admin Password: ");
    scanf("%19s", password);
    // Hardcoded simple password for college project realism
    if(strcmp(password, "admin123") == 0) {
        return 1;
    }
    printf("Access Denied! Incorrect Password.\n");
    return 0;
}

void adminModule() {
    if(!verifyAdmin()) return;

    int choice;
    while(1) {
        printf("\n--- ADMIN PANEL ---\n");
        printf("1. View All Bookings\n");
        printf("2. Check Total Revenue\n");
        printf("3. Reset/Initialize Buses (WARNING: Wipes data)\n");
        printf("4. Logout\n");
        printf("Select option: ");
        scanf("%d", &choice);

        if(choice == 1) {
            viewAllBookings();
        } else if(choice == 2) {
            checkRevenue();
        } else if(choice == 3) {
            initializeBuses();
        } else if(choice == 4) {
            break;
        } else {
            printf("Invalid selection!\n");
        }
    }
}

void viewAllBookings() {
    FILE *fp = fopen("bookings.txt", "r");
    Ticket t;
    if(fp == NULL) {
        printf("No bookings made yet.\n");
        return;
    }

    printf("\n--- ALL BOOKINGS RECORD ---\n");
    printf("%-10s | %-15s | %-20s | %-6s | %-6s\n", "Ticket ID", "Roll No", "Name", "Bus ID", "Seat");
    printf("-----------------------------------------------------------------------\n");
    char line[200];
    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "%d,%[^,],%[^,],%d,%d,%f", &t.ticketId, t.rollNo, t.studentName, &t.busId, &t.seatNo, &t.amountPaid) == 6) {
            printf("%-10d | %-15s | %-20s | %-6d | %-6d\n", t.ticketId, t.rollNo, t.studentName, t.busId, t.seatNo);
        }
    }
    fclose(fp);
}

void checkRevenue() {
    FILE *fp = fopen("bookings.txt", "r");
    Ticket t;
    float total = 0.0;
    if(fp != NULL) {
        char line[200];
        while(fgets(line, sizeof(line), fp)) {
            if(sscanf(line, "%d,%[^,],%[^,],%d,%d,%f", &t.ticketId, t.rollNo, t.studentName, &t.busId, &t.seatNo, &t.amountPaid) == 6) {
                total += t.amountPaid;
            }
        }
        fclose(fp);
    }
    printf("\n-----------------------------------\n");
    printf("Total Revenue Collected: Rs. %.2f\n", total);
    printf("-----------------------------------\n");
}

void initializeBuses() {
    char confirm;
    printf("\nAre you sure you want to reset all buses? This erases all data (y/n): ");
    scanf(" %c", &confirm);
    
    if(confirm == 'y' || confirm == 'Y') {
        busList[0].busId = 1;
        strcpy(busList[0].route, "Main Gate -> Hostels");
        
        busList[1].busId = 2;
        strcpy(busList[1].route, "Hostels -> Academic Block");
        
        busList[2].busId = 3;
        strcpy(busList[2].route, "Library -> IT Park");

        for(int i=0; i<MAX_BUS; i++) {
            busList[i].availableSeats = SEATS_PER_BUS;
            for(int j=0; j<SEATS_PER_BUS; j++) {
                busList[i].seats[j] = 0; // mark all empty
            }
        }
        saveData();
        remove("bookings.txt"); // delete old bookings file safely
        printf("All buses have been reset to empty!\n");
    }
}

// ==========================
// ADMIN API FUNCTIONS
// ==========================

void apiVerifyAdmin(char* password) {
    if(strcmp(password, "admin123") == 0) {
        printf("{\"status\": \"success\"}\n");
    } else {
        printf("{\"status\": \"error\", \"message\": \"Incorrect password\"}\n");
    }
}

void apiGetAllBookings() {
    FILE *fp = fopen("bookings.txt", "r");
    Ticket t;
    int first = 1;
    printf("[\n");
    if(fp != NULL) {
        char line[200];
        while(fgets(line, sizeof(line), fp)) {
            if(sscanf(line, "%d,%[^,],%[^,],%d,%d,%f", &t.ticketId, t.rollNo, t.studentName, &t.busId, &t.seatNo, &t.amountPaid) == 6) {
                if(!first) printf(",\n");
                printf("  {\"ticketId\": %d, \"rollNo\": \"%s\", \"studentName\": \"%s\", \"busId\": %d, \"seatNo\": %d, \"amountPaid\": %.2f}",
                    t.ticketId, t.rollNo, t.studentName, t.busId, t.seatNo, t.amountPaid);
                first = 0;
            }
        }
        fclose(fp);
    }
    printf("\n]\n");
}

void apiGetRevenue() {
    FILE *fp = fopen("bookings.txt", "r");
    Ticket t;
    float total = 0.0;
    if(fp != NULL) {
        char line[200];
        while(fgets(line, sizeof(line), fp)) {
            if(sscanf(line, "%d,%[^,],%[^,],%d,%d,%f", &t.ticketId, t.rollNo, t.studentName, &t.busId, &t.seatNo, &t.amountPaid) == 6) {
                total += t.amountPaid;
            }
        }
        fclose(fp);
    }
    printf("{\"revenue\": %.2f}\n", total);
}

void apiResetBuses() {
    busList[0].busId = 1;
    strcpy(busList[0].route, "Main Gate -> Hostels");
    
    busList[1].busId = 2;
    strcpy(busList[1].route, "Hostels -> Academic Block");
    
    busList[2].busId = 3;
    strcpy(busList[2].route, "Library -> IT Park");

    for(int i=0; i<MAX_BUS; i++) {
        busList[i].availableSeats = SEATS_PER_BUS;
        for(int j=0; j<SEATS_PER_BUS; j++) {
            busList[i].seats[j] = 0; // mark all empty
        }
    }
    saveData();
    remove("bookings.txt");
    printf("{\"status\": \"success\"}\n");
}

void apiGetPendingStudents() {
    FILE *fp = fopen("students.txt", "r");
    Student s;
    int first = 1;
    printf("[\n");
    if(fp) {
        char line[100];
        while(fgets(line, sizeof(line), fp)) {
            if(sscanf(line, "%[^,],%[^,],%d", s.rollNo, s.name, &s.status) == 3) {
                if(s.status == 0) {
                    if(!first) printf(",\n");
                    printf("  {\"rollNo\": \"%s\", \"name\": \"%s\"}", s.rollNo, s.name);
                    first = 0;
                }
            }
        }
        fclose(fp);
    }
    printf("\n]\n");
}

void apiApproveStudent(char* rollNo) {
    FILE *fp = fopen("students.txt", "r");
    FILE *temp = fopen("temp_students.txt", "w");
    if(!fp) {
        if(temp) fclose(temp);
        printf("{\"status\": \"error\", \"message\": \"Database error.\"}");
        return;
    }
    
    Student s;
    int found = 0;
    char line[100];
    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "%[^,],%[^,],%d", s.rollNo, s.name, &s.status) == 3) {
            if(strcmp(s.rollNo, rollNo) == 0) {
                s.status = 1; // Approved
                found = 1;
            }
            fprintf(temp, "%s,%s,%d\n", s.rollNo, s.name, s.status);
        }
    }
    fclose(fp);
    fclose(temp);
    remove("students.txt");
    rename("temp_students.txt", "students.txt");
    if(found) printf("{\"status\": \"success\"}");
    else printf("{\"status\": \"error\", \"message\": \"Student not found.\"}");
}

void apiRejectStudent(char* rollNo) {
    FILE *fp = fopen("students.txt", "r");
    FILE *temp = fopen("temp_students.txt", "w");
    if(!fp || !temp) {
        if(fp) fclose(fp);
        if(temp) fclose(temp);
        printf("{\"status\": \"error\", \"message\": \"Database error.\"}");
        return;
    }
    Student s;
    int found = 0;
    char line[100];
    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "%[^,],%[^,],%d", s.rollNo, s.name, &s.status) == 3) {
            if(strcmp(s.rollNo, rollNo) == 0) {
                found = 1;
            } else {
                fprintf(temp, "%s,%s,%d\n", s.rollNo, s.name, s.status);
            }
        }
    }
    fclose(fp);
    fclose(temp);
    remove("students.txt");
    rename("temp_students.txt", "students.txt");
    if(found) printf("{\"status\": \"success\"}");
    else printf("{\"status\": \"error\", \"message\": \"Student not found.\"}");
}

void apiBlacklistStudent(char* rollNo) {
    FILE *fp = fopen("students.txt", "r");
    FILE *temp = fopen("temp_students.txt", "w");
    if(!fp) {
        if(temp) fclose(temp);
        printf("{\"status\": \"error\", \"message\": \"Database error.\"}");
        return;
    }
    Student s;
    int found = 0;
    char line[100];
    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "%[^,],%[^,],%d", s.rollNo, s.name, &s.status) == 3) {
            if(strcmp(s.rollNo, rollNo) == 0) {
                s.status = 2; // Blacklisted
                found = 1;
            }
            fprintf(temp, "%s,%s,%d\n", s.rollNo, s.name, s.status);
        }
    }
    fclose(fp);
    fclose(temp);
    remove("students.txt");
    rename("temp_students.txt", "students.txt");
    if(found) printf("{\"status\": \"success\"}");
    else printf("{\"status\": \"error\", \"message\": \"Student not found.\"}");
}

void apiUnblacklistStudent(char* rollNo) {
    FILE *fp = fopen("students.txt", "r");
    FILE *temp = fopen("temp_students.txt", "w");
    if(!fp) {
        if(temp) fclose(temp);
        printf("{\"status\": \"error\", \"message\": \"Database error.\"}");
        return;
    }
    Student s;
    int found = 0;
    char line[100];
    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "%[^,],%[^,],%d", s.rollNo, s.name, &s.status) == 3) {
            if(strcmp(s.rollNo, rollNo) == 0) {
                s.status = 1; // Back to Approved
                found = 1;
            }
            fprintf(temp, "%s,%s,%d\n", s.rollNo, s.name, s.status);
        }
    }
    fclose(fp);
    fclose(temp);
    remove("students.txt");
    rename("temp_students.txt", "students.txt");
    if(found) printf("{\"status\": \"success\"}");
    else printf("{\"status\": \"error\", \"message\": \"Student not found.\"}");
}

void apiGetAllStudents() {
    FILE *fp = fopen("students.txt", "r");
    Student s;
    int first = 1;
    printf("[\n");
    if(fp) {
        char line[100];
        while(fgets(line, sizeof(line), fp)) {
            if(sscanf(line, "%[^,],%[^,],%d", s.rollNo, s.name, &s.status) == 3) {
                if(!first) printf(",\n");
                printf("  {\"rollNo\": \"%s\", \"name\": \"%s\", \"status\": %d}", s.rollNo, s.name, s.status);
                first = 0;
            }
        }
        fclose(fp);
    }
    printf("\n]\n");
}
