#include<conio.h>

#include "eventhandler.hpp"

namespace kyc
{
    EventHandler::EventHandler(std::condition_variable &cv, std::mutex &eventMutex, bool &searchInterrupted) : mCV{cv}, mMutex{eventMutex}, mSearchInterrupted{searchInterrupted}
    {
        mThread = std::thread{&EventHandler::inputLoop, this};
    }

    EventHandler::~EventHandler()
    {
        if (mThread.joinable())
        {
            mThread.join();
        }
    }
    void EventHandler::inputLoop()
    {
        while (true)
        {
            char const input = getKeyboardInput();
            {
                std::lock_guard<std::mutex> const lock{mMutex};
                mBufferedInput.append(1, input);
                mSearchInterrupted = true;
            }
            mCV.notify_one();
            if (input == '0' || mBufferedInput.length() >= 4)
            {
                return;
            }
        }
    }

    char EventHandler::getKeyboardInput() const
    {
        char const input = _getch();
        return input;
    }

    std::string EventHandler::getBufferedString() const
    {
        return mBufferedInput;
    }
}