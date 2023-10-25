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


//! @file
//! @author Alexis Royer
//! @brief No_Ncurses_Console class definition.

#ifndef _CLI_NO_NCURSES_CONSOLE_H_
#define _CLI_NO_NCURSES_CONSOLE_H_

#include "cli/namespace.h"
#include "cli/io_device.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    //! @brief No_Ncurses_Console intput/output device class.
    class No_Ncurses_Console : public IODevice
    {
    public:
        //! @brief Constructor.
        explicit No_Ncurses_Console(
            const bool B_AutoDelete     //!< Auto-deletion flag.
            );
	//by sunyang
        explicit No_Ncurses_Console(
            const bool B_AutoDelete,     //!< Auto-deletion flag.
	    const char* const STR_FileName
            );

        //! @brief Destructor.
        virtual ~No_Ncurses_Console(void);

    private:
        //! @brief No default constructor.
        explicit No_Ncurses_Console(void);
        //! @brief No copy constructor.
        No_Ncurses_Console(const No_Ncurses_Console&);
        //! @brief No assignment operator.
        No_Ncurses_Console& operator=(const No_Ncurses_Console&);

    protected:
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool OpenDevice(void);
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool CloseDevice(void);
    public:
        // Inherit doxygen comments from cli::IODevice interface documentation.
        virtual const KEY GetKey(void) const;

    public:
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void PutString(const char* const STR_Out) const;
        
        virtual int PutBuffer(const char* const STR_Out,int len) const;
        
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void Beep(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void CleanScreen(void) const;

    private:
        const bool  UartDisableEcho(int fd);
        const int GetCh(void) const;
        void PutCh(char ch) const;

    private:
        //! Internal data.
	    int m_fd;
	    const char* m_pFileName;
    };

CLI_NS_END(cli)

#endif // _CLI_NO_NCURSES_CONSOLE_H_

