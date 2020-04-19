#include <iostream>
#include "psql_comet.h"

int main()
{
    constexpr size_t MANAGER_THREADS = 4U;
    std::array<std::string, 2U> channels{"channel_1", "channel_2",};
    PSQLcomet comet;
    comet.add_channels(channels);
    comet.start(MANAGER_THREADS);

    return 0;
}
