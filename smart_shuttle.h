#ifndef SMART_SHUTTLE_H
#define SMART_SHUTTLE_H

#define MAX_BUS 3
#define SEATS_PER_BUS 30
#define SHUTTLE_FARE 20.0

typedef struct {
    int busId;
    char route[50];
    int seats[SEATS_PER_BUS]; // 0: empty, 1: booked
    int availableSeats;
} Bus;

typedef struct {
    int ticketId;
    char rollNo[20];
    char studentName[50];
    int busId;
    int seatNo;
    float amountPaid;
} Ticket;

typedef struct {
    char rollNo[20];
    char name[50];
    int status; // 0: pending, 1: approved, 2: blacklisted
} Student;

// Global array declaration
extern Bus busList[MAX_BUS];

// Core Functions
void loadData();
void saveData();

// Menus
void studentModule();
void adminModule();

// Student Functions
void viewRoutes();
void bookSeat();
void cancelTicket();
void generateTicketFile(Ticket t);

// Admin Functions
int verifyAdmin();
void viewAllBookings();
void checkRevenue();
void initializeBuses();

// API Functions
void apiGetRoutes();
void apiGetSeats(int busId);
void apiBookSeat(int busId, int seatNo, char* rollNo, char* name);
void apiGetTickets(char* rollNo);
void apiCancelTicket(int ticketId);
void apiRegisterStudent(char* rollNo, char* name);
void apiVerifyLogin(char* rollNo, char* name);

// Admin API
void apiVerifyAdmin(char* password);
void apiGetAllBookings();
void apiGetRevenue();
void apiResetBuses();
void apiGetPendingStudents();
void apiApproveStudent(char* rollNo);
void apiRejectStudent(char* rollNo);
void apiBlacklistStudent(char* rollNo);
void apiUnblacklistStudent(char* rollNo);
void apiGetAllStudents();

// Web Server
void start_web_server();

#endif


