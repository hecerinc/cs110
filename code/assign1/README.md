NOTE that even though the file assignment doesn't specify this for the movie record,

>  If the number of bytes dedicated to the actor’s name (always even) and the short (always 2) isn’t a multiple of four, then two additional '\0''s appear after the two bytes storing the number of movies. This padding is conditionally done so that the four-byte integers that follow sit at addresses that are multiples of four (again, because the 64-bit myth's might be configured to require this).

This part that's written under the **Actor record** also applies to the movie file (except that it's the number of bytes allocated for title,year, and # of actors).
