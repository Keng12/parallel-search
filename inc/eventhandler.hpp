#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

#include <cmath>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "threadpool.hpp"
#include "vector.hpp"

// Create lambda function to take indices as pass by copy/reference?
// Check if indices empty -> Yes: Quit
// No: Pop and continue
namespace kyc
{
    class EventHandler
    {
        std::condition_variable_any &mCV;
        std::shared_timed_mutex &mMutex;
        bool &mSearchCanceled;
        bool &mEventHandler;
        std::string mBufferedInput{};
        std::thread mThread{};
        char getKeyboardInput();
        void inputLoop();

    public:
        EventHandler(std::condition_variable_any &cv, std::shared_timed_mutex &mutex, bool &searchCanceled, bool &eventHandler);
        ~EventHandler();
        std::string getBufferedString() const;
    };
} // namespace kyc
#endif