#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
int main()
{
    shmat(0, 0, SHM_RDONLY);
}
