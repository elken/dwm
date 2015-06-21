#include "gaplessgrid.c"
// Needed for media keys
#include "X11/XF86keysym.h"

#define NUMCOLORS       13
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

static const char dmenufont[] = "Meslo LG M for Powerline-8";
static const char dwmpath[] = "/home/elken/build/dwm/dwm";

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

static const Layout layouts[] = {
    /* symbol   gaps    arrange */
    { "",      True,   tile },
    { "",      True,   NULL },
    { "",      False,  monocle },
    { "",      False,  gaplessgrid },
    { "",      True,   bstack },
};

static const Tag tags[] = {
    /* name     layout          mfact   nmaster */
    { "",    &layouts[0],    -1,     -1 },
    { "",    &layouts[3],    -1,     -1 },
    { "",    &layouts[2],    -1,     -1 },
    { "",    &layouts[0],    -1,     -1 },
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
    { "Iceweasel", 	NULL, 	    NULL,       1 << 1,     True,       -1 },
    { "Firefox", 	NULL, 	    NULL,       1 << 1,     True,       -1 },
    { "Icedove",   	NULL,       NULL,       1 << 2,     False,      -1 },
    { "Thunderbird",   	NULL,       NULL,       1 << 2,     False,      -1 },
    { "Google-chrome",  NULL,       NULL,       1 << 5,     True,       -1 },
    { "Steam",		"Steam",    "Steam",    1 << 6,     False,      -1 },
    { "VirtualBox",     NULL,       NULL,       1 << 3,     False,      -1 },
    { "Popcorntime",    NULL,       NULL,       1 << 5,     True,       -1 },
    { "Transmission",   NULL,       NULL,       1 << 3,     False,      -1 }
};

//static const char *menu[] = { "dmenu_run", "-fn", font, "-nb", "#002b36", "-nf", "#568e75", "-sb", "#002b36", "-sf", "#268bd2", "-h", "22", NULL }; 
static const char *menu[] = { "dmenu_run", "-fn", dmenufont, "-nb", "#002b36", "-nf", "#568e75", "-sb", "#002b36", "-sf", "#268bd2", "-h", "22", NULL }; 
static const char *term[] = { "st", NULL };
static const char *webb[] = { "iceweasel", NULL };
static const char *mail[] = { "icedove", NULL };
static const char *edit[] = { "emacsclient", "-c", NULL };
static const char *mdia[] = { "st", "-e", "ncmpcpp", NULL };
static const char *file[] = { "thunar", NULL };
//static const char *htop[] = { "st", "-e", "htop", NULL};

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
