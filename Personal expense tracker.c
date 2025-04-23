#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50

const char *accounts_file = "accounts.txt";

float balance = 0;
float initialSalary = 0;

typedef struct {
    char category[50];
    char description[100];
    float amount;
    char datetime[25];
    int month;
} Expense;

char current_user[MAX_USERNAME_LEN];

void getTime(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 25, "%d/%m/%Y %H:%M:%S", t);
}

void createAccount() {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char passwordConfirm[MAX_PASSWORD_LEN];
    
    printf("\nCREATE NEW ACCOUNT\n");
    printf("ENTER USERNAME: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0; 

    FILE *file = fopen(accounts_file, "r");
    if (file) {
        char line[150];
        while (fgets(line, sizeof(line), file)) {
            char existing_username[MAX_USERNAME_LEN], existing_password[MAX_PASSWORD_LEN];
            if (sscanf(line, " %[^\n]|%s", existing_username, existing_password) == 2) {
                if (strcmp(existing_username, username) == 0) {
                    printf("USERNAME ALREADY EXISTS. PLEASE CHOOSE A DIFFERENT ONE.\n");
                    fclose(file);
                    return;
                }
            }
        }
        fclose(file);
    }

    printf("ENTER PASSWORD: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    printf("CONFIRM PASSWORD: ");
    fgets(passwordConfirm, sizeof(passwordConfirm), stdin);
    passwordConfirm[strcspn(passwordConfirm, "\n")] = 0;

    if (strcmp(password, passwordConfirm) != 0) {
        printf("PASSWORDS DO NOT MATCH! PLEASE TRY AGAIN.\n");
        return;
    }

    file = fopen(accounts_file, "a");
    if (file) {
        fprintf(file, "%s|%s\n", username, password);
        fclose(file);
        printf("ACCOUNT CREATED SUCCESSFULLY! YOU CAN NOW LOG IN.\n");
    } else {
        printf("ERROR CREATING ACCOUNT. PLEASE TRY AGAIN LATER.\n");
    }
}

int login() {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char line[150];

    printf("ENTER USERNAME: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("ENTER PASSWORD: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    FILE *file = fopen(accounts_file, "r");
    if (!file) {
        printf("NO ACCOUNTS FOUND. PLEASE CREATE AN ACCOUNT FIRST.\n");
        return 0;
    }

    int logged_in = 0;
    while (fgets(line, sizeof(line), file)) {
        char stored_username[MAX_USERNAME_LEN], stored_password[MAX_PASSWORD_LEN];
        if (sscanf(line, " %[^\n]|%s", stored_username, stored_password) == 2) {
            if (strcmp(username, stored_username) == 0 && strcmp(password, stored_password) == 0) {
                logged_in = 1;
                strcpy(current_user, username);
                break;
            }
        }
    }

    fclose(file);

    if (logged_in) {
        printf("LOGIN SUCCESSFUL!\n");

        char balance_file[100];
        sprintf(balance_file, "%s_balance.txt", current_user);
        FILE *bfile = fopen(balance_file, "r");
        if (bfile) {
            fscanf(bfile, "%f %f", &initialSalary, &balance);
            fclose(bfile);
            printf("WELCOME BACK! YOUR CURRENT BALANCE IS: RS %.2f\n", balance);
        } else {
            printf("ENTER YOUR TOTAL BALANCE (SALARY): RS ");
            scanf("%f", &initialSalary);
            balance = initialSalary;
            bfile = fopen(balance_file, "w");
            if (bfile) {
                fprintf(bfile, "%.2f %.2f", initialSalary, balance);
                fclose(bfile);
            }
            printf("BALANCE SET TO RS %.2f\n", balance);
        }

        return 1;
    } else {
        printf("INVALID USERNAME OR PASSWORD. PLEASE TRY AGAIN.\n");
        return 0;
    }
}

void updateBalanceFile() {
    char balance_file[100];
    sprintf(balance_file, "%s_balance.txt", current_user);
    FILE *file = fopen(balance_file, "w");
    if (file) {
        fprintf(file, "%.2f %.2f", initialSalary, balance);
        fclose(file);
    }
}

void saveExpenseToFile(Expense e) {
    char expense_file[100];
    sprintf(expense_file, "%s_expenses.txt", current_user);

    FILE *file = fopen(expense_file, "a");
    if (file) {
        fprintf(file, "%s|%s|%.2f|%s|%d\n", e.category, e.description, e.amount, e.datetime, e.month);
        fclose(file);
    }
}

void addExpense() {
    if (balance <= 0) {
        printf("\nINSUFFICIENT BALANCE. CANNOT ADD EXPENSES.\n");
        return;
    }

    Expense e;
    printf("\n========== ADD NEW EXPENSE ==========\n");

    printf("ENTER THE MONTH (1-12): ");
    scanf("%d", &e.month);
    if (e.month < 1 || e.month > 12) {
        printf("INVALID MONTH! PLEASE ENTER A VALUE BETWEEN 1 AND 12.\n");
        return;
    }

    printf("CATEGORY       : ");
    scanf(" %[^\n]", e.category);
    printf("DESCRIPTION    : ");
    scanf(" %[^\n]", e.description);
    printf("AMOUNT (RS)    : ");
    scanf("%f", &e.amount);

    if (e.amount > balance) {
        printf("INSUFFICIENT BALANCE. CURRENT BALANCE: RS %.2f\n", balance);
        return;
    }

    getTime(e.datetime);
    balance -= e.amount;
    updateBalanceFile();
    saveExpenseToFile(e);
    printf("EXPENSE ADDED SUCCESSFULLY! REMAINING BALANCE: RS %.2f\n", balance);
}

void viewExpensesByCategoryTable() {
    char expense_file[100];
    sprintf(expense_file, "%s_expenses.txt", current_user);
    FILE *file = fopen(expense_file, "r");
    if (!file) {
        printf("\nNO EXPENSES FOUND.\n");
        return;
    }

    Expense expenses[500];
    int count = 0;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^|]|%[^|]|%f|%[^|]|%d", expenses[count].category, expenses[count].description, &expenses[count].amount, expenses[count].datetime, &expenses[count].month);
        count++;
    }
    fclose(file);

    char printed[100][50];
    int printedCount = 0;

    printf("\n========== EXPENSE REPORT (CATEGORY-WISE) ==========\n");

    int i, j;
    for (i = 0; i < count; i++) {
        int found = 0;
        for (j = 0; j < printedCount; j++) {
            if (strcmp(expenses[i].category, printed[j]) == 0) {
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("\nCATEGORY: %s\n", expenses[i].category);
            printf("%-3s %-25s %-10s %-20s\n", "NO", "DESCRIPTION", "AMOUNT", "DATE & TIME");
            printf("---------------------------------------------------------------\n");

            int serial = 1;
            for (j = 0; j < count; j++) {
                if (strcmp(expenses[j].category, expenses[i].category) == 0) {
                    printf("%-3d %-25s RS %-7.2f %-20s\n", serial++, expenses[j].description, expenses[j].amount, expenses[j].datetime);
                }
            }

            strcpy(printed[printedCount], expenses[i].category);
            printedCount++;
        }
    }
}

void viewExpensesByMonth() {
    int targetMonth;
    printf("ENTER THE MONTH TO FILTER (1-12): ");
    scanf("%d", &targetMonth);
    if (targetMonth < 1 || targetMonth > 12) {
        printf("INVALID MONTH.\n");
        return;
    }

    char expense_file[100];
    sprintf(expense_file, "%s_expenses.txt", current_user);
    FILE *file = fopen(expense_file, "r");
    if (!file) {
        printf("\nNO EXPENSES FOUND.\n");
        return;
    }

    char line[256];
    Expense e;
    int serial = 1;
    float total = 0;

    printf("\n========== EXPENSES FOR MONTH %d ==========\n", targetMonth);
    printf("%-3s %-15s %-25s %-10s %-20s\n", "NO", "CATEGORY", "DESCRIPTION", "AMOUNT", "DATE & TIME");
    printf("-------------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^|]|%[^|]|%f|%[^|]|%d", e.category, e.description, &e.amount, e.datetime, &e.month);
        if (e.month == targetMonth) {
            printf("%-3d %-15s %-25s RS %-7.2f %-20s\n", serial++, e.category, e.description, e.amount, e.datetime);
            total += e.amount;
        }
    }

    printf("\nTOTAL FOR MONTH %d: RS %.2f\n", targetMonth, total);
    fclose(file);
}

void printAllExpenses() {
    char expense_file[100];
    sprintf(expense_file, "%s_expenses.txt", current_user);
    FILE *file = fopen(expense_file, "r");
    if (!file) {
        printf("\nNO EXPENSES TO DISPLAY.\n");
        return;
    }

    char line[256];
    Expense e;
    int serial = 1;
    float totalSpent = 0;

    printf("\n========== ALL EXPENSES ==========\n");
    printf("%-3s %-15s %-25s %-10s %-20s\n", "NO", "CATEGORY", "DESCRIPTION", "AMOUNT", "DATE & TIME");
    printf("-------------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^|]|%[^|]|%f|%[^|]|%d", e.category, e.description, &e.amount, e.datetime, &e.month);
        printf("%-3d %-15s %-25s RS %-7.2f %-20s\n", serial++, e.category, e.description, e.amount, e.datetime);
        totalSpent += e.amount;
    }

    fclose(file);

    printf("\nTOTAL SPENT       : RS %.2f\n", totalSpent);
    printf("REMAINING BALANCE : RS %.2f\n", balance);
}

