#include <errno.h>
#include "utils.h"
#include "../common/myexcept.h"

void utils::ignore_signal(int sig)
{
    struct sigaction act = {0};
    act.sa_handler = SIG_IGN;
    if(sigaction(sig, &act, NULL) == -1) 
        throw errno_exception(errno);
}