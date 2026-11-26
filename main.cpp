#include <iostream>

extern "C"
{
#include "sqlite/sqlite3.h"
}

using namespace std;

void watchlistMenu(sqlite3 *db);

void addMovieToWatchlist(sqlite3 *db);
void viewWatchlist(sqlite3 *db);
void removeMovieFromWatchlist(sqlite3 *db);

void createTables(sqlite3 *db);
void showMenu();

void watchlistMenu(sqlite3 *db)
{
    int choice;

    do
    {
        cout << "\n====== WATCHLIST ======\n";
        cout << "1. Add Movie\n";
        cout << "2. View Watchlist\n";
        cout << "3. Remove Movie\n";
        cout << "4. Back to Main Menu\n";
        cout << "Choice: ";

        cin >> choice;

        switch (choice)
        {
        case 1:
            addMovieToWatchlist(db);
            break;

        case 2:
            viewWatchlist(db);
            break;

        case 3:
            removeMovieFromWatchlist(db);
            break;

        case 4:
            cout << "Returning to main menu.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 4);
}

void addMovieToWatchlist(sqlite3 *db)
{
    string movieTitle;

    cin.ignore();

    cout << "Movie Title: ";
    getline(cin, movieTitle);

    string sql = "INSERT INTO watchlist (movie_title) VALUES ('" + movieTitle + "');";

    char *errMsg = nullptr;

    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc == SQLITE_OK)
    {
        cout << "Movie added to watchlist.\n";
    }
    else
    {
        cout << "Error adding movie: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

static int watchlistCallback(void *data, int argc, char **argv, char **azColName)
{
    cout << "- " << argv[1] << "\n";
    return 0;
}

void viewWatchlist(sqlite3 *db)
{
    cout << "\n====== WATCHLIST ======\n";

    string sql = "SELECT * FROM watchlist;";

    sqlite3_exec(db, sql.c_str(), watchlistCallback, nullptr, nullptr);
}

void removeMovieFromWatchlist(sqlite3 *db)
{
    string movieTitle;

    cin.ignore();

    cout << "Movie Title to Remove: ";
    getline(cin, movieTitle);

    string sql = "DELETE FROM watchlist WHERE movie_title = '" + movieTitle + "';";

    char *errMsg = nullptr;

    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc == SQLITE_OK)
    {
        cout << "Movie removed from watchlist.\n";
    }
    else
    {
        cout << "Error removing movie: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

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
            watchlistMenu(db);
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