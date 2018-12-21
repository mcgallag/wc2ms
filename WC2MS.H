/**
 ** WC2MS.H - utility functions
 ** Written by Michael C. Gallagher <mcgallag@gmail.com>
 ** 12/20/2018
 **
 ** Distributed under MIT License. See LICENSE for details
 **/

#include <curses.h>
#include "curses.h"

#define DOWN_ARROW 2
#define UP_ARROW 3
#define LEFT_ARROW 4
#define RIGHT_ARROW 5
#define ENTER_KEY '\n'

#define MENU_OFFSET 4
#define MENU_INDENT 3

#define HEADER_MARGIN 10

#define COMPLETE_Y_OFFSET 12
#define COMPLETE_HEIGHT 5

#define NUM_WC2_SYSTEMS 12
#define NUM_SO1_SYSTEMS 4
#define NUM_SO2_SYSTEMS 4

#define MAX_STR_LENGTH 24

#define KEY_ESCAPE 27

#define WC2 0
#define SO1 1
#define SO2 2

#define TOTAL_GAMES 3

#define PATROL "Patrol"
#define STRIKE "Strike"
#define ESCORT "Escort"
#define RENDEZVOUS "Rendezvous"
#define DEFEND "Defend"

#define BATCH_FILE_NAME "GO.BAT"

const int SystemsPerGame[TOTAL_GAMES] = {
    NUM_WC2_SYSTEMS, NUM_SO1_SYSTEMS, NUM_SO2_SYSTEMS
};

typedef enum program_state {
    QUIT, GAME, SYSTEM, MISSION, COMPLETE
} ProgramState;

typedef struct main_prog {
    WINDOW *wnd;
    int max_rows;
    int max_cols;
    int game_selection;
    int system_selection;
    int mission_selection;
    char lastinput;
    int cursor;
    char batch_output[80];
} MainProgram;

char GameTitles[TOTAL_GAMES][32] = {
    "Wing Commander 2",
    "Secret Operations 1",
    "Secret Operations 2"
};

char GameAbbrevs[TOTAL_GAMES][4] = {
    "WC2", "SO1", "SO2"
};

char WC2Systems[TOTAL_GAMES][NUM_WC2_SYSTEMS][MAX_STR_LENGTH] = {
    {
        "Gwynedd System",
        "Niven System",
        "Ghorah Khar",
        "Novaya Kiev",
        "Heaven's Gate",
        "Tesla System",
        "Enigma System",
        "K'tithrak Mang",
        "Ghorah Khar (Alt)",
        "Novaya Kiev (Alt)",
        "Tesla (Alt)",
        "Gwynedd (Alt)"        
    },
    {
        "Pembroke System",
        "Rigel System",
        "Ghorah Khar (Part 1)",
        "Ghorah Khar (Part 2)"
    },
    {
        "Canewdon (Part 1)",
        "Canewdon (Part 2)",
        "Canewdon (Part 3)",
        "Ayers Rock"
    }
};

int MissionsPerSystem[TOTAL_GAMES][NUM_WC2_SYSTEMS] = {
    { 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4 },
    { 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0 }
};

char WC2MissionTypes[NUM_WC2_SYSTEMS][4][12] = {
    { PATROL, DEFEND, ESCORT, PATROL }, // Gwynedd
    { PATROL, PATROL, ESCORT, PATROL }, // Niven
    { PATROL, ESCORT, ESCORT, DEFEND }, // Ghorah Khar
    { STRIKE, RENDEZVOUS, STRIKE, RENDEZVOUS }, // Novaya Kiev
    { PATROL, ESCORT, ESCORT, STRIKE }, // Heaven's Gate
    { PATROL, STRIKE, ESCORT, ESCORT }, // Tesla
    { STRIKE, STRIKE, STRIKE, "" }, // Enigma
    { STRIKE, PATROL, STRIKE, STRIKE }, // K'tithrak Mang
    { PATROL, PATROL, PATROL, PATROL }, // Ghorah Khar (alt)
    { ESCORT, PATROL, PATROL, RENDEZVOUS }, // Novaya Kiev (alt)
    { ESCORT, STRIKE, DEFEND, ESCORT }, // Tesla (alt)
    { STRIKE, PATROL, STRIKE, STRIKE } // Gwynedd (alt)
};

char SO1MissionTypes[NUM_SO1_SYSTEMS][5][12] = {
    { PATROL, PATROL, DEFEND, PATROL, DEFEND }, // Pembroke
    { PATROL, PATROL, PATROL, PATROL, STRIKE }, // Rigel
    { PATROL, RENDEZVOUS, PATROL, PATROL, STRIKE }, // Ghorah Khar 1
    { STRIKE, STRIKE, PATROL, STRIKE, STRIKE } // Ghorah Khar 2
};

char SO2MissionTypes[NUM_SO2_SYSTEMS][5][12] = {
    { STRIKE, PATROL, STRIKE, ESCORT, PATROL }, // Canewdon 1
    { PATROL, STRIKE, PATROL, STRIKE, PATROL }, // Canewdon 2
    { PATROL, PATROL, RENDEZVOUS, STRIKE, PATROL }, // Canewdon 3
    { STRIKE, STRIKE, STRIKE, STRIKE, PATROL } // Ayers Rock
};

MainProgram SetupCurses(void);
void Display(MainProgram mp);
void RetrieveInput(MainProgram *mp);
void ProcessInput(MainProgram *mp);
void DisposeCurses(void);
void DisplayMenu(MainProgram mp);
int ConvertInput(char c);
void StateDown(MainProgram *mp);
void StateUp(MainProgram *mp);
void PrintMissionType(MainProgram mp, int i);
void PrintHeader(MainProgram mp);
void MakeSelection(MainProgram *mp);
void DisplayPrompt(MainProgram mp);
void DrawBackground(void);
void DisplayComplete(MainProgram mp);
void PrintMission(MainProgram mp, int i, char *buf);
void GenerateDOSCommand(MainProgram *mp);
void WriteBatchFile(MainProgram *mp);