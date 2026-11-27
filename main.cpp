#include <iostream>
#include "include/TMDB.h"
#include <curl/curl.h>
#include <cstdlib>
#include <vector>

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

void viewWatchlist(sqlite3 *db);
void removeMovieFromWatchlist(sqlite3 *db);

void historyMenu(sqlite3 *db);

void markAsWatched(sqlite3 *db);
void viewHistory(sqlite3 *db);

void ratingsMenu(sqlite3 *db);

void rateMovie(sqlite3 *db);
void changeMovieRating(sqlite3 *db);
void viewRatings(sqlite3 *db);

void statisticsMenu(sqlite3 *db);
void viewStatistics(sqlite3 *db);

void recommendationsMenu(sqlite3 *db);

void createTables(sqlite3 *db);
void showMenu();

struct MovieRecord
{
    sqlite3_int64 id;
    string title;
};

struct RatedMovie
{
    string title;
    int rating;
};

static int movieRecordCallback(void *data, int argc, char **argv, char **azColName)
{
    if (argc < 2 || argv[0] == nullptr || argv[1] == nullptr)
    {
        return 0;
    }

    vector<MovieRecord> *movies = static_cast<vector<MovieRecord> *>(data);
    movies->push_back({static_cast<sqlite3_int64>(atoll(argv[0])), argv[1]});
    return 0;
}

