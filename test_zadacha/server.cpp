// Код для сервера
#include <iostream> // Подключение библиотеки для ввода-вывода
#include <winsock2.h> // Подключение библиотеки Winsock
#include <fstream> // Подключение библиотеки для работы с файлами

#pragma comment(lib, "ws2_32.lib") // Подключаем библиотеку Winsock

// Функция для получения файла от клиента
void ReceiveFile(SOCKET clientSocket) {
    DWORD fileSize; // Переменная для хранения размера файла
    // Получаем размер файла
    int result = recv(clientSocket, (char*)&fileSize, sizeof(fileSize), 0); // Получаем размер файла
    if (result <= 0) { // Проверяем результат
        std::cerr << "Ошибка получения размера файла." << std::endl; // Выводим ошибку
        return; // Выходим из функции
    }

    BYTE* buffer = new BYTE[fileSize]; // Выделяем память для буфера
    // Получаем данные файла
    result = recv(clientSocket, (char*)buffer, fileSize, 0); // Получаем данные файла
    if (result <= 0) { // Проверяем результат
        std::cerr << "Ошибка получения данных файла." << std::endl; // Выводим ошибку
        delete[] buffer; // Освобождаем память
        return; // Выходим из функции
    }

    std::ofstream outFile("received_screenshot.bmp", std::ios::binary); // Открываем файл для записи
    outFile.write((char*)buffer, fileSize); // Записываем данные в файл
    outFile.close(); // Закрываем файл

    std::cout << "Скриншот получен и сохранён как received_screenshot.bmp" << std::endl; // Выводим сообщение
    delete[] buffer; // Освобождаем память
}

// Главная функция сервера
int main() {
    WSADATA wsaData; // Структура для инициализации Winsock
    SOCKET serverSocket, clientSocket; // Сокеты для сервера и клиента
    sockaddr_in serverAddr, clientAddr; // Структуры для хранения адресов сервера и клиента
    int addrLen = sizeof(clientAddr); // Длина адреса клиента

    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Инициализация Winsock
        std::cerr << "Ошибка инициализации Winsock." << std::endl; // Выводим ошибку
        return 1; // Возвращаем 1 при ошибке
    }

    // Создание сокета
    serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Создаем сокет
    if (serverSocket == INVALID_SOCKET) { // Проверяем, успешно ли создан сокет
        std::cerr << "Ошибка создания сокета." << std::endl; // Выводим ошибку
        WSACleanup(); // Очищаем Winsock
        return 1; // Возвращаем 1 при ошибке
    }

    // Настройка адреса сервера
    serverAddr.sin_family = AF_INET; // Устанавливаем семейство адресов
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Принимаем соединения на всех интерфейсах
    serverAddr.sin_port = htons(8080); // Устанавливаем порт, на котором будет слушать сервер

    // Привязка сокета к адресу
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) { // Привязываем сокет
        std::cerr << "Ошибка привязки сокета." << std::endl; // Выводим ошибку
        closesocket(serverSocket); // Закрываем сокет
        WSACleanup(); // Очищаем Winsock
        return 1; // Возвращаем 1 при ошибке
    }

    // Начинаем слушать входящие соединения
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) { // Начинаем прослушивание
        std::cerr << "Ошибка прослушивания." << std::endl; // Выводим ошибку
        closesocket(serverSocket); // Закрываем сокет
        WSACleanup(); // Очищаем Winsock
        return 1; // Возвращаем 1 при ошибке
    }

    std::cout << "Сервер запущен, ожидаем подключения..." << std::endl; // Выводим сообщение

    // Принимаем входящее соединение
    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen); // Принимаем соединение
    if (clientSocket == INVALID_SOCKET) { // Проверяем, успешно ли принято соединение
        std::cerr << "Ошибка принятия соединения." << std::endl; // Выводим ошибку
        closesocket(serverSocket); // Закрываем сокет
        WSACleanup(); // Очищаем Winsock
        return 1; // Возвращаем 1 при ошибке
    }

    // Получаем данные от клиента
    char buffer[1024]; // Буфер для получения данных
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); // Получаем данные
    if (bytesReceived > 0) { // Проверяем, были ли получены данные
        buffer[bytesReceived] = '\0'; // Завершаем строку
        std::cout << "Полученные данные: " << buffer << std::endl; // Выводим полученные данные
    }

    // Вызываем функцию для получения файла
    ReceiveFile(clientSocket); // Получаем файл от клиента

    // Закрываем сокеты и очищаем Winsock
    closesocket(clientSocket); // Закрываем сокет клиента
    closesocket(serverSocket); // Закрываем сокет сервера
    WSACleanup(); // Очищаем Winsock

    return 0; // Возвращаем 0 при завершении программы
}