void deleteAllExpenses() {
    char expense_file[100];
    sprintf(expense_file, "%s_expenses.txt", current_user);
    FILE *file = fopen(expense_file, "w");
    if (file) fclose(file);

    balance = initialSalary;
    updateBalanceFile();
    printf("\nALL EXPENSES HAVE BEEN DELETED. BALANCE RESET TO RS %.2f\n", balance);
}

void deleteAccount() {
    char line[200], username[MAX_USERNAME_LEN], password[MAX_PASSWORD_LEN];
    printf("ARE YOU SURE YOU WANT TO DELETE YOUR ACCOUNT? (YES TO CONFIRM): ");
    fgets(line, sizeof(line), stdin);
    line[strcspn(line, "\n")] = 0;
    if (strcmp(line, "YES") != 0) return;

    FILE *file = fopen(accounts_file, "r");
    FILE *temp = fopen("temp.txt", "w");

    while (fscanf(file, " %[^\n]|%s", username, password) != EOF) {
        if (strcmp(username, current_user) != 0) {
            fprintf(temp, "%s|%s\n", username, password);
        }
    }

    fclose(file);
    fclose(temp);

    remove(accounts_file);
    rename("temp.txt", accounts_file);

    char expense_file[100], balance_file[100];
    sprintf(expense_file, "%s_expenses.txt", current_user);
    sprintf(balance_file, "%s_balance.txt", current_user);
    remove(expense_file);
    remove(balance_file);

    printf("YOUR ACCOUNT AND ALL RELATED DATA HAVE BEEN DELETED.\n");
}

