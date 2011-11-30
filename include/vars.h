/*
-----------------------------------------------------------
This is the embedded URL precache uses as a remote download source.
A "precache.list" should be located at location specified.
*/
#define PRECACHE_URL "http://localhost/project/"

/*
-----------------------------------------------------------
This is the window title for the launcher application
*/
#define PRECACHE_WINDOW_TITLE "Precache Download Test"

/*
-----------------------------------------------------------
Colors used for rendering (RGBA floats)
*/
#define PRECACHE_WINDOW_BACKGROUND_COLOR	{ 0.25, 0.25, 0.25, 1.0 }

// button background color
#define PRECACHE_BUTTON_COLOR				{ 0, .75, .75, 1 }

// button color on mouse over
#define PRECACHE_BUTTON_HOVER_COLOR			{ 0, .5, .5, 1 }

// button text color
#define PRECACHE_BUTTON_TEXT_COLOR			{ 1.0, 1.0, 1.0, 1.0 }

// progress bar colors
#define PRECACHE_PROGRESSBAR_OUTLINE_COLOR	{ 0.0, 0.0, 0.0, 1.0 }
#define PRECACHE_PROGRESSBAR_EMPTY_COLOR	{ 0.07, 0.5, 0.07, 1.0 }
#define PRECACHE_PROGRESSBAR_COMPLETE_COLOR	{ 0.0, 1.0, 0.0, 1.0 }