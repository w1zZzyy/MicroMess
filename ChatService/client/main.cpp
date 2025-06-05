#include "client.hpp"

int main(int argc, char* argv[]) 
{
    assert(argc == 3);

    try {
        Client client(argv[1], argv[2]);
        client.launch();
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}