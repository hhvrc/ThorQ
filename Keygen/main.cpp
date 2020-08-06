#include <fstream>

#include <crypto.h>

int main(int argc, char** argv)
{
    char id[33];
    id[32] = 0;

    for (int i = 0; i < 32; i++)
    {
        std::uint8_t c = ((uint32_t)rand()) % 62;

        id[i] = (c < 10) ? (c + '0') : (c < 36) ? (c + 'a' - 10) : (c + 'A' - 36);
    }
    printf("%s\n", id);

    /*
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
    file.close();*/
}
