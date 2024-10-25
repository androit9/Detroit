#include "MyFrame.h" // Подключение заголовочного файла MyFrame.h, содержащего объявление класса MyFrame
#include <wx/sizer.h> // Подключение для работы с компоновщиками в wxWidgets
#include <wx/image.h> // Подключение для работы с изображениями в wxWidgets
#include <winsock2.h> // Подключение для работы с сокетами Windows
#include <ws2tcpip.h> // Подключение для работы с TCP/IP сокетами
#include <thread> // Подключение для работы с потоками стандартной библиотеки

#pragma comment(lib, "ws2_32.lib") // Подключение библиотеки для работы с сокетами Windows

// Конструктор класса MyFrame
MyFrame::MyFrame() : wxFrame(nullptr, wxID_ANY, "Server Clients") {
    // Создание элемента списка для отображения клиентов
    clientList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    
    // Добавление колонок в элемент списка для отображения информации о клиентах
    clientList->InsertColumn(0, "Username", wxLIST_FORMAT_LEFT, 100); // Колонка для имени пользователя
    clientList->InsertColumn(1, "Hostname", wxLIST_FORMAT_LEFT, 100); // Колонка для имени хоста
    clientList->InsertColumn(2, "IP Address", wxLIST_FORMAT_LEFT, 100); // Колонка для IP-адреса
    clientList->InsertColumn(3, "Action", wxLIST_FORMAT_LEFT, 100); // Колонка для действий (например, кнопки)

    // Создание компоновщика для вертикального расположения элементов
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(clientList, 1, wxEXPAND | wxALL, 5); // Добавление элемента списка в компоновщик
    SetSizer(sizer); // Установка компоновщика для окна

    StartServer(); // Запуск сервера для обработки клиентов
}

// Метод для запуска сервера
void MyFrame::StartServer() {
    // Создание потока для работы сервера
    std::thread([this]() {
        WSADATA wsaData; // Структура для хранения информации о версии Winsock
        // Инициализация Winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            wxMessageBox("WSAStartup failed.", "Error", wxOK | wxICON_ERROR); // Ошибка инициализации
            return; // Завершение работы метода
        }

        // Создание сокета для сервера
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            wxMessageBox("Socket creation failed.", "Error", wxOK | wxICON_ERROR); // Ошибка создания сокета
            WSACleanup(); // Очистка Winsock
            return; // Завершение работы метода
        }

        sockaddr_in serverAddr; // Структура для хранения адреса сервера
        serverAddr.sin_family = AF_INET; // Установка семейства адресов
        serverAddr.sin_addr.s_addr = INADDR_ANY; // Установка IP-адреса для прослушивания
        serverAddr.sin_port = htons(8080); // Установка порта для прослушивания

        // Привязка сокета к адресу
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            wxMessageBox("Bind failed.", "Error", wxOK | wxICON_ERROR); // Ошибка привязки
            closesocket(serverSocket); // Закрытие сокета
            WSACleanup(); // Очистка Winsock
            return; // Завершение работы метода
        }

        // Начало прослушивания входящих соединений
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            wxMessageBox("Listen failed.", "Error", wxOK | wxICON_ERROR); // Ошибка прослушивания
            closesocket(serverSocket); // Закрытие сокета
            WSACleanup(); // Очистка Winsock
            return; // Завершение работы метода
        }

        while (true) { // Бесконечный цикл для обработки клиентов
            sockaddr_in clientAddr; // Структура для хранения адреса клиента
            int clientAddrSize = sizeof(clientAddr); // Размер структуры адреса клиента
            // Принятие входящего соединения
            SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
            if (clientSocket == INVALID_SOCKET) {
                wxMessageBox("Accept failed.", "Error", wxOK | wxICON_ERROR); // Ошибка принятия соединения
                continue; // Переход к следующему циклу
            }

            char hostname[256]; // Массив для хранения имени хоста клиента
            // Получение имени хоста клиента
            getnameinfo((sockaddr*)&clientAddr, sizeof(clientAddr), hostname, sizeof(hostname), nullptr, 0, NI_NUMERICHOST);

            // Получение имени пользователя
            char username[256]; // Массив для хранения имени пользователя
            DWORD username_len = sizeof(username); // Размер массива
            GetUserNameA(username, &username_len); // Получение имени пользователя

            {
                std::lock_guard<std::mutex> lock(clientMutex); // Блокировка мьютекса для безопасного доступа к данным
                clients.emplace_back(username, hostname, inet_ntoa(clientAddr.sin_addr)); // Добавление информации о клиенте в вектор
                AddClient(clients.back()); // Добавление клиента в список
            }

            // Обработка клиента в отдельном потоке
            std::thread([this, clientSocket]() {
                // Реализуйте здесь логику для получения скриншота, если нужно
                closesocket(clientSocket); // Закрытие сокета клиента после обработки
            }).detach(); // Отсоединение потока
        }

        closesocket(serverSocket); // Закрытие сокета сервера
        WSACleanup(); // Очистка Winsock
    }).detach(); // Отсоединение потока
}

// Метод для добавления клиента в список
void MyFrame::AddClient(const ClientInfo& client) {
    long index = clientList->InsertItem(clientList->GetItemCount(), client.username); // Добавление нового элемента в список
    clientList->SetItem(index, 1, client.hostname); // Установка имени хоста
    clientList->SetItem(index, 2, client.ip); // Установка IP-адреса

    // Создание кнопки для получения скриншота
    wxButton* screenButton = new wxButton(this, wxID_ANY, "Screen", wxDefaultPosition, wxDefaultSize);
    clientList->SetItemData(index, reinterpret_cast<uintptr_t>(screenButton)); // Сохранение кнопки в данных элемента
    screenButton->Bind(wxEVT_BUTTON, &MyFrame::OnScreenButton, this); // Привязка события нажатия кнопки к методу
}

// Метод для обработки нажатия кнопки "Screen"
void MyFrame::OnScreenButton(wxCommandEvent& event) {
    // Здесь вы можете реализовать логику для получения скриншота от клиента
    TakeScreenshot(); // Вызов метода для получения скриншота
}

// Метод для получения скриншота
void MyFrame::TakeScreenshot() {
    // Получаем размер экрана
    wxScreenDC screenDC; // Создание устройства контекста для экрана
    wxSize size = wxGetDisplaySize(); // Получение размера дисплея

    // Создаем изображение
    wxBitmap bitmap(size.x, size.y); // Создание битмапа заданного размера
    wxMemoryDC memoryDC(bitmap); // Создание контекста памяти для рисования в битмапе

    // Копируем экран в bitmap
    memoryDC.Blit(0, 0, size.x, size.y, &screenDC, 0, 0); // Копирование содержимого экрана в битмап

    // Сохраняем изображение в файл
    wxImage image = bitmap.ConvertToImage(); // Преобразование битмапа в изображение
    wxFileName fileName(wxStandardPaths::Get().GetDocumentsDir(), "screenshot.png"); // Определение имени файла для сохранения

    // Сохранение изображения в файл
    if (image.SaveFile(fileName.GetFullPath(), wxBITMAP_TYPE_PNG)) {
        wxMessageBox("Screenshot saved to " + fileName.GetFullPath(), "Success", wxOK | wxICON_INFORMATION); // Успешно сохранено
    } else {
        wxMessageBox("Failed to save screenshot.", "Error", wxOK | wxICON_ERROR); // Ошибка сохранения
    }
}
