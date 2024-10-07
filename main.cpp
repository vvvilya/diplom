#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <json/json.h>  // Подключаем jsoncpp для работы с JSON

// Структура для хранения координат
struct Coordinates {
    double latitude;
    double longitude;
};

// Функция для чтения карты из файла terrain.json
std::vector<Coordinates> loadMap(const std::string& filename) {
    std::ifstream file(filename, std::ifstream::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка при открытии файла карты: " << filename << std::endl;
        return {};
    }

    // Создаем объект JSON для парсинга
    Json::Value root;
    file >> root;

    std::vector<Coordinates> terrain;
    for (const auto& point : root["terrain"]) {
        Coordinates coord;
        coord.latitude = point["latitude"].asDouble();
        coord.longitude = point["longitude"].asDouble();
        terrain.push_back(coord);
    }

    return terrain;
}

// Пример функции для моделирования траектории
std::vector<Coordinates> simulateTrajectory(const std::vector<Coordinates>& map, double windFactor) {
    std::vector<Coordinates> trajectory = map;  // Начальная траектория совпадает с картой

    // Простая модель воздействия ветра
    for (auto& coord : trajectory) {
        coord.latitude += windFactor * 0.0001;   // Добавляем небольшой сдвиг по широте
        coord.longitude += windFactor * 0.0001;  // Добавляем небольшой сдвиг по долготе
    }

    return trajectory;
}

// Функция для логирования данных в файл
void logData(const std::vector<Coordinates>& truePath, const std::vector<Coordinates>& looselyPath,
             const std::vector<Coordinates>& tightlyPath, const std::string& filename) {
    std::ofstream logFile(filename);
    if (!logFile.is_open()) {
        std::cerr << "Ошибка при открытии файла для логирования: " << filename << std::endl;
        return;
    }

    logFile << "True Latitude,True Longitude,Loosely Latitude,Loosely Longitude,Tightly Latitude,Tightly Longitude\n";
    for (size_t i = 0; i < truePath.size(); ++i) {
        logFile << truePath[i].latitude << "," << truePath[i].longitude << ","
                << looselyPath[i].latitude << "," << looselyPath[i].longitude << ","
                << tightlyPath[i].latitude << "," << tightlyPath[i].longitude << "\n";
    }
    logFile.close();
}

// Основная функция симуляции
void runSimulation(int scenario) {
    std::vector<Coordinates> truePath = loadMap("terrain.json");
    if (truePath.empty()) {
        std::cerr << "Ошибка загрузки карты." << std::endl;
        return;
    }

    double windFactor = 0.0;
    switch (scenario) {
        case 1: windFactor = 0.0; break;  // Нормальные условия
        case 2: windFactor = 0.5; break;  // Умеренный ветер
        case 3: windFactor = 1.0; break;  // Сильный ветер
        default:
            std::cerr << "Неверный сценарий." << std::endl;
            return;
    }

    // Моделирование траекторий
    std::vector<Coordinates> looselyPath = simulateTrajectory(truePath, windFactor * 0.8);  // Слабо связанная интеграция
    std::vector<Coordinates> tightlyPath = simulateTrajectory(truePath, windFactor * 0.5);  // Тесно связанная интеграция

    // Логирование данных
    logData(truePath, looselyPath, tightlyPath, "flight_log.csv");

    // Вывод на консоль общих отклонений
    double totalLooselyDeviation = 0.0, totalTightlyDeviation = 0.0;
    for (size_t i = 0; i < truePath.size(); ++i) {
        totalLooselyDeviation += std::hypot(
            looselyPath[i].latitude - truePath[i].latitude,
            looselyPath[i].longitude - truePath[i].longitude
        );
        totalTightlyDeviation += std::hypot(
            tightlyPath[i].latitude - truePath[i].latitude,
            tightlyPath[i].longitude - truePath[i].longitude
        );
    }

    std::cout << "Общее отклонение (слабо связанная): " << totalLooselyDeviation << "\n";
    std::cout << "Общее отклонение (тесно связанная): " << totalTightlyDeviation << "\n";
}

int main() {
    int scenario;
    std::cout << "Выберите сценарий (1 - Нормальные условия, 2 - Умеренный ветер, 3 - Сильный ветер): ";
    std::cin >> scenario;

    runSimulation(scenario);
    return 0;
}