#include <thread>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
int main()
{
    auto id = shmget(IPC_PRIVATE, 1000, 0600 | IPC_CREAT | IPC_EXCL);
    if (id < 0)
    {
        std::cout << "shmget failed" << std::endl;
        break;
    }
    auto ptr = shmat(id, nullptr, 0);
    if (ptr == (char*)-1)
    {
        std::cout << "shmat(" << id << " failed" << std::endl;
        break;
    }
    shmid_ds info;
    if (shmctl(id, IPC_STAT, &info) < 0)
    {
        std::cout << "shmctl IPC_STAT failed" << std::endl;
        break;
    }
    if (info.shm_nattch != 1)
    {
        std::cout << now << " shmget(IPC_PRIVATE) created seg with id " << id << ", size " << info.shm_segsz << " and nattach = " << info.shm_nattch << std::endl;
        shmdt(ptr);
        while(shmctl(id, IPC_STAT, &info) >= 0 and info.shm_nattch)
            ;
        shmctl(id, IPC_RMID, nullptr);
        break;
    }
    if (shmdt(ptr) == -1)
    {
        std::cout << "shmdt(" << ptr << " failed" << std::endl;
        break;
    }
    if (shmctl(id, IPC_RMID, nullptr) < 0)
    {
        std::cout << "shmctl IPC_RMID failed" << std::endl;
        break;
    }
}
