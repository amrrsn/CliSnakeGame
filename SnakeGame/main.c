#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>
#include <string.h>

#define UP 259
#define DOWN 258
#define LEFT 260
#define RIGHT 261
#define ENTER 10
#define BACKSPACE 8

#define DEFAULTSCREEN 4
#define HIGHLIGHTSELECTION 8
#define SNAKECOLOR 16
#define FRUITCOLOR 32

#define PLAY 0
#define CONTROLS 1
#define HTP 2
#define HIGHSCORES 3
#define EXIT 4

#define BOOLSOUND false
#define HTPSOUND "C:\\Users\\test\\Documents\\FreshmanCollege\\SpringSemester\\COS130\\FinalProject\\GameAudio\\htpSound.wav"
#define MMSOUND "C:\\Users\\test\\Documents\\FreshmanCollege\\SpringSemester\\COS130\\FinalProject\\GameAudio\\mainMenuSound.wav"
#define PLAYSOUND "C:\\Users\\test\\Documents\\FreshmanCollege\\SpringSemester\\COS130\\FinalProject\\GameAudio\\playSound.wav"

struct coordinate
{
    signed int x;
    signed int y;
};

struct snake
{
    signed int length;
    signed int score;
    struct coordinate body[3248];
    signed int currentHeadDir;
};

struct file
{
    char userName[5];
    signed int userScore;
};

bool exitCase = false;


void initCurses(void);
void initPlayer(struct snake *playerSnake, int maxx, int maxy);

void mainMenu(struct snake *playerSnake, int maxx, int maxy);
void controlsScreen(int midx, int midy);
void htpScreen(int midx, int midy);

void gameScreen(int maxx, int maxy, struct snake *playerSnake);
bool borderHandling(int topBarrier, int rightBarrier, int bottomBarrier, int leftBarrier, struct snake *playerSnake, bool initSnake, bool wasPaused);
void snakeFoodCheck(int foodPos[], struct snake *playerSnake, int maxx, int maxy);
void snakePositionControl(struct snake *playerSnake, struct coordinate body[], clock_t *lastUpdateTime);
void snakeUpdater(struct snake *playerSnake, bool *initSnake, clock_t *lastUpdateTime, bool *wasPaused);
bool delayUpdate(clock_t *lastUpdateTime);
bool pauseScreen();

void deathScreen(struct snake *playerSnake);
void highScoresScreen();

int main()
{
    // Initialize
    initCurses();

    int maxy = getmaxy(stdscr);
    int maxx = getmaxx(stdscr);

    struct snake playerSnake;
    struct snake *playerPointer;
    playerPointer = &playerSnake;

    srand(time(NULL));


    while(!exitCase)
        mainMenu(playerPointer, maxx, maxy);

    endwin();   // exit curses

    return 0;
}

void initPlayer(struct snake *playerSnake, int maxx, int maxy)
{
    playerSnake->length = 5;
    playerSnake->score = 0;
    playerSnake->body[0].x = maxx / 2;
    playerSnake->body[0].y = maxy / 2;
    playerSnake->currentHeadDir = RIGHT;

    for(int i = 1; i < playerSnake->length; i++)
    {
        playerSnake->body[i].x = playerSnake->body[i - 1].x - 1;
        playerSnake->body[i].y = playerSnake->body[i - 1].y;
    }
}


