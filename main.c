#include <stdio.h>
#include <stdlib.h>
#include "smart_shuttle.h"

// Definition of the global array
Bus busList[MAX_BUS];

int main(int argc, char *argv[]) {
    int userChoice;
    
    // Load existing data at startup
    loadData();

    // If running in Render environment or Docker, start Web Server automatically
    if(getenv("RENDER") || getenv("PORT") || (argc > 1 && strcmp(argv[1], "--web") == 0)) {
        start_web_server();
        return 0;
    }

    while(1) {
        printf("\n========================================\n");
        printf("   SMART SHUTTLE - TICKET AUTOMATION    \n");
        printf("========================================\n");
        printf("1. Student Panel (Book/Cancel)\n");
        printf("2. Admin Panel (Manage/Reports)\n");
        printf("3. Start Web Server (Browser Mode)\n");
        printf("4. Exit System\n");
        printf("========================================\n");
        printf("Enter choice (1-4): ");
        if (scanf("%d", &userChoice) != 1) {
            printf("Invalid input! Exiting...\n");
            break;
        }

        switch(userChoice) {
            case 1:
                studentModule();
                break;
            case 2:
                adminModule();
                break;
            case 3:
                start_web_server();
                break;
            case 4:
                saveData();
                printf("Saving data... Thank you for using Smart Shuttle!\n");
                exit(0);
            default:
                printf("Invalid option! Please try again.\n");
        }
    }
    return 0;
}
