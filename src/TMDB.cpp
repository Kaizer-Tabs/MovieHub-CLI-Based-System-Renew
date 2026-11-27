#include "../include/TMDB.h"
#include "../include/json.hpp"

#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <limits>

using namespace std;
using json = nlohmann::json;

namespace
{
int starsForTMDBRating(double rating)
{
    int stars = static_cast<int>(rating / 2.0 + 0.5);
    if (stars < 0)
    {
        return 0;
    }
    return stars > 5 ? 5 : stars;
}

void printTMDBRating(const json &movie)
{
    if (movie.value("vote_count", 0) <= 0)
    {
        cout << "Not rated yet\n";
        return;
    }

    double rating = movie.value("vote_average", 0.0);
    int stars = starsForTMDBRating(rating);
    cout << rating << "/10 | " << string(static_cast<size_t>(stars), '*')
         << " (" << stars << "/5 stars)\n";
}

bool fetchTMDBJson(
    const string &endpoint,
    const string &query,
    json &data,
    const string &additionalParams = string())
{
    string apiKey = readApiKey();
    if (apiKey.empty())
    {
        cerr << "TMDB API key is missing. Add it to config/tmdb_api_key.txt.\n";
        return false;
    }

    CURL *curl = curl_easy_init();
    if (curl == nullptr)
    {
        cerr << "Unable to initialize the TMDB request.\n";
        return false;
    }

    char *encodedKey = curl_easy_escape(curl, apiKey.c_str(), static_cast<int>(apiKey.size()));
    char *encodedQuery = nullptr;
    if (!query.empty())
    {
        encodedQuery = curl_easy_escape(curl, query.c_str(), static_cast<int>(query.size()));
    }

    if (encodedKey == nullptr || (!query.empty() && encodedQuery == nullptr))
    {
        cerr << "Unable to encode the TMDB request.\n";
        curl_free(encodedKey);
        curl_free(encodedQuery);
        curl_easy_cleanup(curl);
        return false;
    }

    string url = "https://api.themoviedb.org/3" + endpoint + "?api_key=" + encodedKey;
    if (encodedQuery != nullptr)
    {
        url += "&query=";
        url += encodedQuery;
    }
    if (!additionalParams.empty())
    {
        url += "&" + additionalParams;
    }
    curl_free(encodedKey);
    curl_free(encodedQuery);

    string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode result = curl_easy_perform(curl);
    long statusCode = 0;
    if (result == CURLE_OK)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
    }
    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
    {
        cerr << "TMDB request failed: " << curl_easy_strerror(result) << "\n";
        return false;
    }
    if (statusCode < 200 || statusCode >= 300)
    {
        cerr << "TMDB returned HTTP " << statusCode << ". Check the API key and try again.\n";
        return false;
    }

    try
    {
        data = json::parse(response);
    }
    catch (const json::exception &error)
    {
        cerr << "Unable to read TMDB response: " << error.what() << "\n";
        return false;
    }
    return true;
}

void printMovieResults(const json &data, const string &heading)
{
    if (!data.is_object() || !data.contains("results") || !data["results"].is_array())
    {
        cerr << "TMDB response did not contain movie results.\n";
        return;
    }

    cout << "\n====== " << heading << " ======\n\n";
    const json &results = data["results"];
    if (results.empty())
    {
        cout << "No movies found.\n";
        return;
    }

    size_t count = results.size() < 10 ? results.size() : 10;
    for (size_t i = 0; i < count; ++i)
    {
        const json &movie = results[i];
        string title = movie.value("title", string("Untitled"));
        string releaseDate = movie.value("release_date", string());

        cout << i + 1 << ". " << title;
        if (releaseDate.size() >= 4)
        {
            cout << " (" << releaseDate.substr(0, 4) << ")";
        }
        cout << " | TMDB rating: ";
        printTMDBRating(movie);
    }
}
}

string readApiKey()
{
    ifstream file("config/tmdb_api_key.txt");

    string apiKey;

    getline(file, apiKey);

    return apiKey;
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output)
{
    output->append((char *)contents, size * nmemb);
    return size * nmemb;
}

void showTMDBMovieList(const string &endpoint, const string &heading)
{
    json data;
    if (fetchTMDBJson(endpoint, string(), data))
    {
        printMovieResults(data, heading);
    }
}

