
#include <iostream>
#include <fstream>
#include <portaudio.h>

int main() {
    const int taxa = 16000;
    const int duracao = 5;
    const int amostras = taxa * duracao;

    short audio[amostras];

    PaError erro = Pa_Initialize();

    if (erro != paNoError) {
        std::cout << "Erro: "
                  << Pa_GetErrorText(erro) << std::endl;
        return 1;
    }

    PaStream *stream;

    erro = Pa_OpenDefaultStream(
        &stream,
        1,
        0,
        paInt16,
        taxa,
        256,
        nullptr,
        nullptr
    );

    if (erro != paNoError) {
        std::cout << "Erro ao abrir microfone: "
                  << Pa_GetErrorText(erro) << std::endl;
        Pa_Terminate();
        return 1;
    }

    erro = Pa_StartStream(stream);

    if (erro != paNoError) {
        std::cout << "Erro ao iniciar gravacao: "
                  << Pa_GetErrorText(erro) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Fale agora! Gravando por 5 segundos..."
              << std::endl;

    erro = Pa_ReadStream(stream, audio, amostras);

    if (erro != paNoError && erro != paInputOverflowed) {
        std::cout << "Erro na gravacao: "
                  << Pa_GetErrorText(erro) << std::endl;
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    std::ofstream arquivo("gravacao.pcm", std::ios::binary);

    arquivo.write(
        reinterpret_cast<char*>(audio),
        sizeof(audio)
    );

    arquivo.close();

    std::cout << "Gravacao salva em gravacao.pcm"
              << std::endl;

    return 0;
}

