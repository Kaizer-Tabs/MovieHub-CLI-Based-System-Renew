#include <iostream>
#include "include/TMDB.h"
#include <curl/curl.h>
extern "C"
{
#include "sqlite/sqlite3.h"
}

using namespace std;

int watchlistCount = 0;
int historyCount = 0;
int ratingsCount = 0;

double averageRating = 0.0;
int highestRating = 0;
int lowestRating = 0;

void watchlistMenu(sqlite3 *db);

void addMovieToWatchlist(sqlite3 *db);
void viewWatchlist(sqlite3 *db);
void removeMovieFromWatchlist(sqlite3 *db);

void historyMenu(sqlite3 *db);

void markAsWatched(sqlite3 *db);
void viewHistory(sqlite3 *db);

void ratingsMenu(sqlite3 *db);

void rateMovie(sqlite3 *db);
void viewRatings(sqlite3 *db);

void statisticsMenu(sqlite3 *db);
void viewStatistics(sqlite3 *db);

void recommendationsMenu(sqlite3 *db);

void favoriteMovies(sqlite3 *db);
void topTenMovies(sqlite3 *db);

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

void historyMenu(sqlite3 *db)
{
    int choice;

    do
    {
        cout << "\n====== HISTORY ======\n";
        cout << "1. Mark as Watched\n";
        cout << "2. View History\n";
        cout << "3. Back to Main Menu\n";
        cout << "Choice: ";

        cin >> choice;

        switch (choice)
        {
        case 1:
            markAsWatched(db);
            break;

        case 2:
            viewHistory(db);
            break;

        case 3:
            cout << "Returning to main menu.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 3);
}

void markAsWatched(sqlite3 *db)
{
    string movieTitle;

    cin.ignore();

    cout << "Movie Title to Mark as Watched: ";
    getline(cin, movieTitle);

    string sql = "INSERT INTO history (movie_title, watched_date) VALUES ('" + movieTitle + "', datetime('now'));";

    string deleteSql = "DELETE FROM watchlist WHERE movie_title = '" + movieTitle + "';";

    char *errMsg = nullptr;

    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc == SQLITE_OK)
    {
        sqlite3_exec(
            db,
            deleteSql.c_str(),
            nullptr,
            nullptr,
            &errMsg);

        cout << "Movie marked as watched.\n";
    }
    else
    {
        cout << "Error marking movie: " << errMsg << "\n";

        sqlite3_free(errMsg);
    }
}

static int historyCallback(void *data, int argc, char **argv, char **azColName)
{
    cout << "- " << argv[1] << " | " << argv[2] << "\n";
    return 0;
}

void viewHistory(sqlite3 *db)
{
    cout << "\n====== HISTORY ======\n";

    string sql = "SELECT * FROM history;";

    sqlite3_exec(db, sql.c_str(), historyCallback, nullptr, nullptr);
}

void ratingsMenu(sqlite3 *db)
{
    int choice;

    do
    {
        cout << "\n====== RATINGS ======\n";
        cout << "1. Rate Movie\n";
        cout << "2. View Ratings\n";
        cout << "3. Back to Main Menu\n";
        cout << "Choice: ";

        cin >> choice;

        switch (choice)
        {
        case 1:
            rateMovie(db);
            break;

        case 2:
            viewRatings(db);
            break;

        case 3:
            cout << "Returning to main menu.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 3);
}

void rateMovie(sqlite3 *db)
{
    string movieTitle;
    int rating;

    cin.ignore();

    cout << "Movie Title to Rate: ";
    getline(cin, movieTitle);

    cout << "Rating (1-5): ";
    cin >> rating;

    if (rating < 1 || rating > 5)
    {
        cout << "Invalid rating. Please enter a value between 1 and 5.\n";
        return;
    }

    string sql = "INSERT INTO ratings (movie_title, rating) VALUES ('" + movieTitle + "', " + to_string(rating) + ");";

    char *errMsg = nullptr;

    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc == SQLITE_OK)
    {
        cout << "Movie rated successfully.\n";
    }
    else
    {
        cout << "Error rating movie: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

static int ratingsCallback(void *data, int argc, char **argv, char **azColName)
{
    cout << "- " << argv[1] << " | " << argv[2] << "/5" << "\n";
    return 0;
}

void viewRatings(sqlite3 *db)
{
    cout << "\n====== RATINGS ======\n";

    string sql = "SELECT * FROM ratings;";

    char *errMsg = nullptr;

    int rc = sqlite3_exec(db, sql.c_str(), ratingsCallback, nullptr, &errMsg);

    if (rc != SQLITE_OK)
    {
        cout << "Error retrieving ratings: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

void statisticsMenu(sqlite3 *db)
{
    viewStatistics(db);
}

static int statisticsCallback(void *data, int argc, char **argv, char **azColName)
{
    if (argv[0] != nullptr)
    {
        *(double *)data = atof(argv[0]);
    }
    return 0;
}

static int intCallback(void *data, int argc, char **argv, char **azColName)
{
    if (argv[0] != nullptr)
    {
        *(int *)data = atoi(argv[0]);
    }
    return 0;
}

void viewStatistics(sqlite3 *db)
{
    string sqlwatchlist = "SELECT COUNT(*) FROM watchlist;";
    sqlite3_exec(db, sqlwatchlist.c_str(), intCallback, &watchlistCount, nullptr);

    string sqlhistory = "SELECT COUNT(*) FROM history;";
    sqlite3_exec(db, sqlhistory.c_str(), intCallback, &historyCount, nullptr);

    string sqlratings = "SELECT COUNT(*) FROM ratings;";
    sqlite3_exec(db, sqlratings.c_str(), intCallback, &ratingsCount, nullptr);

    string sqlaverage = "SELECT AVG(rating) FROM ratings;";
    sqlite3_exec(db, sqlaverage.c_str(), statisticsCallback, &averageRating, nullptr);

    string sqlhighest = "SELECT MAX(rating) FROM ratings;";
    sqlite3_exec(db, sqlhighest.c_str(), intCallback, &highestRating, nullptr);

    string sqllowest = "SELECT MIN(rating) FROM ratings;";
    sqlite3_exec(db, sqllowest.c_str(), intCallback, &lowestRating, nullptr);

    cout << "\n====== STATISTICS ======\n";
    cout << "Movies in Watchlist: " << watchlistCount << "\n";
    cout << "Movies Watched: " << historyCount << "\n";
    cout << "Movies Rated: " << ratingsCount << "\n";
    cout << "Average Rating: " << averageRating << "\n";
    cout << "Highest Rating: " << highestRating << "\n";
    cout << "Lowest Rating: " << lowestRating << "\n";
}

void recommendationsMenu(sqlite3 *db)
{
    int choice;

    do
    {
        cout << "\n====== RECOMMENDATIONS ======\n";
        cout << "1. Favorite Movies\n";
        cout << "2. Top 10 Movies\n";
        cout << "3. Back to Main Menu\n";
        cout << "Choice: ";

        cin >> choice;

        switch (choice)
        {
        case 1:
            favoriteMovies(db);
            break;

        case 2:
            topTenMovies(db);
            break;

        case 3:
            cout << "Returning to main menu.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 3);
}

static int favoriteMoviesCallback(void *data, int argc, char **argv, char **azColName)
{
    cout << "- " << argv[0] << endl;
    return 0;
}

void favoriteMovies(sqlite3 *db)
{
    cout << "\n====== FAVORITE MOVIES ======\n";

    string sql = "SELECT movie_title FROM ratings WHERE rating = 5;";

    sqlite3_exec(db, sql.c_str(), favoriteMoviesCallback, nullptr, nullptr);
}

static int topTenMoviesCallback(void *data, int argc, char **argv, char **azColName)
{
    cout << "- " << argv[0] << " (" << argv[1] << "/5)" << endl;
    return 0;
}

void topTenMovies(sqlite3 *db)
{
    cout << "\n====== TOP 10 MOVIES ======\n";

    string sql = "SELECT movie_title, rating FROM ratings ORDER BY rating DESC LIMIT 10;";

    sqlite3_exec(db, sql.c_str(), topTenMoviesCallback, nullptr, nullptr);
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
            cin.ignore();
            searchMovie(db);
            break;

        case 2:
            watchlistMenu(db);
            break;

        case 3:
            historyMenu(db);
            break;

        case 4:
            ratingsMenu(db);
            break;

        case 5:
            statisticsMenu(db);
            break;

        case 6:
            recommendationsMenu(db);
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