static bool readMovieRecords(sqlite3 *db, const string &sql, vector<MovieRecord> &movies)
{
    movies.clear();
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), movieRecordCallback, &movies, &errMsg);
    if (rc != SQLITE_OK)
    {
        cerr << "Error reading movies: " << (errMsg == nullptr ? sqlite3_errmsg(db) : errMsg) << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

static void printMovieRecords(const vector<MovieRecord> &movies)
{
    for (size_t i = 0; i < movies.size(); ++i)
    {
        cout << i + 1 << ". " << movies[i].title << "\n";
    }
}

void watchlistMenu(sqlite3 *db)
{
    int choice;

    do
    {
        cout << "\n====== WATCHLIST ======\n";
        cout << "1. View Watchlist\n";
        cout << "2. Remove Movie\n";
        cout << "3. Back to Main Menu\n";
        cout << "Choice: ";

        cin >> choice;

        switch (choice)
        {
        case 1:
            viewWatchlist(db);
            break;

        case 2:
            removeMovieFromWatchlist(db);
            break;

        case 3:
            cout << "Returning to main menu.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 3);
}

void viewWatchlist(sqlite3 *db)
{
    cout << "\n====== WATCHLIST ======\n";

    vector<MovieRecord> movies;
    if (!readMovieRecords(db, "SELECT id, movie_title FROM watchlist ORDER BY id;", movies))
    {
        return;
    }
    if (movies.empty())
    {
        cout << "Your watchlist is empty. Search for a movie to add one.\n";
        return;
    }
    printMovieRecords(movies);
}

void removeMovieFromWatchlist(sqlite3 *db)
{
    vector<MovieRecord> movies;
    if (!readMovieRecords(db, "SELECT id, movie_title FROM watchlist ORDER BY id;", movies))
    {
        return;
    }
    if (movies.empty())
    {
        cout << "Your watchlist is empty.\n";
        return;
    }

    cout << "\n====== SELECT MOVIE TO REMOVE ======\n";
    printMovieRecords(movies);

    int selection;
    cout << "Select Movie (0 to cancel): ";
    cin >> selection;
    if (selection < 1 || static_cast<size_t>(selection) > movies.size())
    {
        cout << "Cancelled.\n";
        return;
    }

    sqlite3_stmt *statement = nullptr;
    int rc = sqlite3_prepare_v2(db, "DELETE FROM watchlist WHERE id = ?;", -1, &statement, nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_int64(statement, 1, movies[selection - 1].id);
        rc = sqlite3_step(statement);
    }
    bool removed = rc == SQLITE_DONE && sqlite3_changes(db) == 1;
    sqlite3_finalize(statement);

    if (removed)
    {
        cout << "Movie removed from watchlist.\n";
    }
    else
    {
        cerr << "Error removing movie: " << sqlite3_errmsg(db) << "\n";
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
    vector<MovieRecord> movies;
    if (!readMovieRecords(db, "SELECT id, movie_title FROM watchlist ORDER BY id;", movies))
    {
        return;
    }
    if (movies.empty())
    {
        cout << "Your watchlist is empty. Add movies from Search Movie first.\n";
        return;
    }

    cout << "\n====== SELECT MOVIE TO MARK AS WATCHED ======\n";
    printMovieRecords(movies);

    int selection;
    cout << "Select Movie (0 to cancel): ";
    cin >> selection;
    if (selection < 1 || static_cast<size_t>(selection) > movies.size())
    {
        cout << "Cancelled.\n";
        return;
    }

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        cerr << "Error starting watchlist update: "
             << (errMsg == nullptr ? sqlite3_errmsg(db) : errMsg) << "\n";
        sqlite3_free(errMsg);
        return;
    }

    sqlite3_stmt *insertStatement = nullptr;
    rc = sqlite3_prepare_v2(
        db,
        "INSERT INTO history (movie_title, watched_date) "
        "SELECT movie_title, datetime('now') FROM watchlist WHERE id = ?;",
        -1,
        &insertStatement,
        nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_int64(insertStatement, 1, movies[selection - 1].id);
        rc = sqlite3_step(insertStatement);
    }
    bool inserted = rc == SQLITE_DONE && sqlite3_changes(db) == 1;
    sqlite3_finalize(insertStatement);

    if (!inserted)
    {
        cerr << "Error marking movie as watched: " << sqlite3_errmsg(db) << "\n";
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    sqlite3_stmt *deleteStatement = nullptr;
    rc = sqlite3_prepare_v2(db, "DELETE FROM watchlist WHERE id = ?;", -1, &deleteStatement, nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_int64(deleteStatement, 1, movies[selection - 1].id);
        rc = sqlite3_step(deleteStatement);
    }
    bool removed = rc == SQLITE_DONE && sqlite3_changes(db) == 1;
    sqlite3_finalize(deleteStatement);

    if (!removed)
    {
        cerr << "Error removing movie from watchlist: " << sqlite3_errmsg(db) << "\n";
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        cerr << "Error saving watched movie: "
             << (errMsg == nullptr ? sqlite3_errmsg(db) : errMsg) << "\n";
        sqlite3_free(errMsg);
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }
    cout << "Movie marked as watched.\n";
}

static int historyCallback(void *data, int argc, char **argv, char **azColName)
{
    if (argc < 1 || argv[0] == nullptr)
    {
        return 0;
    }

    vector<string> *movies =
        static_cast<vector<string> *>(data);

    movies->push_back(argv[0]);

    return 0;
}

static int historyViewCallback(void *data, int argc, char **argv, char **azColName)
{
    int *count = static_cast<int *>(data);
    if (argc >= 2 && argv[0] != nullptr && argv[1] != nullptr)
    {
        cout << "- " << argv[0] << " | " << argv[1] << "\n";
        ++(*count);
    }
    return 0;
}

void viewHistory(sqlite3 *db)
{
    cout << "\n====== HISTORY ======\n";

    int count = 0;
    char *errMsg = nullptr;
    int rc = sqlite3_exec(
        db,
        "SELECT movie_title, watched_date FROM history ORDER BY id;",
        historyViewCallback,
        &count,
        &errMsg);
    if (rc != SQLITE_OK)
    {
        cerr << "Error retrieving history: "
             << (errMsg == nullptr ? sqlite3_errmsg(db) : errMsg) << "\n";
        sqlite3_free(errMsg);
    }
    else if (count == 0)
    {
        cout << "No watched movies in history.\n";
    }
}

void ratingsMenu(sqlite3 *db)
{
    int choice;

    do
    {
        cout << "\n====== RATINGS ======\n";
        cout << "1. Rate Unrated Watched Movie\n";
        cout << "2. View My Ratings\n";
        cout << "3. Change Existing Rating\n";
        cout << "4. Look Up TMDB Rating\n";
        cout << "5. TMDB Top Rated by Stars\n";
        cout << "6. Back to Main Menu\n";
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
            changeMovieRating(db);
            break;

        case 4:
            lookupTMDBRating();
            break;

        case 5:
            showTMDBTopRatedByStars();
            break;

        case 6:
            cout << "Returning to main menu.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 6);
}

void rateMovie(sqlite3 *db)
{
    vector<string> movies;

    string sql =
        "SELECT h.movie_title FROM history h "
        "WHERE NOT EXISTS ("
        "SELECT 1 FROM ratings r "
        "WHERE r.movie_title = h.movie_title COLLATE NOCASE"
        ") "
        "GROUP BY h.movie_title COLLATE NOCASE "
        "ORDER BY MIN(h.id);";

    char *errMsg = nullptr;

    int rc = sqlite3_exec(
        db,
        sql.c_str(),
        historyCallback,
        &movies,
        &errMsg);

    if (rc != SQLITE_OK)
    {
        cerr << "Error retrieving unrated watched movies: "
             << (errMsg == nullptr ? sqlite3_errmsg(db) : errMsg) << "\n";
        sqlite3_free(errMsg);
        return;
    }

    if (movies.empty())
    {
        cout << "\nNo unrated watched movies found. Use Change Existing Rating to update a rating.\n";
        return;
    }

    cout << "\n====== UNRATED WATCHED MOVIES ======\n\n";

    for (int i = 0; i < movies.size(); i++)
    {
        cout
            << i + 1
            << ". "
            << movies[i]
            << endl;
    }

    int selection;

    cout << "\nSelect Movie (0 to cancel): ";
    cin >> selection;

    if (selection < 1 ||
        selection > movies.size())
    {
        cout << "Cancelled.\n";
        return;
    }

    int rating;

    cout << "Enter Rating (1-5): ";
    cin >> rating;

    if (rating < 1 || rating > 5)
    {
        cout << "Invalid rating.\n";
        return;
    }

    string selectedMovie =
        movies[selection - 1];

    sqlite3_stmt *statement = nullptr;
    rc = sqlite3_prepare_v2(
        db,
        "INSERT INTO ratings (movie_title, rating) VALUES (?, ?);",
        -1,
        &statement,
        nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(statement, 1, selectedMovie.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(statement, 2, rating);
        rc = sqlite3_step(statement);
    }

    if (rc == SQLITE_DONE)
    {
        cout
            << "\nRating saved successfully.\n";
    }
    else
    {
        cerr << "\nError saving rating: " << sqlite3_errmsg(db) << "\n";
    }
    sqlite3_finalize(statement);
}

static int ratedMoviesCallback(void *data, int argc, char **argv, char **azColName)
{
    if (argc < 2 || argv[0] == nullptr || argv[1] == nullptr)
    {
        return 0;
    }

    vector<RatedMovie> *movies = static_cast<vector<RatedMovie> *>(data);
    movies->push_back({argv[0], atoi(argv[1])});
    return 0;
}

void changeMovieRating(sqlite3 *db)
{
    vector<RatedMovie> movies;
    const string sql =
        "SELECT r.movie_title, r.rating FROM ratings r "
        "WHERE r.id = ("
        "SELECT MAX(r2.id) FROM ratings r2 "
        "WHERE r2.movie_title = r.movie_title COLLATE NOCASE"
        ") "
        "ORDER BY r.movie_title COLLATE NOCASE;";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), ratedMoviesCallback, &movies, &errMsg);
    if (rc != SQLITE_OK)
    {
        cerr << "Error retrieving existing ratings: "
             << (errMsg == nullptr ? sqlite3_errmsg(db) : errMsg) << "\n";
        sqlite3_free(errMsg);
        return;
    }
    if (movies.empty())
    {
        cout << "No existing ratings to change.\n";
        return;
    }

    cout << "\n====== CHANGE EXISTING RATING ======\n";
    for (size_t i = 0; i < movies.size(); ++i)
    {
        cout << i + 1 << ". " << movies[i].title
             << " | Current rating: " << movies[i].rating << "/5\n";
    }

    int selection;
    cout << "Select Movie (0 to cancel): ";
    cin >> selection;
    if (selection < 1 || static_cast<size_t>(selection) > movies.size())
    {
        cout << "Cancelled.\n";
        return;
    }

    int rating;
    cout << "Enter new rating (1-5): ";
    cin >> rating;
    if (rating < 1 || rating > 5)
    {
        cout << "Invalid rating.\n";
        return;
    }

    sqlite3_stmt *statement = nullptr;
    rc = sqlite3_prepare_v2(
        db,
        "UPDATE ratings SET rating = ? WHERE movie_title = ? COLLATE NOCASE;",
        -1,
        &statement,
        nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, rating);
        sqlite3_bind_text(statement, 2, movies[selection - 1].title.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(statement);
    }
    bool updated = rc == SQLITE_DONE && sqlite3_changes(db) > 0;
    sqlite3_finalize(statement);

    if (updated)
    {
        cout << "Rating updated successfully.\n";
    }
    else
    {
        cerr << "Error updating rating: " << sqlite3_errmsg(db) << "\n";
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

    string sql =
        "SELECT r.id, r.movie_title, r.rating FROM ratings r "
        "WHERE r.id = ("
        "SELECT MAX(r2.id) FROM ratings r2 "
        "WHERE r2.movie_title = r.movie_title COLLATE NOCASE"
        ") "
        "ORDER BY r.movie_title COLLATE NOCASE;";

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
        cout << "1. Trending Movies\n";
        cout << "2. Popular Movies\n";
        cout << "3. Top Rated Movies\n";
        cout << "4. Personalized Recommendations\n";
        cout << "5. Back to Main Menu\n";
        cout << "Choice: ";

        cin >> choice;

        switch (choice)
        {
        case 1:
            showTMDBMovieList("/trending/movie/week", "TRENDING MOVIES");
            break;

        case 2:
            showTMDBMovieList("/movie/popular", "POPULAR MOVIES");
            break;

        case 3:
            showTMDBMovieList("/movie/top_rated", "TOP RATED MOVIES");
            break;

        case 4:
            showPersonalizedRecommendations(db);
            break;

        case 5:
            cout << "Returning to main menu.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 5);
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