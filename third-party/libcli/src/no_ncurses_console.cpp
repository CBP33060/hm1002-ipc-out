/*
    Copyright (c) 2006-2018, Alexis Royer, http://alexis.royer.free.fr/CLI

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation
          and/or other materials provided with the distribution.
        * Neither the name of the CLI library project nor the names of its contributors may be used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "cli/pch.h"

#include <stdio.h>
#include <string.h> // strlen
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>

#include "cli/no_ncurses_console.h"
#include "cli/traces.h"

#include <ncurses/curses.h>

CLI_NS_USE(cli)

//! @brief ncurses console trace class singleton redirection.
#define CLI_NO_NCURSES_CONSOLE GetTraceClass()
//! @brief ncurses console trace class singleton.
static const TraceClass& GetTraceClass(void)
{
    static const TraceClass cli_TraceClass("CLI_NO_NCURSES_CONSOLE", Help()
        .AddHelp(Help::LANG_EN, "CLI ncurses console traces")
        .AddHelp(Help::LANG_FR, "Traces de la console ncurses du CLI"));
    return cli_TraceClass;
}


No_Ncurses_Console::No_Ncurses_Console(const bool B_AutoDelete)
  : IODevice("no-ncurses-console", B_AutoDelete),
    m_pFileName(NULL)
{
    GetTraces().Declare(CLI_NO_NCURSES_CONSOLE);
    m_fd = -1;
}

No_Ncurses_Console::No_Ncurses_Console(const bool B_AutoDelete, const char* const STR_FileName)
  : IODevice("no-ncurses-console", B_AutoDelete),
    m_pFileName(STR_FileName)
{
    GetTraces().Declare(CLI_NO_NCURSES_CONSOLE);
    m_fd = -1;
}


No_Ncurses_Console::~No_Ncurses_Console(void)
{
    No_Ncurses_Console::CloseDevice();
}

const bool No_Ncurses_Console::UartDisableEcho(int fd)
{
    struct termios newtio,oldtio;
    /*获取原有串口配置*/
    if  ( tcgetattr( fd,&oldtio)  !=  0) { 
        perror("SetupSerial 1");
        return -1;
    }
    
    tcflush(fd,TCIFLUSH);
    oldtio.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
    oldtio.c_oflag  &= ~OPOST;   /*Output*/
    if((tcsetattr(fd,TCSANOW,&oldtio))!=0)
    {
        perror("com set error");
        return false;
    }

    return true;
}

const bool No_Ncurses_Console::OpenDevice(void)
{
	if(m_pFileName != NULL)
	{
       	m_fd = open(m_pFileName, O_RDWR);
        if(m_fd < 0)
        {
            GetTraces().SafeTrace(CLI_NO_NCURSES_CONSOLE, *this) << "open " << m_pFileName << "failure" << endl;
            return false;
        }

        // 暂时写死在代码里面，后面如果需要配置这些产生需要在OutputDevice这个基类里面添加配置的函数
        UartDisableEcho(m_fd);
	}

    return true;
}

const bool No_Ncurses_Console::CloseDevice(void)
{
    if(m_fd != -1)
    {        
        close(m_fd);
        m_fd = -1;
    }

    return true;
}

const int No_Ncurses_Console::GetCh(void) const
{
    char ch;
    int fd;

    if(m_pFileName != NULL && m_fd != -1 )
    {
        fd = m_fd;
    }
    else
    {
        fd = 0; // stdin
    }
    
    read(fd, &ch, 1);

    return ch;
}

void No_Ncurses_Console::PutCh(char ch) const
{
    int fd;

    if(m_pFileName != NULL && m_fd != -1 )
    {
        fd = m_fd;
    }
    else
    {
        fd = 1;// stdout
    }

    write(fd, &ch, 1);
}

