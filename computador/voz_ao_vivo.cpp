
#include <iostream>
#include <portaudio.h>
#include <vosk_api.h>

int main() {
    const int taxa = 16000;
    const int tamanhoBloco = 4000;

    Pa_Initialize();

    PaStream *stream;

    PaError erro = Pa_OpenDefaultStream(
        &stream,
        1,
        0,
        paInt16,
        taxa,
        4000,
        nullptr,
        nullptr
    );

    if (erro != paNoError) {
        std::cout << "Erro ao abrir microfone: "
                  << Pa_GetErrorText(erro) << std::endl;
        Pa_Terminate();
        return 1;
    }

    VoskModel *modelo = vosk_model_new(
        "../vosk-model-small-pt-0.3"
    );

    if (modelo == nullptr) {
        std::cout << "Erro ao carregar modelo!" << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

VoskRecognizer *reconhecedor =
    vosk_recognizer_new_grm(
        modelo,
        taxa,
        "[\"zero\", \"um\", \"dois\", \"três\", \"quatro\", \"cinco\", \"seis\", \"sete\", \"oito\", \"nove\"]"
    );
    short audio[tamanhoBloco];

    Pa_StartStream(stream);

    std::cout << "Fale alguma coisa. Pressione Ctrl+C para sair."
              << std::endl;

    while (true) {
        erro = Pa_ReadStream(stream, audio, tamanhoBloco);

        if (erro != paNoError && erro != paInputOverflowed) {
            std::cout << "Erro ao ler microfone." << std::endl;
            break;
        }

        if (vosk_recognizer_accept_waveform(
                reconhecedor,
                reinterpret_cast<const char*>(audio),
                sizeof(audio))) {

            std::cout << "Resultado: "
                      << vosk_recognizer_result(reconhecedor)
                      << std::endl;
        }
    }

    vosk_recognizer_free(reconhecedor);
    vosk_model_free(modelo);
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
