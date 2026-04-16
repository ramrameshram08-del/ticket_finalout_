#include <stdio.h>
#include <string.h>
#include "smart_shuttle.h"

void loadData() {
    FILE *fp = fopen("buses.txt", "r");
    if(fp != NULL) {
        for(int i = 0; i < MAX_BUS; i++) {
            if (fscanf(fp, "%d %[^\n]", &busList[i].busId, busList[i].route) != 2) break;
            if (fscanf(fp, "%d", &busList[i].availableSeats) != 1) break;
            for(int j = 0; j < SEATS_PER_BUS; j++) {
                fscanf(fp, "%d", &busList[i].seats[j]);
            }
            fgetc(fp); // consume newline
        }
        fclose(fp);
    } else {
        printf("\n[System] No existing bus data found. Applying default routes...\n");
        // Fresh start default setup
        busList[0].busId = 1; 
        strcpy(busList[0].route, "Main Gate -> Hostels"); 
        
        busList[1].busId = 2; 
        strcpy(busList[1].route, "Hostels -> Academic Block"); 
        
        busList[2].busId = 3; 
        strcpy(busList[2].route, "Library -> IT Park"); 

        for(int i=0; i<MAX_BUS; i++) {
            busList[i].availableSeats = SEATS_PER_BUS;
            for(int j=0; j<SEATS_PER_BUS; j++) {
                busList[i].seats[j] = 0; // 0 means empty
            }
        }
        // Save these defaults initially
        saveData();
    }
}

void saveData() {
    FILE *fp = fopen("buses.txt", "w");
    if(fp != NULL) {
        for(int i = 0; i < MAX_BUS; i++) {
            fprintf(fp, "%d\n%s\n%d\n", busList[i].busId, busList[i].route, busList[i].availableSeats);
            for(int j = 0; j < SEATS_PER_BUS; j++) {
                fprintf(fp, "%d ", busList[i].seats[j]);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    } else {
        printf("Error: Could not save bus state to disk.\n");
    }
}
