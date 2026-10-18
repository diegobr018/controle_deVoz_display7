
#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int main() {
    const char *porta = "/dev/ttyACM0";

    int serial = open(porta, O_RDWR | O_NOCTTY);

    if (serial == -1) {
        std::cout << "Erro ao abrir a porta serial!"
                  << std::endl;
        return 1;
    }

    termios configuracao{};

    if (tcgetattr(serial, &configuracao) != 0) {
        std::cout << "Erro ao configurar a porta!"
                  << std::endl;
        close(serial);
        return 1;
    }

    cfsetispeed(&configuracao, B9600);
    cfsetospeed(&configuracao, B9600);

    configuracao.c_cflag &= ~PARENB;
    configuracao.c_cflag &= ~CSTOPB;
    configuracao.c_cflag &= ~CSIZE;
    configuracao.c_cflag |= CS8;
    configuracao.c_cflag |= CREAD | CLOCAL;
    configuracao.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    configuracao.c_iflag &= ~(IXON | IXOFF | IXANY);
    configuracao.c_oflag &= ~OPOST;
    configuracao.c_cc[VMIN] = 0;
    configuracao.c_cc[VTIME] = 10;

    if (tcsetattr(serial, TCSANOW, &configuracao) != 0) {
        std::cout << "Erro ao aplicar configuracao!"
                  << std::endl;
        close(serial);
        return 1;
    }

tcflush(serial, TCIOFLUSH);

sleep(2);

std::string numero = "5\n";
    write(serial, numero.c_str(), numero.size());

    std::cout << "Numero 5 enviado ao Arduino!"
              << std::endl;

    close(serial);

    return 0;
}
