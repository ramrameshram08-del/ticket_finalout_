#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "smart_shuttle.h"

int isValidRoll(const char *roll) {
    if(strlen(roll) != 9) return 0;
    
    // First 2 digits
    if(!isdigit(roll[0]) || !isdigit(roll[1])) return 0;
    // Next 3 characters
    if(!isalpha(roll[2]) || !isalpha(roll[3]) || !isalpha(roll[4])) return 0;
    // Last 4 digits
    if(!isdigit(roll[5]) || !isdigit(roll[6]) || !isdigit(roll[7]) || !isdigit(roll[8])) return 0;
    
    return 1;
}

void studentModule() {
    int choice;
    while(1) {
        printf("\n--- STUDENT PANEL ---\n");
        printf("1. View Bus Routes & Availability\n");
        printf("2. Book Shuttle Seat\n");
        printf("3. Cancel Ticket\n");
        printf("4. Go Back\n");
        printf("Select option: ");
        scanf("%d", &choice);

        if(choice == 1) {
            viewRoutes();
        } else if(choice == 2) {
            bookSeat();
        } else if(choice == 3) {
            cancelTicket();
        } else if(choice == 4) {
            break;
        } else {
            printf("Invalid selection!\n");
        }
    }
}

void viewRoutes() {
    printf("\n--- CURRENT ROUTES ---\n");
    for(int i=0; i<MAX_BUS; i++) {
        printf("Bus ID: %d | Route: %s | Available Seats: %d\n", 
            busList[i].busId, busList[i].route, busList[i].availableSeats);
    }
}

void bookSeat() {
    Ticket t;
    int bId, selectedSeat = -1;
    
    viewRoutes();
    printf("\nEnter Bus ID to book: ");
    scanf("%d", &bId);

    if(bId < 1 || bId > MAX_BUS) {
        printf("Invalid Bus ID!\n");
        return;
    }

    int busIndex = bId - 1;
    
    if(busList[busIndex].availableSeats == 0) {
        printf("Sorry! This bus is fully booked.\n");
        return;
    }

    printf("\nAvailable Seats (0 = Empty, 1 = Booked):\n");
    for(int i=0; i<SEATS_PER_BUS; i++) {
        printf("[%02d: %d] ", i+1, busList[busIndex].seats[i]);
        if((i+1) % 5 == 0) printf("\n");
    }

    printf("Choose a seat number (1-%d): ", SEATS_PER_BUS);
    scanf("%d", &selectedSeat);

    if(selectedSeat < 1 || selectedSeat > SEATS_PER_BUS) {
        printf("Invalid seat number!\n");
        return;
    }

    if(busList[busIndex].seats[selectedSeat-1] == 1) {
        printf("Seat already booked! Please choose another.\n");
        return;
    }

    while(1) {
        printf("Enter Student Roll No (e.g. 24MID2002): ");
        scanf("%19s", t.rollNo);
        if(isValidRoll(t.rollNo)) {
            break;
        }
        printf("Invalid Format! Must be 2 numbers, 3 letters, 4 numbers.\n");
    }
    printf("Enter Student Name: ");
    scanf(" %49[^\n]", t.studentName);

    // Generate random ticket ID
    srand(time(NULL));
    t.ticketId = rand() % 90000 + 10000;
    t.busId = bId;
    t.seatNo = selectedSeat;
    t.amountPaid = SHUTTLE_FARE;

    // Update memory
    busList[busIndex].seats[selectedSeat-1] = 1;
    busList[busIndex].availableSeats--;

    // File handling
    FILE *fp = fopen("bookings.txt", "a");
    if(fp != NULL) {
        fprintf(fp, "%d,%s,%s,%d,%d,%.2f\n", t.ticketId, t.rollNo, t.studentName, t.busId, t.seatNo, t.amountPaid);
        fclose(fp);
    } else {
        printf("File Error: Cannot save booking.\n");
    }

    printf("\nBooking Successful! Your Ticket ID is: %d\n", t.ticketId);
    generateTicketFile(t);
    saveData();
}

