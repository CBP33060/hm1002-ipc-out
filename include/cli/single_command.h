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
//! @brief SingleCommand class definition.

#ifndef _CLI_SINGLE_COMMAND_H_
#define _CLI_SINGLE_COMMAND_H_

#include "cli/namespace.h"
#include "cli/io_device.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    //! @brief Single command device.
    class SingleCommand : public IODevice
    {
    public:
        //! @brief Constructor.
        explicit SingleCommand(
            const char* const STR_Command,      //!< Command.
            OutputDevice& CLI_Output,           //!< Output.
            const bool B_AutoDelete             //!< Auto-deletion flag.
            );

        //! @brief Destructor.
        virtual ~SingleCommand(void);

    private:
        //! @brief No default constructor.
        explicit SingleCommand(void);
        //! @brief No copy constructor.
        SingleCommand(const SingleCommand&);
        //! @brief No assignment operator.
        SingleCommand& operator=(const SingleCommand&);

    public:
        //! @brief Command accessor.
        //! @return Command string.
        const tk::String GetCommand(void) const;

    protected:
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool OpenDevice(void);
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool CloseDevice(void);
    public:
        // Inherit doxygen comments from cli::IODevice interface documentation.
        virtual const KEY GetKey(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void PutString(const char* const STR_Out) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void Beep(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void CleanScreen(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool WouldOutput(const OutputDevice& CLI_Device) const;

    private:
        //! Output device.
        OutputDevice& m_cliOutput;

        //! The command.
        const tk::String m_strCommand;

        //! Current position in the command.
        mutable int m_iPosition;
    };

CLI_NS_END(cli)

#endif // _CLI_SINGLE_COMMAND_H_
