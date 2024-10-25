#include <wx/wx.h> // Подключение библиотеки wxWidgets для работы с графическим интерфейсом
#include <wx/listctrl.h> // Подключение для работы со списком клиентов
#include <wx/sizer.h> // Подключение для управления компоновкой элементов
#include <wx/thread.h> // Подключение для работы с потоками
#include <wx/filename.h> // Подключение для работы с именами файлов
#include <wx/image.h> // Подключение для работы с изображениями
#include <winsock2.h> // Подключение для работы с сокетами Windows
#include <ws2tcpip.h> // Подключение для работы с TCP/IP сокетами
#include <string> // Подключение для работы со строками
#include <vector> // Подключение для работы с векторами
#include <thread> // Подключение для работы с потоками
#include <mutex> // Подключение для работы с мьютексами
#include <cstdint> // Подключение для работы с фиксированными типами данных
#include <iostream> // Подключение для работы с вводом/выводом
#include <fstream> // Подключение для работы с файлами

#pragma comment(lib, "ws2_32.lib") // Подключение библиотеки для работы с сокетами

// Структура для хранения информации о клиенте
struct ClientInfo {
    std::string username; // Имя пользователя клиента
    std::string hostname; // Имя хоста клиента
    std::string ip; // IP-адрес клиента
    SOCKET socket; // Сокет клиента для связи

    // Конструктор для инициализации структуры ClientInfo
    ClientInfo(const std::string& user, const std::string& host, const std::string& ipAddr, SOCKET sock)
        : username(user), hostname(host), ip(ipAddr), socket(sock) {} // Инициализация полей структуры
};

// Класс для основного окна приложения
class MyFrame : public wxFrame {
public:
    MyFrame(); // Конструктор класса MyFrame
    void StartServer(); // Метод для запуска сервера
    void AddClient(const ClientInfo& client); // Метод для добавления клиента в список
    void OnScreenButton(wxCommandEvent& event); // Метод для обработки нажатия кнопки "Screen"
    void TakeScreenshot(ClientInfo& client); // Метод для получения скриншота от клиента

private:
    wxListCtrl* clientList; // Указатель на элемент списка клиентов
    std::vector<ClientInfo> clients; // Вектор для хранения информации о клиентах
    std::mutex clientMutex; // Мьютекс для синхронизации доступа к данным клиентов

    wxDECLARE_EVENT_TABLE(); // Макрос для объявления таблицы событий
};

// Класс для приложения wxWidgets
class MyApp : public wxApp {
public:
    virtual bool OnInit(); // Метод для инициализации приложения
};

// Определение таблицы событий для класса MyFrame
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
wxEND_EVENT_TABLE()

// Реализация приложения
wxIMPLEMENT_APP(MyApp);

// Метод инициализации приложения
bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame(); // Создание нового окна MyFrame
    frame->Show(true); // Отображение окна
    return true; // Успешная инициализация
}

// Конструктор класса MyFrame
MyFrame::MyFrame() : wxFrame(nullptr, wxID_ANY, "Server Clients") {
    // Создание элемента списка для отображения клиентов
    clientList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    clientList->InsertColumn(0, "Username", wxLIST_FORMAT_LEFT, 100); // Добавление колонки для имени пользователя
    clientList->InsertColumn(1, "Hostname", wxLIST_FORMAT_LEFT, 100); // Добавление колонки для имени хоста
    clientList->InsertColumn(2, "IP Address", wxLIST_FORMAT_LEFT, 100); // Добавление колонки для IP-адреса
    clientList->InsertColumn(3, "Action", wxLIST_FORMAT_LEFT, 100); // Добавление колонки для действий

    // Создание компоновщика для вертикального расположения элементов
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(clientList, 1, wxEXPAND | wxALL, 5); // Добавление элемента списка в компоновщик
    SetSizer(sizer); // Установка компоновщика для окна

    StartServer(); // Запуск сервера
}

// Метод для запуска сервера
void MyFrame::StartServer() {
    std::thread([this]() { // Создание нового потока для работы сервера
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
            GetUser NameA(username, &username_len); // Получение имени пользователя

            {
                std::lock_guard<std::mutex> lock(clientMutex); // Блокировка мьютекса для безопасного доступа к данным
                // Добавление информации о клиенте в вектор
                clients.emplace_back(username, hostname, inet_ntoa(clientAddr.sin_addr), clientSocket);
                AddClient(clients.back()); // Добавление клиента в список
            }
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
    long index = clientList->GetFirstSelected(); // Получение индекса выбранного элемента
    if (index != -1) { // Если элемент выбран
        ClientInfo& client = clients[index]; // Получение информации о выбранном клиенте
        TakeScreenshot(client); // Вызов метода для получения скриншота
    }
}

// Метод для получения скриншота от клиента
void MyFrame::TakeScreenshot(ClientInfo& client) {
    // Отправка команды клиенту
    const char* command = "screenshot"; // Команда для получения скриншота
    send(client.socket, command, strlen(command), 0); // Отправка команды через сокет

    // Ожидание получения изображения
    char buffer[4096]; // Размер буфера для получения изображения
    int bytesReceived = recv(client.socket, buffer, sizeof(buffer), 0); // Получение данных от клиента
    if (bytesReceived > 0) { // Если данные получены
        // Сохранение полученных данных в файл
        std::ofstream outFile("screenshot.png", std::ios::binary); // Открытие файла для записи в бинарном режиме
        outFile.write(buffer, bytesReceived); // Запись данных в файл
        outFile.close(); // Закрытие файла

        wxMessageBox("Screenshot received and saved as screenshot.png", "Success", wxOK | wxICON_INFORMATION); // Успешное получение скриншота
    } else {
        wxMessageBox("Failed to receive screenshot from client.", "Error", wxOK | wxICON_ERROR); // Ошибка получения скриншота
    }
}

// Главная функция приложения
int main() {
    return wxEntry(); // Запуск приложения wxWidgets
}
