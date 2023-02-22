#include "eventhandler.hpp"

namespace kyc
{
    EventHandler::EventHandler(std::condition_variable_any &cv, std::shared_timed_mutex &mutex, bool &searchCanceled, bool &eventHandler) : mCV{cv}, mMutex{mutex}, mSearchCanceled{searchCanceled}, mEventHandler{eventHandler}
    {
        if (mEventHandler)
        {
            mThread = std::thread{&EventHandler::inputLoop, this};
        }
    }

    EventHandler::~EventHandler()
    {
        mThread.join();
    }
    void EventHandler::inputLoop()
    {
        while (true)
        {
            char input = getKeyboardInput();
            {
                std::lock_guard<std::shared_timed_mutex> const lock{mMutex};
                mBufferedInput.append(1, input);
                if (!mSearchCanceled)
                {
                    mSearchCanceled = true;
                }
            }
            mCV.notify_one();
            if (input != '0')
            {
                return;
            }
        }
    }

    char EventHandler::getKeyboardInput()
    {
        char input = 'A';
        return input;
    }

    std::string EventHandler::getBufferedString() const
    {
        return mBufferedInput;
    }
}