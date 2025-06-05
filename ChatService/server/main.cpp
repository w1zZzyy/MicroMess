#include "server.hpp"

int main(int argc, char* argv[]) 
{
    assert(argc == 2);

    try {
        Server server(argv[1]);
        server.launch();
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}