void lookupTMDBRating()
{
    string title;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Movie title to look up: ";
    getline(cin, title);
    if (title.empty())
    {
        cout << "Movie title cannot be empty.\n";
        return;
    }

    json data;
    if (!fetchTMDBJson("/search/movie", title, data))
    {
        return;
    }
    if (!data.is_object() || !data.contains("results") || !data["results"].is_array())
    {
        cerr << "TMDB response did not contain movie results.\n";
        return;
    }
    if (data["results"].empty())
    {
        cout << "No matching movie found.\n";
        return;
    }

    const json &results = data["results"];
    cout << "\n====== TMDB MATCHES FOR \"" << title << "\" ======\n";
    cout << "Showing " << results.size() << " matching movies returned by TMDB.\n\n";
    for (size_t i = 0; i < results.size(); ++i)
    {
        const json &movie = results[i];
        string releaseDate = movie.value("release_date", string());
        cout << i + 1 << ". " << movie.value("title", string("Untitled"));
        if (releaseDate.size() >= 4)
        {
            cout << " (" << releaseDate.substr(0, 4) << ")";
        }
        cout << " | ";
        printTMDBRating(movie);
    }
}

void showTMDBTopRatedByStars()
{
    cout << "\n====== TMDB TOP RATED BY STARS ======\n";
    cout << "TMDB ratings out of 10 are rounded to the nearest whole star.\n";
    cout << "Showing up to five movies per group with at least 100 votes.\n";

    for (int stars = 5; stars >= 1; --stars)
    {
        cout << "\n" << stars << " STAR" << (stars == 1 ? "" : "S") << " ("
             << string(static_cast<size_t>(stars), '*') << ")\n";

        int minimumRating = stars * 2 - 1;
        int maximumRating = stars == 5 ? 10 : stars * 2;
        string filters = "sort_by=vote_average.desc&vote_count.gte=100";
        filters += "&vote_average.gte=" + to_string(minimumRating);
        filters += "&vote_average.lte=" + to_string(maximumRating);
        if (stars < 5)
        {
            filters += ".99";
        }

        json data;
        if (!fetchTMDBJson("/discover/movie", string(), data, filters))
        {
            return;
        }
        if (!data.is_object() || !data.contains("results") || !data["results"].is_array())
        {
            cerr << "TMDB response did not contain movie results.\n";
            return;
        }

        const json &results = data["results"];
        size_t displayed = 0;
        for (size_t i = 0; i < results.size() && displayed < 5; ++i)
        {
            const json &movie = results[i];
            double rating = movie.value("vote_average", 0.0);
            if (starsForTMDBRating(rating) != stars)
            {
                continue;
            }

            string releaseDate = movie.value("release_date", string());
            cout << "- " << movie.value("title", string("Untitled"));
            if (releaseDate.size() >= 4)
            {
                cout << " (" << releaseDate.substr(0, 4) << ")";
            }
            cout << " | " << rating << "/10 | "
                 << string(static_cast<size_t>(stars), '*') << "\n";
            ++displayed;
        }
        if (displayed == 0)
        {
            cout << "No matching movies found.\n";
        }
    }
}

void showPersonalizedRecommendations(sqlite3 *db)
{
    sqlite3_stmt *statement = nullptr;
    const char *sql = "SELECT movie_title FROM ratings ORDER BY rating DESC, id DESC LIMIT 1;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &statement, nullptr);
    if (rc != SQLITE_OK)
    {
        cerr << "Unable to read your ratings: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    rc = sqlite3_step(statement);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(statement);
        if (rc == SQLITE_DONE)
        {
            cout << "Rate a watched movie first to get personalized recommendations.\n";
        }
        else
        {
            cerr << "Unable to read your ratings: " << sqlite3_errmsg(db) << "\n";
        }
        return;
    }

    const unsigned char *titleValue = sqlite3_column_text(statement, 0);
    string title = titleValue == nullptr ? string() : reinterpret_cast<const char *>(titleValue);
    sqlite3_finalize(statement);
    if (title.empty())
    {
        cout << "Your highest-rated movie has no title to search.\n";
        return;
    }

    json searchData;
    if (!fetchTMDBJson("/search/movie", title, searchData))
    {
        return;
    }
    if (!searchData.is_object() || !searchData.contains("results") ||
        !searchData["results"].is_array() || searchData["results"].empty())
    {
        cout << "Could not find \"" << title << "\" on TMDB to make recommendations.\n";
        return;
    }

    int movieId = searchData["results"][0].value("id", 0);
    if (movieId <= 0)
    {
        cerr << "TMDB did not return a valid movie ID for \"" << title << "\".\n";
        return;
    }

    json recommendations;
    string endpoint = "/movie/" + to_string(movieId) + "/recommendations";
    if (fetchTMDBJson(endpoint, string(), recommendations))
    {
        cout << "Based on your highest-rated movie: " << title << "\n";
        printMovieResults(recommendations, "PERSONALIZED RECOMMENDATIONS");
    }
}