void cancelTicket() {
    int tid, found = 0;
    printf("\nEnter Ticket ID to cancel: ");
    scanf("%d", &tid);

    FILE *fp = fopen("bookings.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    Ticket t;

    if(fp == NULL) {
        if(temp) fclose(temp);
        printf("No bookings found!\n");
        return;
    }

    char line[200];
    while(fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d,%[^,],%[^,],%d,%d,%f", &t.ticketId, t.rollNo, t.studentName, &t.busId, &t.seatNo, &t.amountPaid);
        if(t.ticketId == tid) {
            found = 1;
            int bIndex = t.busId - 1;
            busList[bIndex].seats[t.seatNo - 1] = 0;
            busList[bIndex].availableSeats++;
            printf("Ticket %d cancelled successfully. Refund initiated.\n", tid);
        } else {
            fprintf(temp, "%d,%s,%s,%d,%d,%.2f\n", t.ticketId, t.rollNo, t.studentName, t.busId, t.seatNo, t.amountPaid);
        }
    }
    
    fclose(fp);
    fclose(temp);
    
    remove("bookings.txt");
    rename("temp.txt", "bookings.txt");
    
    if(!found) {
        printf("Ticket ID not found!\n");
    } else {
        saveData();
        char filename[50];
        sprintf(filename, "Ticket_%d.txt", tid);
        remove(filename); // Delete the text file if present
    }
}

void generateTicketFile(Ticket t) {
    char filename[50];
    sprintf(filename, "Ticket_%d.txt", t.ticketId);
    FILE *fp = fopen(filename, "w");
    if(fp) {
        fprintf(fp, "=================================\n");
        fprintf(fp, "      SMART SHUTTLE TICKET       \n");
        fprintf(fp, "=================================\n");
        fprintf(fp, "Ticket ID : %d\n", t.ticketId);
        fprintf(fp, "Roll No   : %s\n", t.rollNo);
        fprintf(fp, "Name      : %s\n", t.studentName);
        fprintf(fp, "Bus ID    : %d\n", t.busId);
        fprintf(fp, "Seat No   : %d\n", t.seatNo);
        fprintf(fp, "Fare Paid : Rs. %.2f\n", t.amountPaid);
        fprintf(fp, "=================================\n");
        fprintf(fp, "Please show this while boarding.\n");
        fclose(fp);
        printf("=> A printable ticket '%s' has been generated.\n", filename);
    }
}

// ==========================
// API FUNCTIONS (JSON OUTPUT)
// ==========================

void apiGetRoutes() {
    printf("[\n");
    for(int i=0; i<MAX_BUS; i++) {
        printf("  {\"busId\": %d, \"route\": \"%s\", \"availableSeats\": %d}", 
            busList[i].busId, busList[i].route, busList[i].availableSeats);
        if(i < MAX_BUS - 1) printf(",\n");
    }
    printf("\n]\n");
}

void apiGetSeats(int busId) {
    if(busId < 1 || busId > MAX_BUS) {
        printf("[]\n");
        return;
    }
    int busIndex = busId - 1;
    printf("[\n");
    for(int i=0; i<SEATS_PER_BUS; i++) {
        printf("  %d", busList[busIndex].seats[i]);
        if(i < SEATS_PER_BUS - 1) printf(",\n");
    }
    printf("\n]\n");
}

void apiBookSeat(int busId, int seatNo, char* rollNo, char* name) {
    if(busId < 1 || busId > MAX_BUS || seatNo < 1 || seatNo > SEATS_PER_BUS) {
        printf("{\"status\": \"error\", \"message\": \"Invalid parameters\"}\n");
        return;
    }
    
    // Check Roll Number Format
    if(!isValidRoll(rollNo)) {
        printf("{\"status\": \"error\", \"message\": \"Invalid Roll Number Format! Use format like: 24MID2002 (2 nums, 3 letters, 4 nums).\"}\n");
        return;
    }
    
    int busIndex = busId - 1;
    if(busList[busIndex].seats[seatNo-1] == 1) {
        printf("{\"status\": \"error\", \"message\": \"Seat already booked\"}\n");
        return;
    }

    Ticket t;
    strcpy(t.rollNo, rollNo);
    strcpy(t.studentName, name);
    srand(time(NULL));
    t.ticketId = rand() % 90000 + 10000;
    t.busId = busId;
    t.seatNo = seatNo;
    t.amountPaid = SHUTTLE_FARE;

    busList[busIndex].seats[seatNo-1] = 1;
    busList[busIndex].availableSeats--;

    FILE *fp = fopen("bookings.txt", "a");
    if(fp != NULL) {
        fprintf(fp, "%d,%s,%s,%d,%d,%.2f\n", t.ticketId, t.rollNo, t.studentName, t.busId, t.seatNo, t.amountPaid);
        fclose(fp);
    }

    saveData();
    printf("{\"status\": \"success\", \"ticketId\": %d}\n", t.ticketId);
}

void apiGetTickets(char* rollNo) {
    FILE *fp = fopen("bookings.txt", "r");
    Ticket t;
    int first = 1;
    printf("[\n");
    if(fp != NULL) {
        char line[200];
        while(fgets(line, sizeof(line), fp)) {
            if(sscanf(line, "%d,%[^,],%[^,],%d,%d,%f", &t.ticketId, t.rollNo, t.studentName, &t.busId, &t.seatNo, &t.amountPaid) == 6) {
                if(strcmp(t.rollNo, rollNo) == 0) {
                    if(!first) printf(",\n");
                    printf("  {\"ticketId\": %d, \"busId\": %d, \"seatNo\": %d, \"amountPaid\": %.2f}",
                        t.ticketId, t.busId, t.seatNo, t.amountPaid);
                    first = 0;
                }
            }
        }
        fclose(fp);
    }
    printf("\n]\n");
}

void apiCancelTicket(int ticketId) {
    int found = 0;
    FILE *fp = fopen("bookings.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    Ticket t;

    if(fp == NULL) {
        if(temp) fclose(temp);
        printf("{\"status\": \"error\", \"message\": \"No bookings found\"}\n");
        return;
    }

    char line[200];
    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "%d,%[^,],%[^,],%d,%d,%f", &t.ticketId, t.rollNo, t.studentName, &t.busId, &t.seatNo, &t.amountPaid) == 6) {
            if(t.ticketId == ticketId) {
                found = 1;
                int bIndex = t.busId - 1;
                busList[bIndex].seats[t.seatNo - 1] = 0;
                busList[bIndex].availableSeats++;
            } else {
                fprintf(temp, "%d,%s,%s,%d,%d,%.2f\n", t.ticketId, t.rollNo, t.studentName, t.busId, t.seatNo, t.amountPaid);
            }
        }
    }
    fclose(fp);
    fclose(temp);
    
    remove("bookings.txt");
    rename("temp.txt", "bookings.txt");
    
    if(found) {
        saveData();
        printf("{\"status\": \"success\"}\n");
    } else {
        printf("{\"status\": \"error\", \"message\": \"Ticket not found\"}\n");
    }
}