void gameScreen(int maxx, int maxy, struct snake *playerSnake)
{
    clear();
    timeout(0);
    refresh();

    clock_t lastUpdateTime = clock();
    clock_t *luPtr;
    luPtr = &lastUpdateTime;

    if(BOOLSOUND)
        PlaySound(PLAYSOUND, NULL, SND_ASYNC|SND_LOOP);

    int topBarrier = 0;
    int rightBarrier = maxx - 1;
    int bottomBarrier = maxy - 1;
    int leftBarrier = 0;

    int foodPos[2] = {((rand() % (maxx - 4)) + 2), ((rand() % (maxy - 2)) + 1)};    //{x, y}

    bool isDead = false;
    bool initSnake = true;
    bool *iSP = &initSnake;
    bool wasPaused = false;
    bool *wPP = &wasPaused;

    int lastKeyPressTime = 0;
    const int k = 10;

    while(!isDead)
    {
        switch(getch())
        {
        case 112:
            wasPaused = pauseScreen();
            break;
        case UP:
            if(playerSnake->currentHeadDir != DOWN && (clock() - lastKeyPressTime) >= CLOCKS_PER_SEC/k)
            {
                playerSnake->currentHeadDir = UP;
                lastKeyPressTime = clock();
            }
            break;
        case DOWN:
            if(playerSnake->currentHeadDir != UP && (clock() - lastKeyPressTime) >= CLOCKS_PER_SEC/k)
            {
                playerSnake->currentHeadDir = DOWN;
                lastKeyPressTime = clock();
            }
            break;
        case LEFT:
            if(playerSnake->currentHeadDir != RIGHT && (clock() - lastKeyPressTime) >= CLOCKS_PER_SEC/k)
            {
                playerSnake->currentHeadDir = LEFT;
                lastKeyPressTime = clock();
            }
            break;
        case RIGHT:
            if(playerSnake->currentHeadDir != LEFT && (clock() - lastKeyPressTime) >= CLOCKS_PER_SEC/k)
            {
                playerSnake->currentHeadDir = RIGHT;
                lastKeyPressTime = clock();
            }
            break;
        }

        isDead = borderHandling(topBarrier, rightBarrier, bottomBarrier, leftBarrier, playerSnake, initSnake, wasPaused);
        snakeFoodCheck(foodPos, playerSnake, maxx, maxy);
        snakeUpdater(playerSnake, iSP, luPtr, wPP);
    }
}

void deathScreen(struct snake *playerSnake)
{
    clear();
    refresh();
    timeout(-1);

    int midx = getmaxx(stdscr) / 2;
    int midy = getmaxy(stdscr) / 2;

    struct file *highScoresData;
    highScoresData = (struct file *)malloc(sizeof(struct file));

    char playerName[200] = " ";
    bool scanning = true;
    int namePos = 0;
    char getVal;

    mvprintw(midy - 5, midx - 13, "You died with a score of %d", playerSnake->score);
    mvprintw(midy - 3, midx - 18, "Enter your name to save your score: ");

    timeout(0);
    while(scanning)
    {
        getVal = getch();
        if(getVal != -1)
        {
            if(getVal != ENTER && getVal != BACKSPACE)
            {
                strncat(playerName, &getVal, 1);
                namePos++;
                mvdeleteln(midy - 2, 0);
                mvprintw(midy - 2, midx - (namePos / 2), "%s", playerName);
            }
            else if(getVal == BACKSPACE)
            {
                int length = strlen(playerName);
                playerName[length-1] = '\0';
                namePos--;
                mvdeleteln(midy - 2, 0);
                length = strlen(playerName);
                mvprintw(midy - 2, midx - (length / 2), "%s", playerName);
            }
            else
            {
                scanning = false;
                FILE *fp;
                fp = fopen("./ExtraFiles/HighScores.bin", "a");

                if(playerName[0] == ' ')
                {
                    for(int i = 0; i < strlen(playerName) - 1; i++)
                    {
                        playerName[i] = playerName[i + 1];
                    }
                }

                strcpy((highScoresData)->userName, playerName);
                (highScoresData)->userScore = playerSnake->score;

                fwrite(highScoresData, sizeof(struct file), 1, fp);
                fclose(fp);
            }
        }
    }
    timeout(-1);

    free(highScoresData);

    mvprintw(midy, midx - 15, "Press any character to return.");
    getch();
}

void highScoresScreen()
{
    clear();
    refresh();

    timeout(-1);
    int maxx = getmaxx(stdscr);
    int maxy = getmaxy(stdscr);

    struct file *highScoresData;
    highScoresData = (struct file *)malloc(sizeof(struct file) * 11);

    FILE *fp;
    fp = fopen("./ExtraFiles/HighScores.bin", "r");


    fread(highScoresData, sizeof(struct file), 11, fp);

    mvaddstr(maxy/2 - 7, maxx/2 - 5, "Highscores");
    for(int i = 0; i < 10; i++)
    {
        mvprintw(maxy/2 - 6 + i, maxx/2 - 5, "%d: %s %d", i, (highScoresData + (i))->userName, (highScoresData + (i))->userScore);
    }

    getch();
}

