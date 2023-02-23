// #include<conio.h>

#include "eventhandler.hpp"

namespace kyc
{
    EventHandler::EventHandler(std::condition_variable &cv, std::mutex &mutex, bool &searchCanceled) : mCV{cv}, mMutex{mutex}, mSearchCanceled{searchCanceled}
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
            // char const input = getKeyboardInput();
            std::string input{};
            std::cin >> input;
            std::cout << "Received character: " << input << std::endl;
            {
                std::lock_guard<std::mutex> const lock{mMutex};
                mBufferedInput.append(input);
                mSearchCanceled = true;
            }
            mCV.notify_one();
            if (input == "0" || mBufferedInput.length() >= 5)
            {
                return;
            }
        }
    }

    char EventHandler::getKeyboardInput() const
    {
        char input = 'A';
        // char input = _getch();
        return input;
    }

    std::string EventHandler::getBufferedString() const
    {
        return mBufferedInput;
    }
}