void apiRegisterStudent(char* rollNo, char* name) {
    if(!isValidRoll(rollNo)) {
        printf("{\"status\": \"error\", \"message\": \"Invalid Roll Number format.\"}");
        return;
    }
    
    FILE *fp = fopen("students.txt", "a+");
    if(!fp) {
        printf("{\"status\": \"error\", \"message\": \"Database error.\"}");
        return;
    }
    
    Student s;
    rewind(fp);
    char line[100];
    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "%[^,],%[^,],%d", s.rollNo, s.name, &s.status) == 3) {
            if(strcmp(s.rollNo, rollNo) == 0) {
                printf("{\"status\": \"error\", \"message\": \"Roll number already registered.\"}");
                fclose(fp);
                return;
            }
        }
    }
    
    fprintf(fp, "%s,%s,%d\n", rollNo, name, 0); // 0: pending
    fclose(fp);
    
    printf("{\"status\": \"success\", \"message\": \"Registration submitted! Please wait for admin approval.\"}");
}

void apiVerifyLogin(char* rollNo, char* name) {
    FILE *fp = fopen("students.txt", "r");
    if(!fp) {
        printf("{\"status\": \"error\", \"message\": \"No students registered yet. Please register first.\"}");
        return;
    }
    
    Student s;
    int found = 0;
    char line[100];
    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "%[^,],%[^,],%d", s.rollNo, s.name, &s.status) == 3) {
            if(strcmp(s.rollNo, rollNo) == 0) {
                found = 1;
                if(s.status == 1) {
                    printf("{\"status\": \"success\", \"name\": \"%s\"}", s.name);
                } else if(s.status == 2) {
                    printf("{\"status\": \"error\", \"message\": \"Your account has been blacklisted by the administrator.\"}");
                } else {
                    printf("{\"status\": \"error\", \"message\": \"Your account is still pending admin approval.\"}");
                }
                break;
            }
        }
    }
    fclose(fp);
    
    if(!found) {
        printf("{\"status\": \"error\", \"message\": \"Roll number not found. Please register as a new student.\"}");
    }
}