const KEY No_Ncurses_Console::GetKey(void) const
{
    // ncurses constants.
    static const int NC_KEY_UP = KEY_UP;
        #undef KEY_UP
    static const int NC_KEY_DOWN = KEY_DOWN;
        #undef KEY_DOWN
    static const int NC_KEY_LEFT = KEY_LEFT;
        #undef KEY_LEFT
    static const int NC_KEY_RIGHT = KEY_RIGHT;
        #undef KEY_RIGHT
    static const int NC_KEY_END = KEY_END;
        #undef KEY_END

    while (1)
    {
        int i_Char = GetCh();
        GetTraces().SafeTrace(CLI_NO_NCURSES_CONSOLE, *this) << "i_Char = " << i_Char << endl;
        switch (i_Char)
        {
        // Breakers.
        case 27:
                i_Char = GetCh();
                GetTraces().SafeTrace(CLI_NO_NCURSES_CONSOLE, *this) << "i_Char2 = " << i_Char << endl;
                switch (i_Char)
                {
                case ERR:   return cli::ESCAPE;     // Escape character.
                case 'c':   return cli::COPY;       // ALT+C
                case 3:     return cli::COPY;       // CTR+ALT+C
                case 'x':   return cli::CUT;        // ALT+X
                case 24:    return cli::CUT;        // CTRL+ALT+X
                case 'v':   return cli::PASTE;      // ALT+V
                case 22:    return cli::PASTE;      // CTRL+ALT+V
                case 'z':   return cli::UNDO;       // ALT+Z
                case 26:    return cli::UNDO;       // CTRL+ALT+Z
                case 'y':   return cli::REDO;       // ALT+Y
                case 25:    return cli::REDO;       // CTRL+ALT+Y
                default:
                    // Unknown sequence.
                    break;
                }
            break;
        case 130:
                i_Char = GetCh();
                GetTraces().SafeTrace(CLI_NO_NCURSES_CONSOLE, *this) << "i_Char2 = " << i_Char << endl;
                switch (i_Char)
                {
                case 172:   return cli::EURO;       // Tested on 2018-01-10
                default:
                    // Unknown sequence.
                    break;
                }
            break;

        // Deletions.
        case KEY_BACKSPACE: // Test failed on 2018-01-08: seems that some ncurses implementations do not use that constant.
#if KEY_BACKSPACE != 127
        case 127:           // See https://stackoverflow.com/questions/27200597/c-ncurses-key-backspace-not-working#27203263
#endif
                            GetTraces().SafeTrace(CLI_NO_NCURSES_CONSOLE, *this) << "KEY_BACKSPACE (ncurses) = " << (int) KEY_BACKSPACE << endl;
                            return cli::BACKSPACE;
        case KEY_DC:        return cli::DELETE;
        case KEY_SDC:       return cli::CUT;        // SHIFT+DELETE, tested on 2018-01-09
        case 1011:          return cli::CUT;        // CTRL+SHIFT+DELETE, tested on 2018-01-09, no octal equivalent in 'curses.h'
        case KEY_IC:        return cli::INSERT;

        // Movements
        case NC_KEY_UP:     return cli::KEY_UP;
        case KEY_PPAGE:     return cli::PAGE_UP;
        case 567:           return cli::PAGE_UP;    // CTRL+UP, tested on 2018-01-09, no octal equivalent in 'curses.h'
        case NC_KEY_DOWN:   return cli::KEY_DOWN;
        case KEY_NPAGE:     return cli::PAGE_DOWN;
        case 526:           return cli::PAGE_DOWN;  // CTRL+DOWN, tested on 2018-01-08, no octal equivalent in 'curses.h'
        case NC_KEY_LEFT:   return cli::KEY_LEFT;
        case 546:           return cli::PAGE_LEFT;  // CTRL+LEFT, tested on 2018-01-08, no octal equivalent in 'curses.h'
        case NC_KEY_RIGHT:  return cli::KEY_RIGHT;
        case 561:           return cli::PAGE_RIGHT; // CTRL+RIGHT, tested on 2018-01-08, no octal equivalent in 'curses.h'
        case KEY_HOME:      return cli::KEY_BEGIN;
        case 1:             return cli::KEY_BEGIN;  // CTRL+A
        case NC_KEY_END:    return cli::KEY_END;
        case 5:             return cli::KEY_END;    // CTRL+E

        // Accented characters
#if 0
        // iso-8859-1 specific
        case 225:           return cli::KEY_aacute;
        case 224:           return cli::KEY_agrave;
        case 228:           return cli::KEY_auml;
        case 226:           return cli::KEY_acirc;
        case 231:           return cli::KEY_ccedil;
        case 233:           return cli::KEY_eacute;
        case 232:           return cli::KEY_egrave;
        case 235:           return cli::KEY_euml;
        case 234:           return cli::KEY_ecirc;
        case 237:           return cli::KEY_iacute;
        case 236:           return cli::KEY_igrave;
        case 239:           return cli::KEY_iuml;
        case 238:           return cli::KEY_icirc;
        case 243:           return cli::KEY_oacute;
        case 242:           return cli::KEY_ograve;
        case 246:           return cli::KEY_ouml;
        case 244:           return cli::KEY_ocirc;
        case 250:           return cli::KEY_uacute;
        case 249:           return cli::KEY_ugrave;
        case 252:           return cli::KEY_uuml;
        case 251:           return cli::KEY_ucirc;
#endif

        // Special characters.
        case 96:            return cli::BACK_QUOTE;
#if 0
        // iso-8859-1 specific
        case 163:           return cli::POUND;
        //case 167:           return cli::PARAGRAPH; // Conflict with utf-8 encoding for cli::KEY_ccedil.
        case 176:           return cli::DEGREE;
        //case 178:           return cli::SQUARE; // Conflict with utf-8 encoding for cli::KEY_ograve.
        case 181:           return cli::MICRO;
#endif

        // Control sequences.
        case 3:             return cli::BREAK;      // CTRL+C
        case 4:             return cli::LOGOUT;     // CTRL+D
        case 12:            return cli::CLS;        // CTRL+L
        case 14:            return cli::NEXT;       // CTRL+N
        case 559:           return cli::NEXT;       // ALT+RIGHT, tested on 2018-01-09, no octal equivalent in 'curses.h'
        case 16:            return cli::PREVIOUS;   // CTRL+P
        case 544:           return cli::PREVIOUS;   // ALT+LEFT, tested on 2018-01-09, no octal equivalent in 'curses.h'
        case 25:            return cli::REDO;       // CTRL+Y
        case 26:            return cli::UNDO;       // CTRL+Z, tested on 2018-01-09, no octal equivalent in 'curses.h'
        case 24:            return cli::CUT;        // CTRL+X, tested on 2018-01-09, no octal equivalent in 'curses.h'
        case 521:           return cli::CUT;        // CTRL+SHIFT+DELETE, tested on 2018-01-10, no octal equivalent in 'curses.h'
        case 22:            return cli::PASTE;      // CTRL+V, tested on 2018-01-09, no octal equivalent in 'curses.h'

        // Function keys.
        case 265:           return F1;
        case 266:           return F2;
        case 267:           return F3;
        case 268:           return F4;
        case 269:           return F5;
        case 270:           return F6;
        case 271:           return F7;
        case 272:           return F8;
        case 273:           return F9;
        case 274:           return F10;
        case 275:           return F11;
        case 276:           return F12;

        default:
            do {
                // Call the base implementation transformation routine.
                const KEY e_Char = Char2Key(i_Char);
                if (e_Char == NULL_KEY)
                {
                    GetTraces().SafeTrace(CLI_NO_NCURSES_CONSOLE, *this) << "Char2Key(i_Char) -> NULL_KEY" << endl;
                    // Do nothing, just ignore.
                    // Let the loop waiting for another character.
                }
                else if (e_Char == FEED_MORE)
                {
                    GetTraces().SafeTrace(CLI_NO_NCURSES_CONSOLE, *this) << "Char2Key(i_Char) -> FEED_MORE" << endl;
                    // Let the loop waiting for another character.
                }
                else
                {
                    GetTraces().SafeTrace(CLI_NO_NCURSES_CONSOLE, *this) << "Char2Key(i_Char) -> " << (int) e_Char << endl;
                    return e_Char;
                }
            } while(0);
        }
    }
}

void No_Ncurses_Console::PutString(const char* const STR_Out) const
{
    char str_Char[2] = { '\0', '\0' };

    for (const char* pc_Out = STR_Out; (pc_Out != NULL) && (*pc_Out != '\0'); pc_Out++)
    {
        str_Char[0] = *pc_Out;
        PutCh(str_Char[0]);
    }
}

int No_Ncurses_Console::PutBuffer(const char* const STR_Out,int len) const
{
    if(STR_Out == NULL)
    {
        return -2;
    }

    for (int i = 0; i < len; i++)
    {
        PutCh(STR_Out[i]);
    }

    return len;
}

void No_Ncurses_Console::Beep(void) const
{

}

void No_Ncurses_Console::CleanScreen(void) const
{
 
}