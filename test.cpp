#include <thread>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <atomic>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
int main()
{
    static std::atomic<bool> exit{false};
    static std::mutex m;
    static auto now = +[](std::ostream &os)->std::ostream& {
        auto now   = std::chrono::high_resolution_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto us    = std::chrono::duration_cast<std::chrono::microseconds>(
                      now.time_since_epoch())
                      .count()
                  % 1000000;
        std::lock_guard<std::mutex> g{m};
        return os << std::put_time(std::localtime(&now_c), "%F %T") << '.'
                  << std::setw(6) << std::setfill('0') << us;
    };
    std::thread t1{[] {
        constexpr int shmid = 0;
        while (not exit)
        {
            auto ptr = shmat(shmid, 0, SHM_RDONLY);
            if (ptr != (char *)-1)
            {
                std::cout << now << " shmat(" << shmid << ") succeeded" << std::endl;
                if (shmdt(ptr) == -1)
                {
                    std::cout << "shmdt(" << ptr << " failed" << std::endl;
                }
            }
        }
    }};
    std::thread t2{[] {
        while (not exit)
        {
            auto id = shmget(IPC_PRIVATE, 1000, 0600 | IPC_CREAT | IPC_EXCL);
            if (id < 0)
            {
                std::cout << "shmget failed" << std::endl;
                break;
            }
            if (id > 0)
            {
                shmid_ds info;
                if (shmctl(id, IPC_STAT, &info) < 0)
                {
                    std::cout << "shmctl IPC_STAT failed" << std::endl;
                    break;
                }
                if (info.shm_nattch)
                {
                    std::cout << now << " shmget(IPC_PRIVATE) created seg with id " << id << ", size " << info.shm_segsz << " and nattach = " << info.shm_nattch << std::endl;
                    exit = true;
                    while(shmctl(id, IPC_STAT, &info) >= 0 and info.shm_nattch)
                        ;
                    shmctl(id, IPC_RMID, nullptr);
                    break;
                }
            }
            if (shmctl(id, IPC_RMID, nullptr) < 0)
            {
                std::cout << "shmctl IPC_RMID failed" << std::endl;
                break;
            }
        }
    }};
    t1.join();
    t2.join();
}