void displayMenu() {
    printf("\n         ============================================\n");
    printf("                    PERSONAL EXPENSE TRACKER\n");
    printf("         ============================================\n");
    printf("                1. ADD EXPENSE\n");
    printf("                2. VIEW ALL EXPENSES\n");
    printf("                3. VIEW EXPENSES BY CATEGORY\n");
    printf("                4. VIEW EXPENSES BY MONTH\n");
    printf("                5. DELETE ALL EXPENSES\n");
    printf("                6. VIEW EXPENSES\n");
    printf("                7. DELETE ACCOUNT\n");
    printf("                8. EXIT\n");
    printf("         ============================================\n");
    printf("ENTER YOUR CHOICE: ");
}

int main() {
    int choice;
    int loggedIn = 0;

    while (1) {
        if (!loggedIn) {
            printf("\n\t\t\t\t\t=========== WELCOME TO EXPENSE TRACKER ===========\n\n");
            printf("\t\t\t\t\t\t\t1. CREATE ACCOUNT\t\t\t\t\t\t\t\t\t\n");
            printf(" \t\t\t\t\t\t\t2. LOGIN\t\t\t\t\t\t\t\t\t\n");
            printf("\t\t\t\t\t\t\t3. EXIT\t\t\t\t\t\t\t\t\t\n");
            printf("\n\t\t\t\t\t==================================================\n");
            printf("ENTER YOUR CHOICE: ");
            int action; 
            scanf("%d", &action);

            if (action == 1) createAccount();
            else if (action == 2) loggedIn = login();
            else if (action == 3) {
                printf("EXITING THE PROGRAM. GOODBYE!\n");
                exit(0);
            } else {
                printf("INVALID CHOICE. PLEASE TRY AGAIN.\n");
            }
        } else {
            displayMenu();
            scanf("%d", &choice);

            switch (choice) {
                case 1: 
					addExpense(); 
					break;
                case 2: 
					printAllExpenses(); 
					break;
                case 3: 
					viewExpensesByCategoryTable(); 
					break;
                case 4: 
					viewExpensesByMonth(); 
					break;
                case 5: 
					deleteAllExpenses(); 
					break;
                case 6: 
					printAllExpenses(); 
					break;
                case 7: 
					deleteAccount(); 
					loggedIn = 0; 
					break;
                case 8: 
                    printf("THANK YOU FOR USING EXPENSE TRACKER!\n"); 
                    exit(0);
                default: 
                    printf("INVALID CHOICE. PLEASE TRY AGAIN.\n");
            }
        }
    }

    return 0;
}

