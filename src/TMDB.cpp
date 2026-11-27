#include "../include/TMDB.h"
#include "../include/json.hpp"

#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using json = nlohmann::json;

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

void searchMovie(sqlite3 *db)
{
    vector<string> movieTitles;
    movieTitles.clear();
    string movie;

    cout << "Search Movie: ";
    getline(cin, movie);

    string apiKey = readApiKey();

    string response;

    string url =
        "https://api.themoviedb.org/3/search/movie?api_key=" +
        apiKey +
        "&query=" +
        movie;

    CURL *curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode result = curl_easy_perform(curl);

        if (result == CURLE_OK)
        {
            json data = json::parse(response);

            cout << "\n===== SEARCH RESULTS =====\n\n";

            for (int i = 0;
                 i < 5 && i < data["results"].size();
                 i++)
            {
                string title =
                    data["results"][i]["title"];
                movieTitles.push_back(title);

                string releaseDate =
                    data["results"][i]["release_date"];

                string year =
                    releaseDate.substr(0, 4);

                cout
                    << i + 1
                    << ". "
                    << title
                    << " ("
                    << year
                    << ")"
                    << endl;
            }
        }
    }
    int selection;
    cout << "\nSelect Movie Number (0 to cancel): ";
    cin >> selection;

    if (selection < 1 || selection > movieTitles.size())
    {
        cout << "Cancelled.\n";
        return;
    }

    string sql = "INSERT INTO watchlist (movie_title) VALUES ('" + movieTitles[selection - 1] + "');";
    char *errMsg = nullptr;

    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc == SQLITE_OK)
    {
        cout << "\nMovie added to watchlist.\n";
    }
    else
    {
        cout << "Error adding movie: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}
