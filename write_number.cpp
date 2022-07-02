#include <windows.h>
#include <stdio.h>
int main(int argv,char** argc)
{   
    DWORD t1,PICTURE_SIZE=20,FONT_SIZE=15;
    if(argv==3){
        PICTURE_SIZE=atoi(argc[1]);
        FONT_SIZE=atoi(argc[2]);
    }
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE), hin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hin, &t1);
    t1 &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(hin, t1);
    CONSOLE_CURSOR_INFO cursor = {25, 0};
    SetConsoleCursorInfo(hout, &cursor);
    printf("Press 'z' to reset the console\nPress ' ' to the next number\n\nLook at the console title\n\nPress 'Enter' to begin...");
    getchar();
    SMALL_RECT rc = {0, 0, PICTURE_SIZE - 1, PICTURE_SIZE - 1};
    SetConsoleWindowInfo(hout, 1, &rc);
    SetConsoleScreenBufferSize(hout, {PICTURE_SIZE, PICTURE_SIZE});
    CONSOLE_FONT_INFOEX cfi = {84};
    cfi.dwFontSize.X = cfi.dwFontSize.Y = FONT_SIZE;
    SetCurrentConsoleFontEx(hout, 0, &cfi);
    FillConsoleOutputCharacter(hout, ' ', PICTURE_SIZE * PICTURE_SIZE, {0, 0}, &t1);
    char *pic = (char *)malloc(PICTURE_SIZE * PICTURE_SIZE),now_number=0,count=0, *title = (char*)malloc(100);
    INPUT_RECORD buffer;
    FILE *fout = fopen("pic\\set.ini", "w");
    fprintf(fout, "%d %d", PICTURE_SIZE);
    fclose(fout);
    for (t1 = 0; t1 < PICTURE_SIZE * PICTURE_SIZE; ++t1)
        pic[t1] = -1;
    while (1)
    {
        sprintf(title, "Now write %d times:%d", now_number, count + 1);
        SetConsoleTitle(title);
        ReadConsoleInput(hin, &buffer, 1, &t1);
        if (buffer.EventType == KEY_EVENT && buffer.Event.KeyEvent.bKeyDown && (buffer.Event.KeyEvent.uChar.AsciiChar == ' ' || buffer.Event.KeyEvent.uChar.AsciiChar == 'z'))
        {
            if (buffer.Event.KeyEvent.uChar.AsciiChar == ' ')
            {
                sprintf(title, "Saving...");
                SetConsoleTitle(title);
                sprintf(title, "pic\\%d_%d.pic", now_number, count++);
                if (count == 5)
                {
                    now_number++;
                    count = 0;
                }
                fout = fopen(title, "w");
                fwrite(pic,1,PICTURE_SIZE*PICTURE_SIZE,fout);
                fclose(fout);
                sprintf(title, "Save successfully!");
                SetConsoleTitle(title);
                Sleep(100);
                if (now_number == 10)
                {
                    sprintf(title, "Thank you! Press 'Enter' to exit");
                    SetConsoleTitle(title);
                    getchar();
                    exit(0);
                }
            }
            sprintf(title, "Reseting...");
            SetConsoleTitle(title);
            for (t1 = 0; t1 < PICTURE_SIZE * PICTURE_SIZE; ++t1)
                pic[t1] = -1;
            FillConsoleOutputCharacter(hout, ' ', PICTURE_SIZE * PICTURE_SIZE, {0, 0}, &t1);
            sprintf(title, "Reset successfully!");
            SetConsoleTitle(title);
            Sleep(100);
        }
        else if (buffer.EventType == MOUSE_EVENT && buffer.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
        {
            COORD pos = buffer.Event.MouseEvent.dwMousePosition;
            pic[pos.Y       * PICTURE_SIZE + pos.X    ] = 1;
            FillConsoleOutputCharacter(hout, '@', 1, {pos.X, pos.Y}, &t1);
        }
    }
}
