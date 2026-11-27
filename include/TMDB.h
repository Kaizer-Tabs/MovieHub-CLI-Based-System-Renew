#ifndef TMDB_H
#define TMDB_H

extern "C"
{
#include "../sqlite/sqlite3.h"
}

void searchMovie(sqlite3 *db);

#include <string>

std::string readApiKey();

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output);

void searchMovie(sqlite3 *db);

#endif // TMDB_H