
#include <iostream>
#include <string>
#include <cstring>
#include <portaudio.h>
#include <vosk_api.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int main() {
    const int taxa = 16000;
    const int tamanhoBloco = 4000;
    const char *porta = "/dev/ttyACM0";

    // 1. Abrir a comunicacao com o Arduino
    int serial = open(porta, O_RDWR | O_NOCTTY);

    if (serial == -1) {
        std::cerr << "Erro ao abrir a porta serial."
                  << std::endl;
        return 1;
    }

    // Configurar a serial para 9600 bauds
    termios config{};
    tcgetattr(serial, &config);
    cfsetispeed(&config, B9600);
    cfsetospeed(&config, B9600);
    config.c_cflag &= ~PARENB;
    config.c_cflag &= ~CSTOPB;
    config.c_cflag &= ~CSIZE;
    config.c_cflag |= CS8 | CREAD | CLOCAL;
    config.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    config.c_iflag &= ~(IXON | IXOFF | IXANY);
    config.c_oflag &= ~OPOST;
    config.c_cc[VMIN] = 0;
    config.c_cc[VTIME] = 10;
    tcsetattr(serial, TCSANOW, &config);

    // Dar tempo para o Arduino reiniciar
    sleep(2);
    tcflush(serial, TCIOFLUSH);

    // 2. Carregar o modelo Vosk
    VoskModel *modelo =
        vosk_model_new("../vosk-model-small-pt-0.3");

    if (modelo == nullptr) {
        std::cerr << "Erro ao carregar o modelo Vosk."
                  << std::endl;
        close(serial);
        return 1;
    }

    const char *gramatica =
        "[\"zero\", \"um\", \"dois\", \"três\", \"quatro\", "
        "\"cinco\", \"seis\", \"sete\", \"oito\", \"nove\"]";

    VoskRecognizer *reconhecedor =
        vosk_recognizer_new_grm(modelo, taxa, gramatica);

    // 3. Inicializar o microfone
    PaError erro = Pa_Initialize();

    if (erro != paNoError) {
        std::cerr << "Erro no PortAudio: "
                  << Pa_GetErrorText(erro) << std::endl;
        vosk_recognizer_free(reconhecedor);
        vosk_model_free(modelo);
        close(serial);
        return 1;
    }

    PaStream *stream = nullptr;

    erro = Pa_OpenDefaultStream(
        &stream, 1, 0, paInt16,
        taxa, tamanhoBloco, nullptr, nullptr
    );

    if (erro != paNoError) {
        std::cerr << "Erro ao abrir o microfone: "
                  << Pa_GetErrorText(erro) << std::endl;
        Pa_Terminate();
        vosk_recognizer_free(reconhecedor);
        vosk_model_free(modelo);
        close(serial);
        return 1;
    }

    short audio[tamanhoBloco];

    Pa_StartStream(stream);

    std::cout << "Fale um numero de zero a nove."
              << std::endl;
    std::cout << "Pressione Ctrl+C para encerrar."
              << std::endl;

    // 4. Reconhecer a fala e enviar o algarismo
    while (true) {
        erro = Pa_ReadStream(stream, audio, tamanhoBloco);

        if (erro != paNoError && erro != paInputOverflowed) {
            std::cerr << "Erro ao ler o microfone."
                      << std::endl;
            break;
        }

        if (vosk_recognizer_accept_waveform(
                reconhecedor,
                reinterpret_cast<const char*>(audio),
                sizeof(audio))) {

            const char *resultado =
                vosk_recognizer_result(reconhecedor);

            std::string texto(resultado);

            std::cout << "Reconhecimento: "
                      << texto << std::endl;

            const char *numeros[] = {
                "\"zero\"", "\"um\"", "\"dois\"",
                "\"três\"", "\"quatro\"", "\"cinco\"",
                "\"seis\"", "\"sete\"", "\"oito\"",
                "\"nove\""
            };

            for (int i = 0; i < 10; i++) {
                if (texto.find(numeros[i]) != std::string::npos) {
                    char digito = '0' + i;

                    write(serial, &digito, 1);

                    std::cout << "Enviado ao Arduino: "
                              << digito << std::endl;
                    break;
                }
            }
        }
    }

    // 5. Liberar recursos
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    vosk_recognizer_free(reconhecedor);
    vosk_model_free(modelo);
    close(serial);

    return 0;
}