bool borderHandling(int topBarrier, int rightBarrier, int bottomBarrier, int leftBarrier, struct snake *playerSnake, bool initSnake, bool wasPaused)
{
    if(initSnake == true || wasPaused == true)
    {
        // Border Creation
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        attron(COLOR_PAIR(1));

        int maxx = getmaxx(stdscr);
        int maxy = getmaxy(stdscr);

        for(int i = 0; i < maxx; i++)
        {
            mvprintw(topBarrier, i, "%c", 196);
            mvprintw(maxy - 1, i, "%c", 196);
        }

        for(int i = 1; i < maxy - 1; i++)
        {
            mvprintw(i, 0, "%c", 179);

            mvprintw(i, maxx - 1, "%c", 179);
        }

        mvprintw(topBarrier, 0, "%c", 218);
        mvprintw(bottomBarrier, 0, "%c", 192);
        mvprintw(topBarrier, maxx-1, "%c", 191);
        mvprintw(bottomBarrier, maxx-1, "%c", 217);

        attroff(COLOR_PAIR(1));
    }

    if((playerSnake->body[0].x >= rightBarrier) || (playerSnake->body[0].x <= leftBarrier) || (playerSnake->body[0].y <= topBarrier) || (playerSnake->body[0].y >= bottomBarrier))
        return true;
    else
    {
        for(int i = 3; i < playerSnake->length - 1; i++)
        {
            if(playerSnake->body[0].x == playerSnake->body[i].x && playerSnake->body[0].y == playerSnake->body[i].y)
                return true;

        }
        return false;
    }
}

void snakeFoodCheck(int foodPos[], struct snake *playerSnake, int maxx, int maxy)
{
    if((playerSnake->body[0].x == foodPos[0])
       && (playerSnake->body[0].y == foodPos[1]))
    {
        foodPos[0] = (rand() % (maxx - 4)) + 2;
        foodPos[1] = (rand() % (maxy - 2)) + 1;
        playerSnake->score += 1;
        playerSnake->length += 1;
    }

    init_pair(FRUITCOLOR, COLOR_RED, COLOR_BLACK);
    attron(COLOR_PAIR(FRUITCOLOR));
    mvaddstr(foodPos[1], foodPos[0], "@");
    attroff(COLOR_PAIR(FRUITCOLOR));
}

void snakeUpdater(struct snake *playerSnake, bool *initSnake, clock_t *lastUpdateTime, bool *wasPaused)
{
    if(delayUpdate(lastUpdateTime) || *initSnake || *wasPaused)
    {
        switch(playerSnake->currentHeadDir)
        {
        case UP:
            playerSnake->body[0].y -= 1;
            break;
        case DOWN:
            playerSnake->body[0].y += 1;
            break;
        case LEFT:
            playerSnake->body[0].x -= 1;
            break;
        case RIGHT:
            playerSnake->body[0].x += 1;
            break;
        }

        init_pair(SNAKECOLOR, COLOR_GREEN, COLOR_BLACK);
        attron(COLOR_PAIR(SNAKECOLOR));

        mvaddch(playerSnake->body[0].y, playerSnake->body[0].x, 'O');

        for(int i = 1; i < playerSnake->length - 1; i++)
        {
            mvaddch(playerSnake->body[i].y, playerSnake->body[i].x, '#');
        }

        mvaddch(playerSnake->body[playerSnake->length - 1].y, playerSnake->body[playerSnake->length - 1].x, ' ');

        for(int i = playerSnake->length - 1; i > 0; i--)
        {
            playerSnake->body[i].x = playerSnake->body[i - 1].x;
            playerSnake->body[i].y = playerSnake->body[i - 1].y;
        }

        attroff(COLOR_PAIR(SNAKECOLOR));

        *initSnake = false;
        *wasPaused = false;
        *lastUpdateTime = clock();
    }
}

