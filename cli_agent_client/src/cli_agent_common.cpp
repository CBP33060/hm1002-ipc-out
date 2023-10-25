#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cli_agent_common.hpp"
#include "log_mx.h"
#include "fw_env_para.h"

cli::Cli* const GetCli(void)
{
    cli::Cli::List cli_CLIs(1);
    if ((cli::Cli::FindFromName(cli_CLIs, ".*") <= 0) || (cli_CLIs.IsEmpty()))
    {
        cli::OutputDevice::GetStdErr() << "No CLI found" << cli::endl;
    }
    else
    {
        if (const cli::Cli* const pcli_Cli = cli_CLIs.GetHead())
        {
            return const_cast<cli::Cli*>(pcli_Cli);
        }
    }

    return NULL;
}

cli::Shell* const GetShell(void)
{
    if (const cli::Cli* const pcli_Cli = GetCli())
    {
        if (cli::Shell* const pcli_Shell = & pcli_Cli->GetShell())
        {
            return pcli_Shell;
        }
    }

    return NULL;
}

int parseNumber(const std::string& str) 
{
    size_t firstColon = str.find(':');
    if (firstColon != std::string::npos) 
    {
        size_t secondColon = str.find(':', firstColon + 1);
        if (secondColon != std::string::npos) 
        {
            std::string numberStr = str.substr(secondColon + 1);
            return std::atoi(numberStr.c_str());
        }
    }
    return 0;
}
