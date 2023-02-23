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
        std::condition_variable &mCV;
        std::mutex &mMutex;
        bool &msearchInterrupted;
        std::string mBufferedInput{};
        std::thread mThread{};
        char getKeyboardInput() const;
        void inputLoop();

    public:
        EventHandler(std::condition_variable &cv, std::mutex &mutex, bool &searchInterrupted);
        ~EventHandler();
        std::string getBufferedString() const;
    };
} // namespace kyc
#endif
