# MovieHub-CLI-Based-System-Renew
Renewed MovieHub CLI Based System

## TMDB setup

Store your TMDB API key in `config/tmdb_api_key.txt` (one line) and run the
program from the project directory. Keep this key private and do not commit it.

The Ratings menu keeps your personal 1-5 ratings in the local database and can
look up all matching TMDB community ratings. TMDB top-rated movies can also be
grouped into whole-star ratings, rounded from TMDB's 0-10 vote average.
Already-rated watched movies are excluded from the normal rating list; use
Change Existing Rating to overwrite a saved personal rating.
Recommendations include trending, popular, top-rated, and suggestions based on
your highest-rated movie. Submitting a personal rating to TMDB requires
account/session authentication and is not configured by an API key alone.

Add movies to the watchlist through Search Movie. The Watchlist menu lets you
view or remove saved movies; History lets you select a watchlist movie to mark
as watched and review watched movies with their dates. Movies already in the
watchlist or history cannot be added again.
