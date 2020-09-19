#include <fstream>

#include <QDebug>

#include <crypto.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        qWarning() << "Please provide a password to encrypt the key with";
        return EXIT_FAILURE;
    }
    if (argc > 2)
    {
        qWarning() << "Too many arguments";
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
