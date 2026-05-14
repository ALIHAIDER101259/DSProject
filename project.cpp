#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace std;

struct Passenger 
{
    int passengerId;
    string fullName;
    int age;
    int seatNo;
    Passenger* next;
};

struct FlightRecord {
    int flightID;
    int maxCapacity;
    int reservedSeats;
    double ticketPrice;
    Passenger* passengerList; 
    FlightRecord *left, *right;
};

struct UndoLog 
{
    int fID;
    int pID;
    string pName;
    UndoLog* next;
};

UndoLog* undoStack = nullptr;

struct WaitlistNode 
{
    string name;
    int fID;
    WaitlistNode* next;
};

WaitlistNode* queueFront = nullptr;
WaitlistNode* queueRear = nullptr;

void systemLogin() 
{
    string user, pass;
    int attempts = 0;

    while (attempts < 3) 
    {
        cout << "\n----------- SYSTEM AUTHENTICATION -----------" << endl;
        cout << "Username: ";
        cin >> user;
        cout << "Password: ";
        cin >> pass;

        try {
            if (user == "admin" && pass == "riphah123") 
            {
                cout << "\nAccess Granted! Welcome to the System." << endl;
                return;
            } 
            else 
            {
                throw runtime_error("Invalid Credentials!"); 
            }
        }
        catch (const runtime_error& e) 
        {
            attempts++;
            cout << "Security Alert: " << e.what() << endl;
            cout << "Remaining Attempts: " << (3 - attempts) << endl;
            
            if (attempts == 3)
            {
                cout << "System Locked due to unauthorized access." << endl;
                exit(0);
            }
        }
    }
}

void addToUndoStack(int f, int id, string n) 
{
    UndoLog* newNode = new UndoLog();
    newNode->fID = f;
    newNode->pID = id;
    newNode->pName = n;
    newNode->next = undoStack;
    undoStack = newNode;
}

void addToWaitlist(string n, int f) 
{
    WaitlistNode* newNode = new WaitlistNode();
    newNode->name = n;
    newNode->fID = f;
    newNode->next = nullptr;
    if (queueRear == nullptr) 
    {
        queueFront = queueRear = newNode;
    } 
    else 
    {
        queueRear->next = newNode;
        queueRear = newNode;
    }
}

void saveToDisk(FlightRecord* root, ofstream& file) 
{
    if (root == nullptr) return;
    
    saveToDisk(root->left, file);

    Passenger* temp = root->passengerList;
    while (temp != nullptr) 
    {
        file << root->flightID << " " << temp->passengerId << " " 
             << temp->age << " " << temp->seatNo << " " << temp->fullName << endl;
        temp = temp->next;
    }
    saveToDisk(root->right, file);
}

void exportData(FlightRecord* root) 
{
    ofstream file("Database.txt");
    if (!file) 
    {
        cout << "Critical Error: File connection failed!" << endl;
        return;
    }

    
    saveToDisk(root, file);
    file.close();
    cout << "\nRecords successfully synchronized with 'Database.txt'." << endl;
}

FlightRecord* createFlight(FlightRecord* root, int id, int cap, double price)
{
    if (root == nullptr) 
    {
        FlightRecord* newNode = new FlightRecord();
        newNode->flightID = id;
        newNode->maxCapacity = cap;
        newNode->ticketPrice = price;
        newNode->reservedSeats = 0;
        newNode->passengerList = nullptr;
        newNode->left = newNode->right = nullptr;
        return newNode;
    }


    if (id < root->flightID)
        root->left = createFlight(root->left, id, cap, price);
    else
        root->right = createFlight(root->right, id, cap, price);
    return root;
}

FlightRecord* findFlight(FlightRecord* root, int id)
{
    if (root == nullptr || root->flightID == id)
        return root;
    if (id < root->flightID)
        return findFlight(root->left, id);
    return findFlight(root->right, id);
}

void showFlightSchedule(FlightRecord* root) 
{
    if (root == nullptr) return;

    showFlightSchedule(root->left);
    int vacancy = root->maxCapacity - root->reservedSeats;
    cout << "ID: " << root->flightID << " | Vacancy: " << vacancy 
         << " | Fare: RS " << root->ticketPrice << endl;
    showFlightSchedule(root->right);
}

void processBooking(FlightRecord* root) 
{
    int id;
    cout << "\nEnter Flight ID for Booking: ";
    cin >> id;

    FlightRecord* target = findFlight(root, id);

    if (target == nullptr) 
    {
        cout << "Error: Flight record not found!" << endl;
        return;
    }

    if (target->reservedSeats < target->maxCapacity) 
    {
        Passenger* p = new Passenger();
        cout << "Passenger Full Name: ";
        cin.ignore();
        getline(cin, p->fullName);
        cout << "Age: ";
        cin >> p->age;
        cout << "Passport/CNIC ID: ";
        cin >> p->passengerId;

        p->seatNo = ++(target->reservedSeats);
        p->next = target->passengerList;
        target->passengerList = p;

        cout << "Success: Seat #" << p->seatNo << " confirmed." << endl;
    } 
    else 
    {
        string n;
        cout << "Capacity Full! Enter name for Queue Waitlist: ";
        cin.ignore();
        getline(cin, n);
        addToWaitlist(n, id);
    }
}

void processCancellation(FlightRecord* root)
{
    int fID, pID;


    cout << "\nFlight ID: ";
    cin >> fID;


    cout << "Passenger ID: ";
    cin >> pID;

    FlightRecord* target = findFlight(root, fID);
    if (target == nullptr) return;

    Passenger *curr = target->passengerList, *prev = nullptr;

    while (curr != nullptr && curr->passengerId != pID) 
    {
        prev = curr;
        curr = curr->next;
    }

    if (curr != nullptr) 
    
    {
        addToUndoStack(fID, curr->passengerId, curr->fullName);

        if (prev == nullptr)


            target->passengerList = curr->next;
        else 
            prev->next = curr->next;

        delete curr;
        target->reservedSeats--;
        cout << "Success: Reservation revoked." << endl;
    }
}

int main() {
    systemLogin();

    FlightRecord* root = nullptr;

    root = createFlight(root, 101, 50, 15000);
    root = createFlight(root, 105, 40, 18000);
    root = createFlight(root, 202, 30, 22000);
    root = createFlight(root, 303, 25, 80000);
    root = createFlight(root, 404, 60, 12000);
    root = createFlight(root, 505, 15, 45000);
    root = createFlight(root, 606, 40, 32000);
    root = createFlight(root, 707, 20, 19000);
    root = createFlight(root, 808, 10, 95000);
    root = createFlight(root, 909, 100, 10000);

    int menuChoice;
    while (true) 
    {
        cout << "\n--- AIRLINE RESOURCE MANAGEMENT ---" << endl;
        cout << "1. Display Active Flights" << endl;
        cout << "2. Initialize New Booking" << endl;
        cout << "3. Terminate Reservation" << endl;
        cout << "4. Sync Data to Disk (File)" << endl;
        cout << "5. Exit Application" << endl;
        cout << "Command: ";
        cin >> menuChoice;

        if (menuChoice == 1) 


            showFlightSchedule(root);

        else if (menuChoice == 2) 

            processBooking(root);
        else if (menuChoice == 3) 

            processCancellation(root);

        else if (menuChoice == 4)
            exportData(root);
        else if (menuChoice == 5) 
            break;
    }
    return 0;
}