bool delayUpdate(clock_t *lastUpdateTime)
{
    const int delayAmount = CLOCKS_PER_SEC/10;

    if((clock() - *lastUpdateTime) >= delayAmount)
    {
        return true;
    }

    return false;
}

bool pauseScreen()
{
    clear();
    mvaddstr(getmaxy(stdscr)/2 - 3, getmaxx(stdscr)/2 - 3, "Paused");
    mvaddstr(getmaxy(stdscr)/2 - 1, getmaxx(stdscr)/2 - 16, "Press any character to continue.");
    timeout(-1);
    getch();
    timeout(0);
    clear();
    return true;
}

void mainMenu(struct snake *playerSnake, int maxx, int maxy)
{
    if(BOOLSOUND)
        PlaySound(MMSOUND, NULL, SND_ASYNC|SND_LOOP);
here:

    initPlayer(playerSnake, maxx, maxy);

    clear();
    refresh();

    int menuSelect = 0; // Play: 0; Controls: 1; How To Play: 2; Exit: 3
    int menuUpdate = 0;
    bool isSelecting = true;
    int i = 0;

    init_pair(HIGHLIGHTSELECTION, COLOR_CYAN, COLOR_BLACK);
    init_pair(DEFAULTSCREEN, COLOR_WHITE, COLOR_BLACK);

    attron(COLOR_PAIR(DEFAULTSCREEN));
    mvaddstr((maxy/2) - 6, (maxx/2) - 8, "Snakes & Ladders");
    attroff(COLOR_PAIR(DEFAULTSCREEN));

    attron(COLOR_PAIR(HIGHLIGHTSELECTION));
    mvaddstr((maxy/2) - 4, (maxx/2) - 2, "Play");           //0
    attroff(COLOR_PAIR(HIGHLIGHTSELECTION));


    mvaddstr((maxy/2) - 3, (maxx/2) - 4, "Controls");       //1
    mvaddstr((maxy/2) - 2, (maxx/2) - 5, "How To Play");    //2
    mvaddstr((maxy/2) - 1, (maxx/2) - 5, "Highscores");     //3
    mvaddstr((maxy/2) - 0, (maxx/2) - 2, "Exit");           //4

    while(isSelecting)
    {
        i = getch();
        //mvprintw(0, 0, "%d", i);
        switch(i)
        {
        case UP:
            if(menuUpdate > 0)
                menuUpdate-= 1;
            break;
        case DOWN:
            if(menuUpdate < 4)
                menuUpdate+= 1;
            break;
        case ENTER:
            isSelecting = false;
            break;
        }

        if(isSelecting)
        {
            if(menuUpdate > menuSelect)
            {
                switch(menuUpdate)
                {
                case 1:
                    mvdeleteln((maxy/2) - 3, (maxx/2) - 4);
                    mvinsertln((maxy/2) - 3, (maxx/2) - 4);

                    mvdeleteln((maxy/2) - 4, (maxx/2) - 2);
                    mvinsertln((maxy/2) - 4, (maxx/2) - 2);

                    attron(COLOR_PAIR(HIGHLIGHTSELECTION));
                    mvaddstr((maxy/2) - 3, (maxx/2) - 4, "Controls");       //1
                    attroff(COLOR_PAIR(HIGHLIGHTSELECTION));

                    attron(COLOR_PAIR(DEFAULTSCREEN));
                    mvaddstr((maxy/2) - 4, (maxx/2) - 2, "Play");           //0
                    attroff(COLOR_PAIR(DEFAULTSCREEN));
                    break;
                case 2:
                    mvdeleteln((maxy/2) - 2, (maxx/2) - 5);
                    mvinsertln((maxy/2) - 2, (maxx/2) - 5);

                    mvdeleteln((maxy/2) - 3, (maxx/2) - 4);
                    mvinsertln((maxy/2) - 3, (maxx/2) - 4);

                    attron(COLOR_PAIR(HIGHLIGHTSELECTION));
                    mvaddstr((maxy/2) - 2, (maxx/2) - 5, "How To Play");    //2
                    attroff(COLOR_PAIR(HIGHLIGHTSELECTION));

                    attron(COLOR_PAIR(DEFAULTSCREEN));
                    mvaddstr((maxy/2) - 3, (maxx/2) - 4, "Controls");       //1
                    attroff(COLOR_PAIR(DEFAULTSCREEN));
                    break;
                case 3:
                    mvdeleteln((maxy/2) - 1, (maxx/2) - 2);
                    mvinsertln((maxy/2) - 1, (maxx/2) - 2);

                    mvdeleteln((maxy/2) - 2, (maxx/2) - 5);
                    mvinsertln((maxy/2) - 2, (maxx/2) - 5);

                    attron(COLOR_PAIR(HIGHLIGHTSELECTION));
                    mvaddstr((maxy/2) - 1, (maxx/2) - 5, "Highscores");     //3
                    attroff(COLOR_PAIR(HIGHLIGHTSELECTION));

                    attron(COLOR_PAIR(DEFAULTSCREEN));
                    mvaddstr((maxy/2) - 2, (maxx/2) - 5, "How To Play");    //2
                    attroff(COLOR_PAIR(DEFAULTSCREEN));
                    break;
                case 4:
                    mvdeleteln((maxy/2) - 0, (maxx/2) - 2);
                    mvinsertln((maxy/2) - 0, (maxx/2) - 2);

                    mvdeleteln((maxy/2) - 1, (maxx/2) - 5);
                    mvinsertln((maxy/2) - 1, (maxx/2) - 5);

                    attron(COLOR_PAIR(HIGHLIGHTSELECTION));
                    mvaddstr((maxy/2) - 0, (maxx/2) - 2, "Exit");           //4
                    attroff(COLOR_PAIR(HIGHLIGHTSELECTION));

                    attron(COLOR_PAIR(DEFAULTSCREEN));
                    mvaddstr((maxy/2) - 1, (maxx/2) - 5, "Highscores");     //3
                    attroff(COLOR_PAIR(DEFAULTSCREEN));
                    break;
                }
                menuSelect = menuUpdate;
                refresh();
            }
            else if(menuUpdate < menuSelect)
            {
                switch(menuUpdate)
                {
                case 0:
                    mvdeleteln((maxy/2) - 3, (maxx/2) - 4);
                    mvinsertln((maxy/2) - 3, (maxx/2) - 4);

                    mvdeleteln((maxy/2) - 4, (maxx/2) - 2);
                    mvinsertln((maxy/2) - 4, (maxx/2) - 2);

                    attron(COLOR_PAIR(HIGHLIGHTSELECTION));
                    mvaddstr((maxy/2) - 4, (maxx/2) - 2, "Play");           //0
                    attroff(COLOR_PAIR(HIGHLIGHTSELECTION));

                    attron(COLOR_PAIR(DEFAULTSCREEN));
                    mvaddstr((maxy/2) - 3, (maxx/2) - 4, "Controls");       //1
                    attroff(COLOR_PAIR(DEFAULTSCREEN));
                    break;
                case 1:
                    mvdeleteln((maxy/2) - 2, (maxx/2) - 5);
                    mvinsertln((maxy/2) - 2, (maxx/2) - 5);

                    mvdeleteln((maxy/2) - 3, (maxx/2) - 4);
                    mvinsertln((maxy/2) - 3, (maxx/2) - 4);

                    attron(COLOR_PAIR(HIGHLIGHTSELECTION));
                    mvaddstr((maxy/2) - 3, (maxx/2) - 4, "Controls");       //1
                    attroff(COLOR_PAIR(HIGHLIGHTSELECTION));

                    attron(COLOR_PAIR(DEFAULTSCREEN));
                    mvaddstr((maxy/2) - 2, (maxx/2) - 5, "How To Play");    //2
                    attroff(COLOR_PAIR(DEFAULTSCREEN));
                    break;
                case 2:
                    mvdeleteln((maxy/2) - 1, (maxx/2) - 2);
                    mvinsertln((maxy/2) - 1, (maxx/2) - 2);

                    mvdeleteln((maxy/2) - 2, (maxx/2) - 5);
                    mvinsertln((maxy/2) - 2, (maxx/2) - 5);

                    attron(COLOR_PAIR(HIGHLIGHTSELECTION));
                    mvaddstr((maxy/2) - 2, (maxx/2) - 5, "How To Play");    //2
                    attroff(COLOR_PAIR(HIGHLIGHTSELECTION));

                    attron(COLOR_PAIR(DEFAULTSCREEN));
                    mvaddstr((maxy/2) - 1, (maxx/2) - 5, "Highscores");     //3
                    attroff(COLOR_PAIR(DEFAULTSCREEN));
                    break;
                case 3:
                    mvdeleteln((maxy/2) - 0, (maxx/2) - 2);
                    mvinsertln((maxy/2) - 0, (maxx/2) - 2);

                    mvdeleteln((maxy/2) - 1, (maxx/2) - 5);
                    mvinsertln((maxy/2) - 1, (maxx/2) - 5);

                    attron(COLOR_PAIR(HIGHLIGHTSELECTION));
                    mvaddstr((maxy/2) - 1, (maxx/2) - 5, "Highscores");     //3
                    attroff(COLOR_PAIR(HIGHLIGHTSELECTION));

                    attron(COLOR_PAIR(DEFAULTSCREEN));
                    mvaddstr((maxy/2) - 0, (maxx/2) - 2, "Exit");           //4
                    attroff(COLOR_PAIR(DEFAULTSCREEN));
                    break;
                }
                menuSelect = menuUpdate;
                refresh();
            }
            else
            {
                refresh();
            }
        }
    }

    switch(menuSelect)
    {
    case PLAY:
        gameScreen(maxx, maxy, playerSnake);
        deathScreen(playerSnake);
        break;
    case CONTROLS:
        controlsScreen(maxx/2, maxy/2);
        goto here;
        break;
    case HTP:
        htpScreen(maxx/2, maxy/2);
        break;
    case HIGHSCORES:
        highScoresScreen();
        break;
    case EXIT:
        endwin();
        exit(0);
        break;
    default:
        break;
    }

    refresh();  // output might be buffered, this forces copy to "screen"
}

