#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include <random>
#include <json/json.h>  // Подключаем jsoncpp для работы с JSON

// Класс для хранения координат
class Coordinates {
public:
    double latitude;
    double longitude;
    Coordinates(double lat = 0.0, double lon = 0.0) : latitude(lat), longitude(lon) {}
};

// Класс фильтра Калмана
class KalmanFilter {
public:
    double Q; // Процессное шумовое
    double R; // Измерительное шумовое
    double x; // Оценка состояния
    double P; // Оценка ошибки
    double K; // Коэффициент усиления

    KalmanFilter(double processNoise = 0.1, double measurementNoise = 0.5) {
        Q = processNoise;
        R = measurementNoise;
        x = 0;
        P = 1;
    }

    double update(double measurement) {
        // Прогноз
        P += Q; // Обновление
        K = P / (P + R);
        x += K * (measurement - x);
        P *= (1 - K);
        return x;
    }
};

// Класс для работы с картой местности
class Map {
public:
    std::vector<Coordinates> terrain;

    bool loadMap(const std::string &filename) {
        std::ifstream file(filename, std::ifstream::binary);
        if (!file.is_open()) {
            std::cerr << "Ошибка при открытии файла карты: " << filename << std::endl;
            return false;
        }

        // Создаем объект JSON для парсинга
        Json::Value root;
        file >> root;

        // Чтение координат из JSON
        for (const auto &point : root["terrain"]) {
            Coordinates coord;
            coord.latitude = point["latitude"].asDouble();
            coord.longitude = point["longitude"].asDouble();
            terrain.push_back(coord);
        }

        return true;
    }
};

// Класс для моделирования GNSS
class GNSSModel {
public:
    KalmanFilter filter;

    GNSSModel(KalmanFilter kf) : filter(kf) {}

    std::vector<Coordinates> simulate(const std::vector<Coordinates> &map) {
        std::vector<Coordinates> gnssPath;
        for (const auto &coord : map) {
            double noisyLatitude = coord.latitude + (static_cast<double>(rand()) / RAND_MAX - 0.5) * 0.01; // добавляем шум
            double noisyLongitude = coord.longitude + (static_cast<double>(rand()) / RAND_MAX - 0.5) * 0.01; // добавляем шум

            // Применяем фильтр Калмана
            noisyLatitude = filter.update(noisyLatitude);
            noisyLongitude = filter.update(noisyLongitude);
            gnssPath.push_back({noisyLatitude, noisyLongitude});
        }
        return gnssPath;
    }
};

// Класс для моделирования INS
class INSModel {
public:
    std::vector<Coordinates> simulate(const std::vector<Coordinates> &map) {
        std::vector<Coordinates> insPath = map; // Простое моделирование
        // Здесь можно добавить логику для моделирования INS
        return insPath;
    }
};

// Класс для моделирования внешних условий
class ExternalConditions {
public:
    std::vector<Coordinates> modifyTrajectory(const std::vector<Coordinates> &map, double windFactor) {
        std::vector<Coordinates> modifiedPath = map; // Начальная траектория совпадает с картой
        for (auto &coord : modifiedPath) {
            coord.latitude += windFactor * 0.0001; // Добавляем сдвиг по широте
            coord.longitude += windFactor * 0.0001; // Добавляем сдвиг по долготе
        }
        return modifiedPath;
    }
};

// Класс для симуляции
class Simulation {
public:
    Map terrainMap;
    GNSSModel gnssModel;
    INSModel insModel;
    ExternalConditions externalConditions;

    Simulation() : gnssModel(KalmanFilter()) {}

    void run(int scenario) {
        if (!terrainMap.loadMap("terrain.json")) {
            std::cerr << "Ошибка загрузки карты." << std::endl;
            return;
        }

        double windFactor = 0.0;
        switch (scenario) {
            case 1: windFactor = 0.0; break; // Нормальные условия
            case 2: windFactor = 0.5; break; // Умеренный ветер
            case 3: windFactor = 1.0; break; // Сильный ветер
            default:
                std::cerr << "Неверный сценарий." << std::endl;
                return;
        }

        // Параллельное моделирование
        std::vector<Coordinates> gnssPath, insPath;
        std::thread gnssThread([&]() { gnssPath = gnssModel.simulate(terrainMap.terrain); });
        std::thread insThread([&]() { insPath = insModel.simulate(terrainMap.terrain); });

        // Ждем завершения потоков
        gnssThread.join();
        insThread.join();

        // Моделирование внешних условий
        std::vector<Coordinates> modifiedPath = externalConditions.modifyTrajectory(terrainMap.terrain, windFactor);

        // Логирование данных
        logData(terrainMap.terrain, gnssPath, insPath, "flight_log.csv");
    }

    void logData(const std::vector<Coordinates> &truePath, const std::vector<Coordinates> &gnssPath, const std::vector<Coordinates> &insPath, const std::string &filename) {
        std::ofstream logFile(filename);
        if (!logFile.is_open()) {
            std::cerr << "Ошибка при открытии файла для логирования: " << filename << std::endl;
            return;
        }

        logFile << "True Latitude,True Longitude,GNSS Latitude,GNSS Longitude,INS Latitude,INS Longitude\n";
        for (size_t i = 0; i < truePath.size(); ++i) {
            logFile << truePath[i].latitude << "," << truePath[i].longitude << ","
                    << gnssPath[i].latitude << "," << gnssPath[i].longitude << ","
                    << insPath[i].latitude << "," << insPath[i].longitude << "\n";
        }
        logFile.close();
    }
};

int main() {
    int scenario;
    std::cout << "Выберите сценарий (1 - Нормальные условия, 2 - Умеренный ветер, 3 - Сильный ветер): ";
    std::cin >> scenario;

    Simulation simulation;
    simulation.run(scenario);

    return 0;
}