void searchMovie(sqlite3 *db)
{
    string movie;

    cout << "Search Movie: ";
    getline(cin, movie);

    json data;
    if (!fetchTMDBJson("/search/movie", movie, data))
    {
        return;
    }
    if (!data.is_object() || !data.contains("results") || !data["results"].is_array())
    {
        cerr << "TMDB response did not contain movie results.\n";
        return;
    }

    const json &movies = data["results"];
    cout << "\n===== SEARCH RESULTS =====\n\n";
    for (size_t i = 0; i < 5 && i < movies.size(); ++i)
    {
        string title = movies[i].value("title", string("Untitled"));
        string releaseDate = movies[i].value("release_date", string());

        cout << i + 1 << ". " << title;
        if (releaseDate.size() >= 4)
        {
            cout << " (" << releaseDate.substr(0, 4) << ")";
        }
        cout << " * ";
        printTMDBRating(movies[i]);
    }

    int selection;

    cout << "\nSelect Movie Number (0 to cancel): ";
    cin >> selection;

    if (selection < 1 || selection > 5 || static_cast<size_t>(selection) > movies.size())
    {
        cout << "Cancelled.\n";
        return;
    }

    json selectedMovie = movies[selection - 1];

    cout << "\n================================\n";

    cout << "Title: "
         << selectedMovie["title"]
         << endl;

    cout << "Release Date: "
         << selectedMovie["release_date"]
         << endl;

    cout << "TMDB Rating: ";
    printTMDBRating(selectedMovie);

    cout << "\nOverview:\n"
         << selectedMovie.value("overview", string("No overview available."))
         << endl;

    cout << "\n================================\n";

    string choice;
    do
    {
        cout << "\nAdd to Watchlist? (Y/N): ";
        if (!(cin >> choice))
        {
            cerr << "Unable to read watchlist choice.\n";
            return;
        }
        if (choice != "Y" && choice != "y" && choice != "N" && choice != "n")
        {
            cout << "Please enter Y or N.\n";
        }
    } while (choice != "Y" && choice != "y" && choice != "N" && choice != "n");

    if (choice == "N" || choice == "n")
    {
        cout << "Movie not added.\n";
        return;
    }

    string selectedTitle = selectedMovie.value("title", string());
    if (selectedTitle.empty())
    {
        cerr << "Selected TMDB result has no movie title.\n";
        return;
    }

    sqlite3_stmt *checkStatement = nullptr;
    int rc = sqlite3_prepare_v2(
        db,
        "SELECT "
        "EXISTS(SELECT 1 FROM watchlist WHERE movie_title = ? COLLATE NOCASE), "
        "EXISTS(SELECT 1 FROM history WHERE movie_title = ? COLLATE NOCASE);",
        -1,
        &checkStatement,
        nullptr);
    if (rc != SQLITE_OK)
    {
        cerr << "Error checking whether movie was already saved: " << sqlite3_errmsg(db) << "\n";
        return;
    }
    sqlite3_bind_text(checkStatement, 1, selectedTitle.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(checkStatement, 2, selectedTitle.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(checkStatement);
    if (rc != SQLITE_ROW)
    {
        cerr << "Error checking whether movie was already saved: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(checkStatement);
        return;
    }

    bool alreadyInWatchlist = sqlite3_column_int(checkStatement, 0) != 0;
    bool alreadyWatched = sqlite3_column_int(checkStatement, 1) != 0;
    sqlite3_finalize(checkStatement);
    if (alreadyInWatchlist)
    {
        cout << "\nYou already have this movie in your watchlist.\n";
        return;
    }
    if (alreadyWatched)
    {
        cout << "\nYou already watched this movie.\n";
        return;
    }

    sqlite3_stmt *insertStatement = nullptr;
    rc = sqlite3_prepare_v2(
        db,
        "INSERT INTO watchlist (movie_title) VALUES (?);",
        -1,
        &insertStatement,
        nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(insertStatement, 1, selectedTitle.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(insertStatement);
    }

    if (rc == SQLITE_DONE)
    {
        cout << "\nMovie added to watchlist.\n";
    }
    else
    {
        cerr << "\nError adding movie to watchlist: " << sqlite3_errmsg(db) << "\n";
    }
    sqlite3_finalize(insertStatement);
}