void htpScreen(int midx, int midy)
{
    clear();
    refresh();

    if(BOOLSOUND)
        PlaySound(HTPSOUND, NULL, SND_ASYNC|SND_LOOP);

    mvaddstr(midy - 6, midx - 5, "How To Play");
    mvaddstr(midy - 4, midx - 17, "Use the Controls to move the Snake");
    mvaddstr(midy - 3, midx - 27, "Collect the @pples to increase your Score and Length");
    mvaddstr(midy - 2, midx - 19, "Avoid the Walls and your Body or perish");
    mvaddstr(midy, midx - 15, "Press Any Character To Return.");

    getch();

    refresh();

}

void controlsScreen(int midx, int midy)
{
    clear();
    refresh();

    // TODO Implement The Ability to Change Controls
    mvaddstr((midy) - 6, (midx) - 4, "Controls");
    mvprintw((midy) - 4, (midx) - 8, "Up: Up Arrow Key");
    mvprintw(midy - 3, midx - 10, "Down: Down Arrow Key");
    mvprintw(midy - 2, midx - 10, "Left: Left Arrow Key");
    mvprintw(midy - 1, midx - 11, "Right: Right Arrow Key");

    mvaddstr(midy + 1, midx - 15, "Press Any Character To Return.");
    getch();

    refresh();
}

void initCurses(void)
{
    initscr();              // initialize curses, this also "clears" the screen
    keypad(stdscr, true);   // enables arrow-keys with getch along with other things
    cbreak();               // among other things, disable buffering
    noecho();               // disable "echo" of characters from input
    curs_set(0);            // makes the cursor invisible

    if(has_colors())
        start_color();      // enables coloring the terminal
}
