#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdio>
void ExecuteCommand(const std::string& command) {
    std::cout << "Wykonuję: " << command << std::endl;
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Błąd: Nie udało się wykonać polecenia: " << command << std::endl;
        throw std::runtime_error("Command execution failed");
    }
}
std::vector<int> GetWIMIndices(const std::string& wimPath) {
    std::vector<int> indices;
    char buffer[128];
    std::string command = "dism /Get-WimInfo /WimFile:" + wimPath;
    std::cout << "Wykonuję: " << command << std::endl;
    FILE* pipe = popen(command.cstr(), "r");
    if (!pipe) throw std::runtime_error("Failed to run command: " + command);
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (line.find("Index : ") != std::string::npos) {
            indices.push_back(std::stoi(line.substr(line.find(":") + 1)));
        }
    }
    _pclose(pipe);
    if (indices.empty()) {
        throw std::runtime_error("Nie znaleziono indeksów w pliku WIM.");
    }
    return indices;
}
void ConvertWIMtoESD(const std::string& wimPath, const std::string& esdPath, const std::vector<int>& indices) {
    for (int index : indices) {
        std::string command = "dism /Export-Image /SourceImageFile:" + wimPath +
            " /SourceIndex:" + std::to_string(index) +
            " /DestinationImageFile:" + esdPath +
            " /Compress:recovery /CheckIntegrity";
        ExecuteCommand(command);
    }
}
int main() {
    try {
        std::string wimPath = "install.wim";
        std::string esdPath = "install.esd";
        std::vector<int> indices = GetWIMIndices(wimPath);
        std::cout << "Znalezione indeksy: ";
        for (int index : indices) {
            std::cout << index << " ";
        }
        std::cout << std::endl;
        ConvertWIMtoESD(wimPath, esdPath, indices);
        std::cout << "Konwersja zakończona pomyślnie!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Wystąpił błąd: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}