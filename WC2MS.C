/**
 ** WC2MS.C - contains main function
 ** Written by Michael C. Gallagher <mcgallag@gmail.com>
 ** 12/20/2018
 **
 ** Distributed under MIT License. See LICENSE for details
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "WC2MS.H"

ProgramState state;
char result[80];

int main(int argc, char **argv)
{
    bool running = TRUE;
    char c = 0;
    MainProgram m = SetupCurses();

    while (state != QUIT) {
        refresh();
        clear();
        attron(COLOR_PAIR(1));
        DrawBackground();
        Display(m);
        attroff(COLOR_PAIR(1));
        RetrieveInput(&m);
        ProcessInput(&m);
    }

    DisposeCurses();
}

void DrawBackground(void) {
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            if (x == HEADER_MARGIN && ((y == 1) || (y == 2)))
                attron(A_REVERSE);
            else if (x == (COLS - HEADER_MARGIN) && ((y == 1) || (y == 2)))
                attroff(A_REVERSE);
            mvaddch(y, x, ' ');
        }
    }
}

MainProgram SetupCurses(void) {
    MainProgram mp;
    mp.wnd = initscr();         // initialize the window
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    cbreak();                    // set no waiting for enter key
    noecho();                    // no echoing input
    curs_set(0);                 // hide the cursor
    keypad(stdscr, TRUE);        // set input to get special keys
    keypad(mp.wnd, TRUE);        // set input to get special keys
    getmaxyx(mp.wnd, mp.max_rows, mp.max_cols); // find size of window
    clear();                     // clear screen, set cursor to 0,0
    refresh();                   // implement changes since last refresh

    mp.game_selection = -1;
    mp.system_selection = -1;
    mp.mission_selection = -1;
    mp.lastinput = 0;
    mp.cursor = 0;

    state = GAME;

    return mp;
}

void DisposeCurses() {
    clear();
    refresh();
    endwin();
    if (strlen(result))
        printf(result);
}

void Display(MainProgram mp) {
    PrintHeader(mp);
    DisplayMenu(mp);
    if (state == COMPLETE)
        DisplayComplete(mp);
}

void DisplayComplete(MainProgram mp) {
    // move(COMPLETE_Y_OFFSET, 0);
    attron(A_REVERSE);
    // color the background
    for (int y = 0; y < COMPLETE_HEIGHT; y++) {
        move(COMPLETE_Y_OFFSET + y, MENU_INDENT);
        for (int x = MENU_INDENT; x < COLS - MENU_INDENT; x++) {
            addch(' ');
        }
    }
    //GenerateDOSCommand(mp);

    move(COMPLETE_Y_OFFSET+1, MENU_INDENT+1);
    printw(mp.batch_output);

    static char* prompt = "Press Enter to write %s and finish!";
    move(COMPLETE_Y_OFFSET+3, (int)(COLS - (strlen(prompt) + strlen(BATCH_FILE_NAME) - 2)) / 2);
    attron(A_BLINK);
    printw(prompt, BATCH_FILE_NAME);
    attroff(A_REVERSE | A_BLINK);
    // printw("Complete! Yay!");
}

void GenerateDOSCommand(MainProgram *mp) {
    switch (mp->game_selection) {
        case WC2:
            sprintf(mp->batch_output, "loadfix -32 play-wc2 %d %c", mp->system_selection+1, mp->mission_selection+65);
            break;
        case SO1:
            sprintf(mp->batch_output, "loadfix -32 play-so1 %d %c", mp->system_selection+1, mp->mission_selection+65);
            break;
        case SO2:
            sprintf(mp->batch_output, "loadfix -32 play-so2 %d %c", mp->system_selection+1, mp->mission_selection+65);
            break;
    }
    return;
}

void RetrieveInput(MainProgram *mp) {
    // nothing
    char c = getch();
    mp->lastinput = c;
}

void ProcessInput(MainProgram *mp) {
    if (mp->lastinput == KEY_ESCAPE) {
        state = QUIT;
        return;
    }
    if (mp->lastinput == UP_ARROW || mp->lastinput == KEY_UP) {
        int max;
        switch(state) {
            case GAME:
                max = TOTAL_GAMES;
                break;
            case SYSTEM:
                max = SystemsPerGame[mp->game_selection];
                break;
            case MISSION:
                max = MissionsPerSystem[mp->game_selection][mp->system_selection];
                break;
            default:
                return;
                break;
        }
        mp->cursor = (mp->cursor <= 0) ? (max-1) : mp->cursor - 1;
        return;
    } else if (mp->lastinput == DOWN_ARROW || mp->lastinput == KEY_DOWN) {
        int max = 0;
        switch(state) {
            case GAME:
                max = TOTAL_GAMES;
                break;
            case SYSTEM:
                max = SystemsPerGame[mp->game_selection];
                break;
            case MISSION:
                max = MissionsPerSystem[mp->game_selection][mp->system_selection];
                break;
            default:
                return;
                break;
        }
        max--;
        mp->cursor = (mp->cursor >= max) ? 0 : mp->cursor + 1;
        return;
    } else if (mp->lastinput == LEFT_ARROW || mp->lastinput == KEY_LEFT) {
        StateUp(mp);
        // mp->cursor = 0;
        return;
    } else if (mp->lastinput == RIGHT_ARROW || mp->lastinput == ENTER_KEY || mp->lastinput == KEY_RIGHT || mp->lastinput == KEY_ENTER) {
        if (state == COMPLETE && (mp->lastinput == ENTER_KEY || mp->lastinput == KEY_ENTER)) {
            WriteBatchFile(mp);
        }
        if (state == SYSTEM && mp->cursor == 2) {
            MakeSelection(mp);
        } else {
            MakeSelection(mp);
        }
        StateDown(mp);
        return;
    }
    int i = ConvertInput(mp->lastinput);
    mp->cursor = i-1;
    if (i >= 0) {
        i--;
        switch(state) {
            case GAME:
                mp->game_selection = i;
                break;
            case SYSTEM:
                mp->system_selection = i;
                break;
            case MISSION:
                mp->mission_selection = i;
                break;
        };
        StateDown(mp);
    } else if (i == -1) {
        StateUp(mp);
    }
}

void WriteBatchFile(MainProgram *mp) {
    FILE *fp = fopen(BATCH_FILE_NAME, "wt");
    if (fp) {
        int i = fprintf(fp, mp->batch_output);
        if (i > 0) {
            state = QUIT;
            char *short_cmd;
            short_cmd = strtok(BATCH_FILE_NAME, ".");
            sprintf(result, "Batch file written to %s\nType %s to play...\n", BATCH_FILE_NAME, short_cmd);
        }
        fflush(fp);
        fclose(fp);
    }
}

void MakeSelection(MainProgram *mp) {
    switch (state) {
        case GAME:
            mp->game_selection = mp->cursor;
            break;
        case SYSTEM:
            mp->system_selection = mp->cursor;
            break;
        case MISSION:
            mp->mission_selection = mp->cursor;
            GenerateDOSCommand(mp);
            break;
        case COMPLETE:
            return;
            break;
    }
}

void StateDown(MainProgram *mp) {
    switch (state) {
        case GAME:
            state = SYSTEM;
            mp->cursor = 0;
            break;
        case SYSTEM:
            state = MISSION;
            mp->cursor = 0;
            break;
        case MISSION:
            state = COMPLETE;
            break;
        case COMPLETE:
            return;
            break;
        default:
            state = QUIT;
            break;
    }
}

void StateUp(MainProgram *mp) {
    switch (state) {
        case SYSTEM:
            state = GAME;
            mp->cursor = (mp->game_selection == -1) ? 0 : mp->game_selection;
            break;
        case MISSION:
            state = SYSTEM;
            mp->cursor = (mp->system_selection == -1) ? 0 : mp->system_selection;
            break;
        case COMPLETE:
            mp->cursor = (mp->mission_selection == -1) ? 0 : mp->mission_selection;
            state = MISSION;
            break;
        default:
            state = GAME;
            mp->cursor = 0;
            break;
    }
}

void PrintHeader(MainProgram mp) {
    char stateText[COLS];
    char prompt[COLS];
    switch (state) {
        case GAME:
            sprintf(stateText, "Welcome to Trelane's WC2 Launcher");
            sprintf(prompt, "Select a Game:");
            break;
        case SYSTEM:
            sprintf(stateText, "%s", GameTitles[mp.game_selection]);
            sprintf(prompt, "Select a System:");
            break;
        case MISSION:
            sprintf(stateText, "%s, %s", GameTitles[mp.game_selection], WC2Systems[mp.game_selection][mp.system_selection]);
            sprintf(prompt, "Select a Mission:");
            break;
        case COMPLETE:
        {
            char missionDesc[COLS];
            PrintMission(mp, mp.mission_selection, missionDesc);
            sprintf(stateText, "%s", missionDesc);
            sprintf(prompt, "Press Enter to Go!");
            break;
        }
    }
    attron(A_REVERSE);
    int x = (COLS - strlen(stateText)) / 2;
    mvprintw(1, x, stateText);
    x = (COLS - strlen(prompt)) / 2;
    mvprintw(2, x, prompt);
    attroff(A_REVERSE);
}

void DisplayMenu(MainProgram mp) {
    switch (state) {
        case GAME:
            for (int i = 0; i < TOTAL_GAMES; i++) {
                if (i == mp.cursor) attron(A_REVERSE);
                mvprintw(i+MENU_OFFSET, MENU_INDENT, "%X. %s", i+1, GameTitles[i]);
                if (i == mp.cursor) attroff(A_REVERSE);
            }
            break;
        case SYSTEM:
            for (int i = 0; i < SystemsPerGame[mp.game_selection]; i++) {
                if (i == mp.cursor) attron(A_REVERSE);
                mvprintw(i+MENU_OFFSET, MENU_INDENT, "%X. %s", i+1, WC2Systems[mp.game_selection][i]);
                if (i == mp.cursor) attroff(A_REVERSE);
            }
            break;
        case MISSION:
        case COMPLETE:
            for (int i = 0; i < MissionsPerSystem[mp.game_selection][mp.system_selection]; i++) {
                if (i == mp.cursor) attron(A_REVERSE);
                //PrintMissionType(mp, i);
                char line[COLS-MENU_OFFSET];
                char missionDesc[COLS];
                PrintMission(mp, i, missionDesc);
                sprintf(line, "%d. %s", i+1, missionDesc);
                mvprintw(i+MENU_OFFSET, MENU_INDENT, line);
                if (i == mp.cursor) attroff(A_REVERSE);
            }
            break;
    }
    DisplayPrompt(mp);
}

void DisplayPrompt(MainProgram mp) {
    int y, x;
    getyx(mp.wnd, y, x);
    char prompt[COLS];
    switch (state) {
        case GAME:
            strcpy(prompt, "Select a Game. ESC exits to DOS.");
            break;
        case SYSTEM:
            sprintf(prompt, "Select a %s system. Q - Prev Menu. Esc - Quit", GameAbbrevs[mp.game_selection]);
            break;
        case MISSION:
            sprintf(prompt, "Select mission in %s. Q - Prev Menu. Esc - Quit", WC2Systems[mp.game_selection][mp.system_selection]);
            break;
        case COMPLETE:
            sprintf(prompt, "Confirm mission in %s. Q - Prev Menu. Esc - Quit", WC2Systems[mp.game_selection][mp.system_selection]);
            break;
        default:
            sprintf(prompt, "Not implemented yet.");
            break;
    }
    mvprintw(y+2, MENU_INDENT, prompt);
}

void PrintMission(MainProgram mp, int i, char *buf) {
    switch (mp.game_selection) {
        case WC2:
            sprintf(buf, "%s %c - %s", WC2Systems[mp.game_selection][mp.system_selection], i+65, WC2MissionTypes[mp.system_selection][i]);
            break;
        case SO1:
            sprintf(buf, "%s %c - %s", WC2Systems[mp.game_selection][mp.system_selection], i+65, SO1MissionTypes[mp.system_selection][i]);
            break;
        case SO2:
            sprintf(buf, "%s %c - %s", WC2Systems[mp.game_selection][mp.system_selection], i+65, SO2MissionTypes[mp.system_selection][i]);
            break;
    }
}

int ConvertInput(char c) {
    int i;
    if (isdigit(c)) {
        i = c - '0';
    } else {
        switch (c) {
            case 'A':
            case 'a':
                i = 10;
                break;
            case 'B':
            case 'b':
                i = 11;
                break;
            case 'C':
            case 'c':
                i = 12;
                break;
            case 'Q':
            case 'q':
                i = -1;
                break;
            default:
                i = -2;
                break;
        }
    }
    return i;
}
