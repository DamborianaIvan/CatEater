#pragma once

#include <Arduino.h>

enum class RemoteCommandStatus
{
    Pending,
    Executing,
    Completed
};

struct RemoteCommand
{
    String commandId;
    RemoteCommandStatus status;
    int portions;
};

class RemoteCommandStorage
{
   public:
    bool begin();

    bool load(RemoteCommand& command);
    bool save(const RemoteCommand& command);
    bool clear();

   private:
    static constexpr char COMMAND_FILE[] = "/remote_command.json";
};