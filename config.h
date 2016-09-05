#include "gaplessgrid.c"
// Needed for media keys
#include "X11/XF86keysym.h"

#define NUMCOLORS       17
#define ALTKEY          Mod1Mask
#define WINKEY          Mod4Mask
#define TAGKEYS(KEY,TAG) \
    { ALTKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { ALTKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { ALTKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { ALTKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },
#define STACKKEYS(MOD,ACTION) \
    { MOD, XK_q,     ACTION##stack, {.i = 0 } }, \
    { MOD, XK_a,     ACTION##stack, {.i = 1 } }, \
    { MOD, XK_z,     ACTION##stack, {.i = 2 } }, \
    { MOD, XK_x,     ACTION##stack, {.i = -1 } },
#define INCSTACKKEYS(MOD,ACTION) \
    { MOD, XK_grave, ACTION##stack, {.i = PREVSEL } }, \
    { MOD, XK_j,     ACTION##stack, {.i = INC(+1) } }, \
    { MOD, XK_k,     ACTION##stack, {.i = INC(-1) } },

static const unsigned int tagspacing = 2;       /* space between tags */
static const unsigned int tagpadding = 50;      /* inner padding of tags */
static const unsigned int taglinepx = 2;        /* height of tag underline */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const Bool showsystray = True;           /* false means no systray */
static const unsigned int gappx = 8;            /* gaps between windows */
static const unsigned int borderpx = 2;         /* border pixel of windows */
static const unsigned int snap = 32;            /* snap pixel */
static const Bool showbar = True;               /* false means no bar */
static const Bool topbar = True;                /* false means bottom bar */
static const float mfact = 0.50;                /* factor of master area size [0.05..0.95] */
static const int nmaster = 1;                   /* number of clients in master area */
static const Bool resizehints = False;          /* true means respect size hints in tiled resizals */

static const char *fonts[] = {
    "Meslo LG M for Powerline:size=10",
    "FontAwesome:size=10",
    "Siji:size=10",
};

static const char dmenufont[] = "Hasklig-8";
static const char dwmpath[] = "/home/elken/.dwm/dwm";

#ifdef SOLARIZED_DARK
static const char colors[NUMCOLORS][ColLast][13] = {
    /* border    fg         bg */
    { "#286e75", "#286e75", "#002b36" },        /* [0]  01 - Client normal */
    { "#af8700", "#268bd2", "#002b36" },        /* [1]  02 - Client selected */
    { "#286e75", "#dc322f", "#002b36" },        /* [2]  03 - Client urgent */
    { "#286e75", "#286e75", "#002b36" },        /* [3]  04 - Client occupied */
    { "#002b36", "#dc322f", "#002b36" },        /* [4]  05 - Red */
    { "#002b36", "#af8700", "#002b36" },        /* [5]  06 - Yellow */
    { "#002b36", "#859900", "#002b36" },        /* [6]  07 - Green */
    { "#002b36", "#666666", "#002b36" },        /* [7]  08 - Dark grey */
    { "#002b36", "#DCDCDC", "#002b36" },        /* [8]  09 - Light grey */
    { "#286e75", "#286e75", "#002b36" },        /* [9]  0A - Bar normal*/
    { "#268bd2", "#268bd2", "#002b36" },        /* [10] 0B - Bar selected*/
    { "#286e75", "#dc322f", "#002b36" },        /* [11] 0C - Bar urgent*/
    { "#286e75", "#268bd2", "#002b36" },        /* [12] 0D - Bar occupied*/
};
#endif

#ifdef SOLARIZED_LIGHT
static const char colors[NUMCOLORS][ColLast][13] = {
    /* border    fg         bg */
    { "#93a1a1", "#93a1a1", "#fdf6e3" },        /* [0]  01 - Client normal */
    { "#268bd2", "#268bd2", "#fdf6e3" },        /* [1]  02 - Client selected */
    { "#93a1a1", "#dc322f", "#fdf6e3" },        /* [2]  03 - Client urgent */
    { "#93a1a1", "#93a1a1", "#fdf6e3" },        /* [3]  04 - Client occupied */
    { "#fdf6e3", "#dc322f", "#fdf6e3" },        /* [4]  05 - Red */
    { "#fdf6e3", "#af8700", "#fdf6e3" },        /* [5]  06 - Yellow */
    { "#fdf6e3", "#859900", "#fdf6e3" },        /* [6]  07 - Green */
    { "#fdf6e3", "#666666", "#fdf6e3" },        /* [7]  08 - Dark grey */
    { "#fdf6e3", "#DCDCDC", "#fdf6e3" },        /* [8]  09 - Light grey */
    { "#93a1a1", "#93a1a1", "#fdf6e3" },        /* [9]  0A - Bar normal*/
    { "#268bd2", "#268bd2", "#fdf6e3" },        /* [10] 0B - Bar selected*/
    { "#93a1a1", "#dc322f", "#fdf6e3" },        /* [11] 0C - Bar urgent*/
    { "#93a1a1", "#268bd2", "#fdf6e3" },        /* [12] 0D - Bar occupied*/
};
#endif

#ifdef GRUVBOX
static const char colors[NUMCOLORS][ColLast][17] = {
    /* border    fg         bg */
    { "#282828", "#928374", "#282828" },        /* [0]  01 - Client normal */
    { "#ebdbb2", "#458588", "#282828" },        /* [1]  02 - Client selected */
    { "#83a598", "#fb4934", "#282828" },        /* [2]  03 - Client urgent */
    { "#83a598", "#83a598", "#282828" },        /* [3]  04 - Client occupied */
    { "#282828", "#fb4934", "#282828" },        /* [4]  05 - Red */
    { "#282828", "#fabd2f", "#282828" },        /* [5]  06 - Yellow */
    { "#282828", "#b8bb26", "#282828" },        /* [6]  07 - Green */
    { "#282828", "#928374", "#282828" },        /* [7]  08 - Dark grey */
    { "#282828", "#d5c4a1", "#282828" },        /* [8]  09 - Light grey */
    { "#928374", "#928374", "#282828" },        /* [9]  0A - Bar normal*/
    { "#3c3836", "#a89985", "#282828" },        /* [10] 0B - Bar selected*/
    { "#fb4934", "#fb4934", "#282828" },        /* [11] 0C - Bar urgent*/
    { "#928374", "#458588", "#282828" },        /* [12] 0D - Bar occupied*/
    { "#3c3836", "#3c3836", "#282828" },        /* [13] 0E - Tag normal*/
    { "#83a598", "#83a598", "#282828" },        /* [14] 0F - Tag selected*/
    { "#fb4934", "#fb4934", "#282828" },        /* [15] 10 - Tag urgent*/
    { "#3c3836", "#928374", "#282828" },        /* [16] 11 - Tag occupied*/
};
#endif

static const Layout layouts[] = {
    /* symbol   gaps    arrange */
    { "",      True,   tile },
    { "",      True,   NULL },
    { "",      True,   monocle },
    { "",      True,   gaplessgrid },
    { "",      True,   bstack },
};

static const Tag tags[] = {
    /* name     layout          mfact   nmaster */
    { "",    &layouts[0],    -1,     -1 },
    { "",    &layouts[3],    -1,     -1 },
    { "",    &layouts[2],    -1,     -1 },
    { "",    &layouts[2],    -1,     -1 },
    { "",    &layouts[1],    -1,     -1 },
    { "",    &layouts[2],    -1,     -1 },
    { "",    &layouts[2],    -1,     -1 },
};

static const Rule rules[] = {
    /* xprop(1):
        *	WM_CLASS(STRING) = instance, class
        *	WM_NAME(STRING) = title
        */
    /* class            instance    title       tags mask   isfloating  monitor */
    { "Firefox", 	NULL, 	    NULL,           1 << 1,     False,      -1 },
    { "Chromium", 	NULL, 	    NULL,           1 << 1,     False,      -1 },
    { "Icedove",   	NULL,       NULL,           1 << 2,     False,      -1 },
    { "Thunderbird",   	NULL,       NULL,           1 << 2,     False,      -1 },
    { "Steam",		"Steam",    "Steam",        1 << 6,     False,      -1 },
    { "VirtualBox",     NULL,       NULL,           1 << 3,     False,      -1 },
    { "Popcorntime",    NULL,       NULL,           1 << 5,     False,      -1 },
    { "Transmission",   NULL,       NULL,           1 << 3,     False,      -1 },
    { "mpv",            "gl",       NULL,           1 << 5,     False,      -1 },
    { "Slack",          "slack",   NULL,           1 << 2,     False,      -1 },
    { "Vlc",            "vlc",      NULL,           1 << 5,     False,      -1 }
};

#ifdef SOLARIZED_DARK
static const char *menu[] = { "dmenu_run", "-fn", dmenufont, "-nb", "#002b36", "-nf", "#568e75", "-sb", "#002b36", "-sf", "#268bd2", "-h", "22", NULL }; 
#endif

#ifdef SOLARIZED_LIGHT
static const char *menu[] = { "dmenu_run", "-fn", dmenufont, "-nb", "#fdf6e3", "-nf", "#93a1a1", "-sb", "#fdf6e3", "-sf", "#268bd2", "-h", "22", NULL }; 
#endif

#ifdef GRUVBOX
static const char *menu[] = { "dmenu_run", "-fn", dmenufont, "-nb", "#282828", "-nf", "#928374", "-sb", "#3c3836", "-sf", "#a89984", "-h", "22", NULL }; 
#endif

static const char *term[] = { "termite", NULL };
static const char *webb[] = { "google-chrome-stable", NULL };
static const char *mail[] = { "thunderbird", NULL };
static const char *edit[] = { "termite", "-e", "nvim", NULL };
static const char *mdia[] = { "termite", "-e", "ncmpcpp", NULL };
static const char *file[] = { "thunar", NULL };

// Media keys
static const char *vold[] = { "amixer", "-q", "set", "Master", "5%-", "unmute", NULL };
static const char *volu[] = { "amixer", "-q", "set", "Master", "5%+", "unmute", NULL };
static const char *mute[] = { "amixer", "-q", "set", "Master", "toggle", NULL };
static const char *play[] = { "mpc", "toggle", NULL };
static const char *next[] = { "mpc", "next", NULL };
static const char *prev[] = { "mpc", "prev", NULL };
static const char *stop[] = { "mpc", "stop", NULL };
static const char *bklu[] = { "xbacklight", "-steps", "1", "-time", "0", "-inc", "5", NULL };
static const char *bkld[] = { "xbacklight", "-steps", "1", "-time", "0", "-dec", "5", NULL };
static const char *prts[] = { "xfce4-screenshooter", NULL};

static Key keys[] = {
    { ALTKEY,           XK_p,                       spawn,          {.v = menu } },
    { WINKEY,           XK_w,                       runorraise,     {.v = webb } },
    { WINKEY,           XK_Return,                  spawn,          {.v = term } },
    { WINKEY,           XK_e,                       spawn,          {.v = edit } },
    { WINKEY,           XK_m,                       runorraise,     {.v = mail } },
    { WINKEY,           XK_n,                       runorraise,     {.v = mdia } },
    { WINKEY,           XK_f,                       spawn,          {.v = file } },
    {0,                 XF86XK_AudioLowerVolume,    spawn,          {.v = vold } },
    {0,                 XF86XK_AudioRaiseVolume,    spawn,          {.v = volu } },
    {0,                 XF86XK_AudioMute,           spawn,          {.v = mute } },
    {0,                 XF86XK_AudioPlay,           spawn,          {.v = play } },
    {0,                 XF86XK_AudioNext,           spawn,          {.v = next } },
    {0,                 XF86XK_AudioPrev,           spawn,          {.v = prev } },
    {0,                 XF86XK_AudioStop,           spawn,          {.v = stop } },
    {0,                 XF86XK_MonBrightnessUp,     spawn,          {.v = bklu } },
    {0,                 XF86XK_MonBrightnessDown,   spawn,          {.v = bkld } },
    {0,                 XK_Print,                   spawn,          {.v = prts } },
    { ALTKEY|ShiftMask, XK_F12,                     quit,           {0} },
    { ALTKEY|ShiftMask, XK_BackSpace,               self_restart,   {0} },
    { ALTKEY|ShiftMask, XK_b,                       togglebar,      {0} },
    { ALTKEY,           XK_BackSpace,               killclient,     {0} },
    { ALTKEY|ShiftMask, XK_Return,                  zoom,           {0} },
    { ALTKEY,           XK_Tab,                     view,           {0} },
    { ALTKEY|ShiftMask, XK_f,                       togglefloating, {0} },
    { ALTKEY,           XK_space,                   setlayout,      {0} },
    { ALTKEY,           XK_t,                       setlayout,      {.v = &layouts[0] } },
    { ALTKEY,           XK_f,                       setlayout,      {.v = &layouts[1] } },
    { ALTKEY,           XK_m,                       setlayout,      {.v = &layouts[2] } },
    { ALTKEY,           XK_g,                       setlayout,      {.v = &layouts[3] } },
    { ALTKEY,           XK_b,                       setlayout,      {.v = &layouts[4] } },
    { ALTKEY|ShiftMask, XK_h,                       setmfact,       {.f = -0.05 } },
    { ALTKEY|ShiftMask, XK_l,                       setmfact,       {.f = +0.05 } },
    { ALTKEY,           XK_equal,                   incnmaster,     {.i = +1 } },
    { ALTKEY,           XK_minus,                   incnmaster,     {.i = -1 } },
    { ALTKEY,           XK_0,                       view,           {.ui = ~0 } },
    { ALTKEY|ShiftMask, XK_0,                       tag,            {.ui = ~0 } },
    { ALTKEY,           XK_comma,                   focusmon,       {.i = -1 } },
    { ALTKEY,           XK_period,                  focusmon,       {.i = +1 } },
    { ALTKEY|ShiftMask, XK_comma,                   tagmon,         {.i = -1 } },
    { ALTKEY|ShiftMask, XK_period,                  tagmon,         {.i = +1 } },
    STACKKEYS(WINKEY,                               focus)
    STACKKEYS(WINKEY|ShiftMask,                     push)
    INCSTACKKEYS(ALTKEY,                            focus)
    INCSTACKKEYS(ALTKEY|ShiftMask,                  push)
    TAGKEYS(            XK_1,       0)
    TAGKEYS(            XK_2,       1)
    TAGKEYS(            XK_3,       2)
    TAGKEYS(            XK_4,       3)
    TAGKEYS(            XK_5,       4)
    TAGKEYS(            XK_6,       5)
    TAGKEYS(            XK_7,       6)
};

static Button buttons[] = {
    { ClkLtSymbol,      0,          Button1,        setlayout,      {0} },
    { ClkClientWin,     ALTKEY,     Button1,        movemouse,      {0} },
    { ClkClientWin,     ALTKEY,     Button2,        togglefloating, {0} },
    { ClkClientWin,     ALTKEY,     Button3,        resizemouse,    {0} },
    { ClkTagBar,        0,          Button1,        view,           {0} },
    { ClkTagBar,        0,          Button3,        toggleview,     {0} },
    { ClkTagBar,        ALTKEY,     Button1,        tag,            {0} },
    { ClkTagBar,        ALTKEY,     Button3,        toggletag,      {0} },
};
