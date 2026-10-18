
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <vosk_api.h>

int main() {
    std::cout << "Carregando modelo Vosk..." << std::endl;

    VoskModel *modelo = vosk_model_new(
        "../vosk-model-small-pt-0.3"
    );

    if (modelo == nullptr) {
        std::cout << "Erro ao carregar o modelo!" << std::endl;
        return 1;
    }

    VoskRecognizer *reconhecedor =
        vosk_recognizer_new(modelo, 16000.0);

    std::ifstream arquivo(
        "gravacao.pcm",
        std::ios::binary
    );

    if (!arquivo) {
        std::cout << "Erro ao abrir gravacao.pcm!" << std::endl;
        vosk_recognizer_free(reconhecedor);
        vosk_model_free(modelo);
        return 1;
    }

    std::vector<char> audio(4000);

    while (arquivo.read(audio.data(), audio.size())
           || arquivo.gcount() > 0) {

        int tamanho = arquivo.gcount();

        vosk_recognizer_accept_waveform(
            reconhecedor,
            audio.data(),
            tamanho
        );
    }

    std::cout << "Resultado: "
              << vosk_recognizer_final_result(reconhecedor)
              << std::endl;

    arquivo.close();
    vosk_recognizer_free(reconhecedor);
    vosk_model_free(modelo);

    return 0;
}
