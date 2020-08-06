#include <fstream>

#include <crypto.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Please provide a password to encrypt the key with\n");
        return EXIT_FAILURE;
    }
    if (argc > 2)
    {
        printf("Too many arguments\n");
        return EXIT_FAILURE;
    }

    ThorQ::Crypto c;
    c.save("serverr", argv[1]);

    std::ofstream file("server.pub", std::ofstream::binary);
    {
        auto dat = c.publicKey();
        file.write(reinterpret_cast<char*>(dat.data()), dat.size());
    }
    file.close();
}
