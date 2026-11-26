#include <iostream>

extern "C"
{
#include "sqlite/sqlite3.h"
}

using namespace std;

void createTables(sqlite3 *db);
void showMenu();

void createTables(sqlite3 *db)
{
    char *errMsg = nullptr;

    string watchlist =
        "CREATE TABLE IF NOT EXISTS watchlist ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "movie_title TEXT NOT NULL"
        ");";

    string history =
        "CREATE TABLE IF NOT EXISTS history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "movie_title TEXT NOT NULL,"
        "watched_date TEXT NOT NULL"
        ");";

    string ratings =
        "CREATE TABLE IF NOT EXISTS ratings ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "movie_title TEXT NOT NULL,"
        "rating INTEGER NOT NULL"
        ");";

    sqlite3_exec(db, watchlist.c_str(), nullptr, nullptr, &errMsg);
    sqlite3_exec(db, history.c_str(), nullptr, nullptr, &errMsg);
    sqlite3_exec(db, ratings.c_str(), nullptr, nullptr, &errMsg);

    cout << "Database tables ready.\n";
}

void showMenu()
{
    cout << "\n====== MOVIE HUB ======\n";
    cout << "1. Search Movie\n";
    cout << "2. Watchlist\n";
    cout << "3. History\n";
    cout << "4. Ratings\n";
    cout << "5. Statistics\n";
    cout << "6. Recommendations\n";
    cout << "7. Exit\n";
    cout << "Choice: ";
}

int main()
{
    sqlite3 *db;

    if (sqlite3_open("database/moviehub.db", &db) != SQLITE_OK)
    {
        cout << "Failed to connect database.\n";
        return 1;
    }

    cout << "Database connected successfully!\n";

    createTables(db);

    int choice;

    do
    {
        showMenu();

        cin >> choice;

        switch (choice)
        {
        case 1:
            cout << "Search Movies coming soon.\n";
            break;

        case 2:
            cout << "Watchlist coming soon.\n";
            break;

        case 3:
            cout << "History coming soon.\n";
            break;

        case 4:
            cout << "Ratings coming soon.\n";
            break;

        case 5:
            cout << "Statistics coming soon.\n";
            break;

        case 6:
            cout << "Recommendations coming soon.\n";
            break;

        case 7:
            cout << "Goodbye!\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 7);

    sqlite3_close(db);

    return 0;
}